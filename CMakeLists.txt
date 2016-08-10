cmake_minimum_required(VERSION 2.8.1)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
   message(ERROR "You don't want to configure in the source directory!")
endif()

project(Vc-Benchmarks)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Vc 1.0 REQUIRED)
include_directories(${Vc_INCLUDE_DIR})
set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS ${Vc_ALL_FLAGS})

include(AddTargetProperty)

set(disabled_targets)
if(Vc_COMPILER_IS_GCC AND Vc_GCC_VERSION STREQUAL "4.6.0")
   list(APPEND disabled_targets
      dhryrock_avx
      )
endif()

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(USE_RDTSCP TRUE CACHE STRING "Use to disable the use of the rdtscp instruction. E.g. valgrind doesn't work with it.")
if(NOT USE_RDTSCP)
   add_definitions(-DNO_RDTSCP)
endif()

AddCompilerFlag("-frename-registers")
AddCompilerFlag("-std=c++14")

add_custom_target(other VERBATIM)
add_custom_target(Scalar COMMENT "build Scalar code" VERBATIM)
add_custom_target(SSE COMMENT "build SSE code" VERBATIM)
add_custom_target(AVX COMMENT "build AVX code" VERBATIM)

add_custom_target(benchmarks COMMENT "Run all benchmarks" VERBATIM)
add_target_property(benchmarks EXCLUDE_FROM_DEFAULT_BUILD 1)

add_custom_target(benchmark_data COMMENT "Create data for later processing for all benchmarks" VERBATIM)
add_target_property(benchmark_data EXCLUDE_FROM_DEFAULT_BUILD 1)

macro(vc_generate_datafile target outfilepath)
   get_target_property(exec ${target} OUTPUT_NAME)
   set(outfile "${exec}.dat")
   set(${outfilepath} "${CMAKE_CURRENT_BINARY_DIR}/${exec}.dat")
   add_custom_command(OUTPUT "${${outfilepath}}"
      COMMAND ${target}
      ARGS -o "${${outfilepath}}"
      DEPENDS ${target}
      COMMENT "Running Benchmark ${exec} to generate ${outfile}"
      VERBATIM
      )
endmacro(vc_generate_datafile)

macro(vc_generate_plots name)
   set(dataFilePaths)

   foreach(def "scalar" "sse" ${ARGN})
      set(_t "${name}_${def}")
      vc_generate_datafile(${_t} dfp)
      set(dataFilePaths ${dataFilePaths} ${dfp})
   endforeach(def)

   set(generate_target "generate_${name}_data")
   add_custom_target(${generate_target} DEPENDS ${dataFilePaths} "${scriptfile}")
   add_dependencies(benchmark_data "${generate_target}")
endmacro(vc_generate_plots)

macro(vc_add_benchmark name)
   set(LIBS cpuset)
   if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      set(LIBS ${LIBS} rt -pthread)
   endif()

   set(variants)
   list(APPEND variants "scalar" ${ARGN})
   if(USE_SSE2)
      list(APPEND variants "sse")
   endif()
   if(USE_AVX)
      list(APPEND variants "avx")
   endif()
   if(USE_AVX2)
      list(APPEND variants "avx2")
   endif()

   foreach(def ${variants})
      set(target "${name}_${def}")

      list(FIND disabled_targets ${target} _disabled)
      if(_disabled EQUAL -1)
         add_executable(${target} ${name}.cpp benchmark.cpp)
         target_link_libraries(${target} ${Vc_LIBRARIES} ${LIBS})
         if(def STREQUAL "scalar")
            add_target_property(${target} COMPILE_FLAGS "-DVc_IMPL=Scalar")
            add_target_property(${target} LABELS "Scalar")
            add_dependencies(Scalar ${target})
         elseif(def STREQUAL "sse")
            add_target_property(${target} COMPILE_FLAGS "-DVc_IMPL=SSE")
            add_target_property(${target} LABELS "SSE")
            add_dependencies(SSE ${target})
         elseif(def STREQUAL "avx")
            add_target_property(${target} COMPILE_FLAGS "-DVc_IMPL=AVX")
            add_target_property(${target} LABELS "AVX")
            add_dependencies(AVX ${target})
         elseif(def STREQUAL "avx2")
            add_target_property(${target} COMPILE_FLAGS "-DVc_IMPL=AVX2")
            add_target_property(${target} LABELS "AVX2")
            add_dependencies(AVX ${target})
         else()
            add_target_property(${target} COMPILE_FLAGS "-D${def}")
            add_target_property(${target} LABELS "other")
            add_dependencies(other ${target})
         endif()
         add_custom_target("run_${target}" ${target} DEPENDS ${target} COMMENT "Running ${target}")
         add_target_property("run_${target}" EXCLUDE_FROM_DEFAULT_BUILD 1)
         add_dependencies(benchmarks "run_${target}")
      endif()
   endforeach()
endmacro()

set(NO_AUTOVEC "-fno-tree-vectorize")
check_cxx_compiler_flag(${NO_AUTOVEC} check_compiler_flag_no_autovect_gcc)
if(NOT check_compiler_flag_no_autovect_gcc)
   set(NO_AUTOVEC "-no-vec")
   check_cxx_compiler_flag(${NO_AUTOVEC} check_compiler_flag_no_autovect_icc)
   if(NOT check_compiler_flag_no_autovect_icc)
      message(STATUS "Missing flag to disable autovectorization in the compiler")
      SET(NO_AUTOVEC)
   endif(NOT check_compiler_flag_no_autovect_icc)
endif(NOT check_compiler_flag_no_autovect_gcc)

set(NO_PREFETCH "-fno-prefetch-loop-arrays")
check_cxx_compiler_flag(${NO_PREFETCH} check_compiler_flag_no_prefetch_gcc)
if(NOT check_compiler_flag_no_prefetch_gcc)
   message(STATUS "No compiler flag known to disable prefetching")
   set(NO_PREFETCH "")
endif()

find_file(HAVE_SYS_MMAN sys/mman.h)
if(HAVE_SYS_MMAN)
   set(USE_MLOCK_IN_BENCHMARKS TRUE CACHE BOOL "Use memlock to get more reliable results in the benchmarks.")
   if(NOT USE_MLOCK_IN_BENCHMARKS)
      add_definitions(-DVC_BENCHMARK_NO_MLOCK)
   endif(NOT USE_MLOCK_IN_BENCHMARKS)
else(HAVE_SYS_MMAN)
   add_definitions(-DVC_BENCHMARK_NO_MLOCK)
endif(HAVE_SYS_MMAN)
mark_as_advanced(HAVE_SYS_MMAN)

set(FAST_MATH_BENCHMARK FALSE CACHE BOOL "Use floating point math shortcuts. Might not be IEEE compliant!")
if(FAST_MATH_BENCHMARK)
   if(COMPILER_IS_GCC OR COMPILER_IS_OPEN64)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
   elseif(COMPILER_IS_INTEL)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fp-model fast=2")
   elseif(COMPILER_IS_MSVC)
      message(WARNING "Missing flag for MSVC to implement fast-math")
   endif()
endif(FAST_MATH_BENCHMARK)

add_library(cpuset STATIC cpuset.c)
add_target_property(cpuset LABELS "other")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   set(REALTIME_BENCHMARKS FALSE CACHE BOOL "Use SCHED_FIFO priority to give benchmarks highest priority, achieving less jitter in the results.")
   if(REALTIME_BENCHMARKS)
      add_executable(rtwrapper rtwrapper.cpp)
      add_definitions(-DSCHED_FIFO_BENCHMARKS)
      add_custom_command(TARGET rtwrapper POST_BUILD
         COMMAND sudo chown root rtwrapper
         COMMAND sudo chmod +s rtwrapper
         WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
         COMMENT "Making the rtwrapper suid root"
         )
   endif(REALTIME_BENCHMARKS)

   set(LOG_CPUTIME FALSE CACHE BOOL "Enable additional logging of the CPU time. This adds little information and makes time keeping more costly and less accurate.")
   if(LOG_CPUTIME)
      add_definitions(-DVC_USE_CPU_TIME)
   endif()
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
add_definitions(-DVC_COMPILE_BENCHMARKS)

if(EXISTS "${Vc_SOURCE_DIR}/common/interleavedmemory.h")
   vc_add_benchmark(interleavedmemorywrapper VC_USE_MASKMOV_SCATTER)
endif()
vc_add_benchmark(arithmetics)
vc_add_benchmark(arithmetics2)
vc_add_benchmark(flops)
vc_add_benchmark(gather VC_USE_BSF_GATHERS VC_USE_POPCNT_BSF_GATHERS VC_USE_SET_GATHERS)
vc_add_benchmark(scatter VC_USE_BSF_SCATTERS VC_USE_POPCNT_BSF_SCATTERS)
vc_add_benchmark(mask)
vc_add_benchmark(compare VC_DISABLE_PTEST)
vc_add_benchmark(math)
vc_add_benchmark(memio)
add_target_property(memio_scalar COMPILE_FLAGS "${NO_AUTOVEC} ${NO_PREFETCH}")
if(USE_SSE2 AND NO_PREFETCH)
   add_target_property(memio_sse COMPILE_FLAGS ${NO_PREFETCH})
endif()
if(USE_AVX AND NO_PREFETCH)
   add_target_property(memio_avx COMPILE_FLAGS ${NO_PREFETCH})
endif()
vc_add_benchmark(dhryrock)
vc_add_benchmark(whetrock)

add_executable(flops_autovect autoflops.cpp benchmark.cpp)
target_link_libraries(flops_autovect ${Vc_LIBRARIES} ${LIBS})
add_executable(flops_noautovect autoflops.cpp benchmark.cpp)
check_cxx_compiler_flag("-fno-tree-vectorize" check_compiler_flag_no_autovect_gcc)
add_target_property(flops_noautovect COMPILE_FLAGS "${NO_AUTOVEC}")
target_link_libraries(flops_noautovect ${Vc_LIBRARIES} ${LIBS})

add_executable(sort sort.cpp benchmark.cpp)
target_link_libraries(sort ${Vc_LIBRARIES} ${LIBS})
add_target_property(sort OUTPUT_NAME sort)
add_custom_target("run_sort" sort DEPENDS sort COMMENT "Running sort")
add_target_property("run_sort" EXCLUDE_FROM_DEFAULT_BUILD 1)
add_dependencies(benchmarks "run_sort")

#vc_generate_plots(arithmetics)
#vc_generate_plots(arithmetics2)
#vc_generate_plots(flops)
#vc_generate_plots(gather VC_NO_BSF_LOOPS)
#vc_generate_plots(mask)
#vc_generate_plots(compare VC_DISABLE_PTEST)
#vc_generate_plots(math)
#vc_generate_plots(memio)

if(NOT COMPILER_IS_MSVC)
   add_executable(constants constants.cpp benchmark.cpp)
   target_link_libraries(constants ${Vc_LIBRARIES} ${LIBS})
endif()

#find_package(Qt4)
#if(QT4_FOUND)
#   include(${QT_USE_FILE})
#   include_directories(${CMAKE_CURRENT_BINARY_DIR})
#   qt4_generate_moc(../examples/mandelbrot/mandel.h moc_mandel.cpp)
#
#   set(SOURCES mandelbrot.cpp ../examples/mandelbrot/mandel.cpp
#      ${CMAKE_CURRENT_BINARY_DIR}/moc_mandel.cpp)
#
#   add_executable(mandelbrotbench_sse ${SOURCES})
#   add_target_property(mandelbrotbench_sse COMPILE_FLAGS "-DVC_IMPL=SSE")
#   target_link_libraries(mandelbrotbench_sse ${QT_LIBRARIES} ${Vc_LIBRARIES})
#
#   add_executable(mandelbrotbench_scalar ${SOURCES})
#   add_target_property(mandelbrotbench_scalar COMPILE_FLAGS "-DVC_IMPL=Scalar")
#   target_link_libraries(mandelbrotbench_scalar ${QT_LIBRARIES} ${Vc_LIBRARIES})
#
#   if(USE_AVX)
#      add_executable(mandelbrotbench_avx ${SOURCES})
#      add_target_property(mandelbrotbench_avx COMPILE_FLAGS "-DVC_IMPL=AVX")
#      target_link_libraries(mandelbrotbench_avx ${QT_LIBRARIES} ${Vc_LIBRARIES})
#   endif()
#endif()

exec_program(${CMAKE_CXX_COMPILER} ARGS --version OUTPUT_VARIABLE CXX_VERSION)
configure_file(benchmark-all.sh benchmark-all.sh @ONLY)
