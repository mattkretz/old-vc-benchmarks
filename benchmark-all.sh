#!/bin/bash

case "@QT4_FOUND@" in
  YES|TRUE|true|yes|on|ON) haveQt4=true ;;
  *) haveQt4=false ;;
esac
case "@USE_SSE2@" in
  YES|TRUE|true|yes|on|ON) haveSse=true ;;
  *) haveSse=false ;;
esac
case "@USE_AVX@" in
  YES|TRUE|true|yes|on|ON) haveAvx=true ;;
  *) haveAvx=false ;;
esac
case "@USE_AVX2@" in
  YES|TRUE|true|yes|on|ON) haveAvx2=true ;;
  *) haveAvx2=false ;;
esac
case "@USE_XOP@" in
  YES|TRUE|true|yes|on|ON) haveXop=true ;;
  *) haveXop=false ;;
esac
case "@USE_FMA4@" in
  YES|TRUE|true|yes|on|ON) haveFma4=true ;;
  *) haveFma4=false ;;
esac

cd `dirname $0`
resultsDir="benchmark-all-`hostname`-`date '+%Y-%m-%d-%H-%M-%S'`"
mkdir $resultsDir || exit
echo "Storing benchmark results to $PWD/$resultsDir"

srcdir='@Vc_SOURCE_DIR@'
rev=`cut -d' ' -f2 "$srcdir/.git/HEAD"`
branch=${rev##*/}
rev=`cat "$srcdir/.git/$rev"`

cat > $resultsDir/metadata <<EOF
build type	: @CMAKE_BUILD_TYPE@
compiler	: `echo "@CXX_VERSION@"|head -n1`
strict aliasing	: @ENABLE_STRICT_ALIASING@
fast math	: @FAST_MATH_BENCHMARK@
realtime	: @REALTIME_BENCHMARKS@
target arch	: @TARGET_ARCHITECTURE@
flags		: `echo '@CMAKE_CXX_FLAGS@'|sed 's/-W[^ ]*//g'|tr -s '[:space:]'`
Vc branch	: $branch
Vc revision	: $rev
hostname	: `hostname`
machine		: `uname -m`
`grep -m1 -B2 'model name' /proc/cpuinfo`
EOF

# examine the CPU topology, we'd like to run benchmarks on all cores, but keep thread siblings idle
# (i.e. Intel Hyperthreading or AMD Modules)
blockedIds=()
usableCores=()
for cpu in `find /sys/devices/system/cpu -name "cpu[0-9]*"`; do
  coreid=${cpu##*/cpu}
  if [[ -z "${blockedIds[$coreid]}" ]]; then
    usableCores=(${usableCores[@]} $coreid)
    blockedIds[$coreid]=1
    if [[ -r $cpu/topology/thread_siblings_list ]]; then
      read siblings < $cpu/topology/thread_siblings_list
      case "$siblings" in
        *-*)
          start=${siblings%%-*}
          end=${siblings##*-}
          for (( i=$start; $i<=$end; ++i)); do
            blockedIds[$i]=1
          done
          ;;
        *,*)
          for i in ${siblings//,/ }; do
            blockedIds[$i]=1
          done
      esac
    fi
  fi
done

# One core (and its sibling) must stay idle for the OS (and shell). Removing the first usable core
# from the list:
usableCores=(${usableCores[@]:1:${#usableCores[@]}})

idleCores=(${usableCores[@]})

echo "The following cores will be used for parallel execution of the benchmarks: ${idleCores[@]}"
echo

fifo="$resultsDir/.fifo"
mkfifo "$fifo"

executeBench()
{
  name=${1}_${2}
  if [[ -z "${idleCores[@]}" ]]; then
    idleCores=(`cat "$fifo"`)
  fi
  if test -x ./$name; then
    coreid=${idleCores[${#idleCores[@]}-1]}
    unset idleCores[${#idleCores[@]}-1]
    (
    outfile=$resultsDir/$name
    if test "$2" = "avx"; then
      $haveXop && outfile=${outfile}-mxop
      $haveFma4 && outfile=${outfile}-mfma4
    else
      $haveAvx && outfile=${outfile}-mavx
    fi
    outfile=${outfile}-run$3.dat
    printf "%22s -o %s\tStarted.\n" "$name" "$outfile"
    if numactl --physcpubind=$coreid --localalloc ./$name -o $outfile >/dev/null 2>&1; then
      printf "%22s -o %s\tDone.\n" "$name" "$outfile"
    else
      printf "%22s -o %s\tFAILED.\n" "$name" "$outfile"
      rm -f $outfile
    fi
    echo $coreid > "$fifo" &
    ) &
  else
    printf "%22s SKIPPED\n" "$name"
  fi
}

if which benchmarking.sh >/dev/null; then
  echo "Calling 'benchmarking.sh start' to disable powermanagement and Turbo-Mode"
  benchmarking.sh start
fi

for run in 1 2 3; do
for bench in \
  interleavedmemorywrapper flops arithmetics2 gather scatter mask compare math memio dhryrock whetrock mandelbrotbench
do
  executeBench $bench scalar $run
  $haveSse && executeBench $bench sse $run
  $haveAvx && executeBench $bench avx $run
  $haveAvx2 && executeBench $bench avx2 $run
done
done
wait
cat "$fifo"
rm -f "$fifo"

if which benchmarking.sh >/dev/null; then
  echo "Calling 'benchmarking.sh stop' to re-enable powermanagement and Turbo-Mode"
  benchmarking.sh stop
fi

echo "Packing results into ${resultsDir}.tar.gz"
tar -czf ${resultsDir}.tar.gz ${resultsDir}/

# vim: sw=2 et
