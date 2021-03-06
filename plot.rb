#!/usr/bin/env ruby
# coding:utf-8
require 'tmpdir'

$argv = ARGV
$gnuplotOptions = {# {{{
    :font => 'CM Sans,8',
    :size => '19.55cm,11cm',
    :svg => false
}# }}}
class LabelTranslation #{{{
    def initialize(trans = Hash.new)
        @trans = {
            'interleavedmemorywrapper' => 'InterleavedMemoryWrapper Benchmark',
            'memio' => 'Load/Store Benchmark',
            'arithmetics' => 'Arithmetic Operations Benchmark',
            'arithmetics2' => 'Arithmetic Operations Benchmark (without loads/stores)',
            'flops' => 'Peak Flop Benchmark',
            'gather' => 'Gathers Benchmark',
            'scatter' => 'Scatters Benchmark',
            'mask' => 'Masks Benchmark',
            'compare' => 'Compares Benchmark',
            'math' => 'Math Functions Benchmark',
            'dhryrock' => 'Dhryrock Benchmark (Integer Vectors)',
            'whetrock' => 'Whetrock Benchmark (Floating-Point Vectors)',

            'half L1' => '⅟₂ L1',
            'half L2' => '⅟₂ L2',
            'half L3' => '⅟₂ L3',
            '4x L3' => '4×L3',

            '-nan' => '0',
            'nan' => '0',

            'sse' => 'SSE',
            'sse-mnoavx' => 'SSE (binary ops)',
            'sse-mavx' => 'SSE (ternary ops)',
            'scalar' => 'Scalar',
            'scalar-mnoavx' => 'Scalar (binary ops)',
            'scalar-mavx' => 'Scalar (ternary ops)',
            'avx' => 'AVX',
            'avx-mavx' => 'AVX',
            'avx-mxop' => 'AVX, XOP',
            'avx-mfma4' => 'AVX, FMA4',
            'avx-mxop-mfma4' => 'AVX, XOP, FMA4'
        }.merge(trans)
    end

    def translate(str)
        if str.is_a?(Array)
            return str.map { |x| translate(x) }.join(', ')
        end
        if str =~ /^".*"$/
            tmp = @trans[str[1..-2]]
            return "\"#{tmp}\"" if tmp
        end
        @trans[str] or str
    end
end #}}}
class SortOrder #{{{1
    def initialize(hash = Hash.new)
        @order = hash
        #@order.default = 0
    end
    def [](v)
        v =~ /^"(.*)"$/ and v = $~[1]
        @order.index(v) or v
    end
    def sort(obj)
        begin
            obj.sort_by do |v|
                if v.is_a? Array
                    v.map{|vv| @order.index(vv) or vv}
                else
                    @order.index(v) or v
                end
            end
        rescue ArgumentError
            puts "WARNING: sortOrder outdated"
            obj.sort_by do |v|
                if v.is_a? Array
                    p(v => (v.map{|vv| @order.index(vv)}))
                else
                    p(v => @order.index(v))
                end
                v
            end
        end
    end
end #}}}1
$benchmarks = {
    'interleavedmemorywrapper' => { #{{{1
        :key => 'right top',
        :sort => [:groups, :bars, :clusters],
        :pageColumn => ['datatype', 'Member Count'],
        #:groupColumn => 'Member Count',
        :barColumns => 'benchmark.name',
        :clusterColumns => 'Implementation',
        :dataColumn => 'Bytes/Cycle',
        :labelTranslation => LabelTranslation.new(
            '2' => '2 Entries',
            '3' => '3 Entries',
            '4' => '4 Entries',
            '5' => '5 Entries',
            '6' => '6 Entries',
            '7' => '7 Entries',
            '8' => '8 Entries'
        )
    },
    'memio' => { #{{{1
        :key => 'right top',
        :sort => [:groups, :bars, :clusters],
        :pageColumn => 'MemorySize',
        :groupColumn => 'benchmark.name',
        :barColumns => ['Alignment', 'Implementation'],
        :clusterColumns => 'datatype',
        :dataColumn => 'Bytes/Cycle',
        :labelTranslation => LabelTranslation.new(
            'read' => 'load',
            'write' => 'store',
            'r/w' => 'load \\& store',
            'aligned' => 'Aligned',
            'aligned mem/unaligned instr' => 'Aligned Memory, Unaligned Instruction',
            'unaligned' => 'Unaligned'
        )
    },
    'arithmetics' => { #{{{1
        :sort => [:pages, :bars],
        :pageColumn => 'datatype',
        :groupColumn => 'unrolling',
        :barColumns => 'Implementation',
        :clusterColumns => 'benchmark.name',
        :dataColumn => 'Ops/Cycle',
        :labelTranslation => LabelTranslation.new(
            'arithmetics' => 'Arithmetic Operations'
        ),
        :ylabel => 'Operations / Cycle'
    },
    'arithmetics2' => { #{{{1
        :sort => [:pages, :bars],
        :pageColumn => 'datatype',
        :groupColumn => 'unrolling',
        :barColumns => 'Implementation',
        :clusterColumns => 'benchmark.name',
        :dataColumn => 'Ops/Cycle',
        :labelTranslation => LabelTranslation.new(
            'arithmetics' => 'Arithmetic Operations'
        ),
        :ylabel => 'Operations / Cycle'
    },
    'flops' => { #{{{1
        :sort => [:clusters, :bars],
        :barColumns => 'benchmark.name',
        :clusterColumns => 'Implementation',
        :dataColumn => 'FLOPs/Cycle',
        :labelTranslation => LabelTranslation.new(
            'flops' => 'Peak-Flop Benchmark',
            'asm reference' => 'Assembler',
            'intrinsics reference' => 'Intrinsics',
            'class' => 'Vc'
        ),
        :ylabel => 'Floating-Point Operations / Cycle'
    },
    'gather' => { #{{{1
        :sort => [:bars, :clusters],
        :pageColumn => 'index spread',
        :groupColumn => 'benchmark.name',
        :barColumns => 'Implementation',
        :clusterColumns => 'datatype',
        :dataColumn => 'Values/Cycle'
    },
    'scatter' => { #{{{1
        :sort => [:bars, :clusters],
        :pageColumn => 'index spread',
        :groupColumn => 'benchmark.name',
        :barColumns => 'Implementation',
        :clusterColumns => 'datatype',
        :dataColumn => 'Values/Cycle'
    },
    'mask' => { #{{{1
        :sort => [:bars, :clusters],
        :pageColumn => 'benchmark.name',
        :barColumns => 'datatype',
        :clusterColumns => 'Implementation',
        :dataColumn => 'Ops/Cycle',
        :ylabel => 'Operations / Cycle'
    },
    'compare' => { #{{{1
        :sort => [:bars, :clusters],
        :pageColumn => 'benchmark.name',
        :barColumns => 'datatype',
        :clusterColumns => 'Implementation',
        :dataColumn => 'Ops/Cycle',
        :labelTranslation => LabelTranslation.new(
            'compare' => 'Compare Operations'
        ),
        :ylabel => 'Operations / Cycle'
    },
    'math' => { #{{{1
        :sort => [:pages, :bars, :clusters],
        :pageColumn => 'benchmark.name',
        :barColumns => 'Implementation',
        :clusterColumns => 'datatype',
        :dataColumn => 'Ops/Cycle',
        :ylabel => 'Operations / Cycle'
    },
    'dhryrock' => { #{{{1
        :sort => [:bars],
        :clusterColumns => 'benchmark.name',
        :barColumns => 'Implementation',
        :dataColumn => 'Ops/Cycle',
        :ylabel => 'Operations / Cycle'
    },
    'whetrock' => { #{{{1
        :sort => [:bars],
        :clusterColumns => 'benchmark.name',
        :barColumns => 'Implementation',
        :dataColumn => 'Ops/Cycle',
        :ylabel => 'Operations / Cycle'
    } #}}}1
}
if $argv.include? '--help' or $argv.include? '-h'# {{{
    puts "Usage: plot.rb [<options>] [<list of benchmarks>]"
    puts
    puts "Options:"
    puts "  -h|--help            This message."
    puts "  --debug              Only print gnuplot script to stdout."
    puts "  --font <family,size> Define the font. (default: #{$gnuplotOptions[:font]})"
    puts "  --size <w,h>         Define the page size. (default: #{$gnuplotOptions[:size]})"
    puts "  --svg                Output to SVG instead of PDF"
    puts "  --compare <tarball1> <tarball2>  Plot the differences between two benchmark runs"
    puts
    puts "Available benchmarks:"
    $benchmarks.each do |b|
        puts "  #{b[0]}"
    end
    puts "  mandelbrot"
    exit
end# }}}
if $argv.include? '--font'# {{{
    p = $argv[$argv.index('--font') + 1]
    $argv = $argv - ['--font', p]
    $gnuplotOptions[:font] = p
end
if $argv.include? '--size'
    p = $argv[$argv.index('--size') + 1]
    $argv = $argv - ['--size', p]
    $gnuplotOptions[:size] = p
end
if $argv.include? '--svg'
    $argv = $argv - ['--svg']
    $gnuplotOptions[:svg] = true
end# }}}
$sortOrder = SortOrder.new([ #{{{1
    'sfloat_v',
    'float_v',
    'double_v',
    'int_v',
    'uint_v',
    'short_v',
    'ushort_v',

    'Scalar',
    'Scalar (binary ops)',
    'Scalar (ternary ops)',
    'SSE',
    'SSE (binary ops)',
    'SSE (ternary ops)',
    'AVX',
    'AVX, XOP',
    'AVX, FMA4',
    'AVX, XOP, FMA4',

    'deinterleave (index vector)',
    'deinterleave (successive)',
    'interleave (index vector)',
    'interleave (successive)',
    'normalize vectors (baseline)',
    'normalize interleaved vectors (manually)',
    'normalize interleaved vectors (index vector)',
    'normalize interleaved vectors (successive)',

    'load',
    'store',
    'load \\& store',

    'Aligned, Scalar',
    'Aligned Memory, Unaligned Instruction, Scalar',
    'Unaligned, Scalar',
    'Aligned, Scalar (binary ops)',
    'Aligned Memory, Unaligned Instruction, Scalar (binary ops)',
    'Unaligned, Scalar (binary ops)',
    'Aligned, Scalar (ternary ops)',
    'Aligned Memory, Unaligned Instruction, Scalar (ternary ops)',
    'Unaligned, Scalar (ternary ops)',
    'Aligned, SSE',
    'Aligned Memory, Unaligned Instruction, SSE',
    'Unaligned, SSE',
    'Aligned, SSE (binary ops)',
    'Aligned Memory, Unaligned Instruction, SSE (binary ops)',
    'Unaligned, SSE (binary ops)',
    'Aligned, SSE (ternary ops)',
    'Aligned Memory, Unaligned Instruction, SSE (ternary ops)',
    'Unaligned, SSE (ternary ops)',
    'Aligned, AVX',
    'Aligned Memory, Unaligned Instruction, AVX',
    'Unaligned, AVX',
    'Aligned, AVX, XOP',
    'Aligned Memory, Unaligned Instruction, AVX, XOP',
    'Unaligned, AVX, XOP',
    'Aligned, AVX, FMA4',
    'Aligned Memory, Unaligned Instruction, AVX, FMA4',
    'Unaligned, AVX, FMA4',
    'Aligned, AVX, XOP, FMA4',
    'Aligned Memory, Unaligned Instruction, AVX, XOP, FMA4',
    'Unaligned, AVX, XOP, FMA4',

    'Vc',
    'Intrinsics',
    'Assembler'
]) #}}}1
class ColumnFilter #{{{1
    def initialize(grep, name_column)
        @name_column = name_column.map {|i| [i[0], i[1], -1]}
        @grep = grep.flatten.map do |v|
            if v.is_a? String then
                "\"#{v}\""
            else
                v
            end
        end
    end

    def prepare(colnames)
        @name_column.map! do |col|
            col[2] = colnames.index col[1]
            fail if col[2] === nil
            col
        end
    end

    def process(row)
        if (@grep.map{|i| i === nil or row.include? i}).reduce(:&)
            @name_column.each do |col|
                yield(col[0], row[col[2]])
            end
        end
    end

    def headers
        @name_column.map { |col| col[0] }
    end
end

class TableRow #{{{1
    def initialize(keys)
        @keys = keys
        @data = Hash.new(0)
    end

    def []=(key, value)
        @data[key] = value
    end

    def [](key)
        @data[key]
    end

    def ==(rhs)
        fail unless rhs.is_a? String
        @keys == rhs
    end

    def fields(headers)
        [@keys] + headers[1..-1].map do |h|
            self[h]
        end
    end
end

class DataParser #{{{1
    def parseFields(fields) #{{{2
        r = Array.new
        fields.each do |f|
            if f =~ /^"(.*)"$/
                r.push $~[1]
            elsif f =~ /^\d*$/
                r.push f.to_i
            else
                r.push f.to_f
            end
        end
        return r;
    end
    def squaredSum(x, y)# {{{
        return Math.sqrt(x * x + y * y)
    end# }}}
    def initialize(bench, tr, dirs = []) #{{{2
        @data = Array.new
        @colnames = Array.new
        versionline = nil
        colheads = nil
        Dir.chdir dirs.empty? ? '.' : dirs[0] do
            Dir.glob("#{bench}_*.dat").each do |filename|
                dat = File.new(filename, "r")
                versionline = dat.readline.strip.match /^Version (\d+)$/
                tmp = $~[1].to_i
                colheads = dat.readline.strip[1..-2]
                if @version === nil
                    @version = tmp
                    @colnames = colheads.split("\"\t\"")
                else
                    fail if @version != tmp
                    fail if @colnames != colheads.split("\"\t\"")
                end
                impl = '"' + filename[bench.length + 1..-5] + '"'
                impl = tr.translate impl
                dat.readlines.each do |line|
                    @data.push(line.strip.split("\t").map{|x| tr.translate x} + [impl])
                end
            end
        end
        @colnames << "Implementation"
        if !dirs.empty?
            Dir.chdir dirs[1] do
                data2 = Array.new
                Dir.glob("#{bench}_*.dat").each do |filename|
                    dat = File.new(File.join(dirs[1], filename), "r")
                    fail unless versionline == dat.readline.strip.match(/^Version (\d+)$/)
                    fail unless colheads == dat.readline.strip[1..-2]
                    impl = tr.translate('"' + filename[bench.length + 1..-5] + '"')
                    dat.readlines.each do |line|
                        data2.push(line.strip.split("\t").map{|x| tr.translate x} + [impl])
                    end
                end
                @data = [@data, data2].transpose.map do |lines|
                    (lines + [@colnames]).transpose.map do |pair|
                        if pair[0] =~ /^"(.*)"$/
                            fail "#{pair[0]} != #{pair[1]}" if pair[0] != pair[1]
                            pair[0]
                        elsif pair[0] =~ /^\d*$/
                            (pair[0].to_i - pair[1].to_i).to_s
                        elsif pair[2] =~ /_stddev$/
                            squaredSum(pair[0].to_f, pair[1].to_f).to_s
                        else
                            (pair[0].to_f - pair[1].to_f).to_s
                        end
                    end
                end
            end
        end
    end

    def empty? #{{{2
        return @data.empty?
    end

    def write(keys, columnFilters, tr) #{{{2
        keys = [keys] unless keys.is_a? Array
        s = ''
        columnFilters.each {|cf| cf.prepare(@colnames)}
        # { ['half L1'] => { 'read' => 1.2, 'write' => 2.3 }, ... }

        keyIndexes = keys.map {|i| @colnames.index i}
        keyIndexes.compact!

        table = Array.new

        @data.each do |row|
            keyValues = tr.translate((keyIndexes.map {|i| row[i]}).join(', ').gsub(/", "/, ', '))
            index = table.index keyValues
            if index === nil
                index = table.size
                table[index] = TableRow.new keyValues
            end
            columnFilters.each do |cf|
                cf.process(row) {|name, value| table[index][name] = value}
            end
        end

        headers = [keys.join(', ')] + (columnFilters.map {|cf| cf.headers}).flatten
        s << '"' << headers.join("\"\t\"") << "\"\n"
        table.each do |row|
            fields = row.fields headers
            s << fields.join("\t") << "\n"
        end
        return s
    end

    def list(columnname) #{{{2
        if columnname.is_a? Array
            i = columnname.map { |x| @colnames.index x }
            contents = @data.map { |x| i.map { |j| x[j][1..-2] } }
            contents.uniq
        else
            i = @colnames.index(columnname)
            if i and not @data.empty?
                contents = @data.map { |x| x[i] }
                contents.uniq.map { |x| x[1..-2] }
            else
                nil
            end
        end
    end
    #}}}2
    def maximumY(col) #{{{2
        i = @colnames.index col
        (@data.map { |row| row[i] }).max
    end #}}}2
    def sort(columns, sortOrder) #{{{2
        columns = [columns] unless columns.is_a? Array
        columns.map! { |i| @colnames.index i }
        #p @data.map{|row| row[columns[0]]}
        @data = @data.sort_by do |v|
            columns.map { |i| sortOrder[v[i]] }
        end
        #p @data.map{|row| row[columns[0]]}
    end #}}}2
    attr_reader :version
end #}}}1
$gnuplot = if $argv.include? '--debug' #{{{1
    $argv = $argv - ['--debug']
    STDOUT
else
    IO.popen("gnuplot", 'w')
end #}}}1
#gnuplot header{{{1
$gnuplot.print <<EOF
set style line  1 lc rgbcolor "#CCCCCC"
set grid y ls 1
set autoscale y

set style line  1 lc rgbcolor "#AF3737"
set style line 15 lc rgbcolor "#AF5537"
set style line  6 lc rgbcolor "#AF7337"
set style line 20 lc rgbcolor "#AF9137"
set style line 11 lc rgbcolor "#AFAF37"

set style line  2 lc rgbcolor "#91AF37"
set style line 16 lc rgbcolor "#73AF37"
set style line  7 lc rgbcolor "#54963E"
set style line 21 lc rgbcolor "#37AF37"
set style line 12 lc rgbcolor "#37AF55"

set style line  3 lc rgbcolor "#37AF73"
set style line 17 lc rgbcolor "#37AF91"
set style line  8 lc rgbcolor "#37AFAF"
set style line 22 lc rgbcolor "#3791AF"
set style line 13 lc rgbcolor "#3773AF"

set style line  4 lc rgbcolor "#3755AF"
set style line 18 lc rgbcolor "#3737AF"
set style line  9 lc rgbcolor "#5537AF"
set style line 23 lc rgbcolor "#7337AF"
set style line 14 lc rgbcolor "#AF37AF"

set style line  5 lc rgbcolor "#AF3791"
set style line 19 lc rgbcolor "#AF3773"
set style line 10 lc rgbcolor "#AF3755"
set style line 24 lc rgbcolor "#737373"

set style increment user

EOF
if $gnuplotOptions[:svg]
    $gnuplot.puts "set terminal svg noenhanced size #{$gnuplotOptions[:size]} font \"#{$gnuplotOptions[:font]}\""
else
    $gnuplot.puts "set terminal pdf color noenhanced font \"#{$gnuplotOptions[:font]}\" size #{$gnuplotOptions[:size]}"
end
$gnuplot.print <<EOF
set pointsize 0.6
set bars fullwidth
set style histogram errorbars gap 1 lw 1
set style data histogram
set style fill transparent solid 0.85 border -1
set border 10       # only lines on the left and right
#set boxwidth 0.91      # width of the bars
set xtics scale 0 # no tics on below histogram bars
set ytics scale 0
set y2tics scale 0
set bmargin 3.5

#set yrange [0:36]
#set xtics nomirror rotate by -45 scale 0
EOF
#}}}1
# ##### MAIN: process benchmarks {{{1
$pdfs = Array.new
def processBenchmark(tmpdirs = [])#{{{
    ($argv.empty? ? $benchmarks : ($argv - ['mandelbrot'])).each do |bench|#{{{
        if bench.is_a? Array
            opt = bench[1]
            bench = bench[0]
        else
            opt = $benchmarks[bench]
        end
        labelTranslation = opt[:labelTranslation] ? opt[:labelTranslation] : LabelTranslation.new

        parser = DataParser.new(bench, labelTranslation, tmpdirs)
        next if parser.empty?

        sort = opt[:sort] ? opt[:sort] : Array.new
        key = opt[:key] ? opt[:key] : 'left top'

        col = opt[:dataColumn]
        if parser.version == 3 and col.match /^([^\/]+)s\/([^\/]+)$/ then
            col = $~[1] + '/' + $~[2] + 's'
        end
        maxy = parser.maximumY col

        pdffile = (opt[:outname] or bench) + ($gnuplotOptions[:svg] ? '.svg' : '.pdf')
        $pdfs << pdffile
        $gnuplot.print <<EOF
#set yrange [0:#{maxy}]
set yrange [#{tmpdirs.empty? ? '0' : '*'}:*]
set output "#{pdffile}"
set ylabel "#{opt[:ylabel] or opt[:dataColumn].sub /\//, ' / '}"
set key #{key}
EOF

        pageNames = parser.list(opt[:pageColumn])
        pageNames = [nil] if pageNames === nil
        pageNames = $sortOrder.sort pageNames if sort.include? :pages

        groupNames = parser.list(opt[:groupColumn])
        groupNames = [nil] if groupNames === nil
        groupNames.map! { |x| [labelTranslation.translate(x), x]}
        groupNames = $sortOrder.sort groupNames if sort.include? :groups

        titleNames = parser.list(opt[:barColumns])
        titleNames.map! { |x| [labelTranslation.translate(x), x]}
        titleNames = $sortOrder.sort titleNames if sort.include? :bars

        clusterNames = parser.list(opt[:clusterColumns])
        parser.sort opt[:clusterColumns], $sortOrder if sort.include? :clusters

        pageNames.each do |page|# {{{
            data = ''
            gnuplot_print = Array.new
            at = 0
            groupNames.each do |group|
                filters = Array.new
                titleNames.each do |title|
                    filters \
                        << ColumnFilter.new([page, group[1], title[1]],
                                            [[title[0], col]]) \
                        << ColumnFilter.new([page, group[1], title[1]],
                                            [[title[0] + ' stddev', col + '_stddev']])
                end
                tmp = parser.write(opt[:clusterColumns], filters, labelTranslation) + "e\n"
                tmp = tmp[tmp.index("\n")+1..-1] if at > 0
                titleNames.size.times { data << tmp }

                if group[1] != nil
                    gnuplot_print << "  newhistogram \" \\r#{group[0]}\" at #{at}"
                end
                1.upto(titleNames.size) do |i|
                    i2 = i * 2
                    gnuplot_print << "  '-' using #{i2}:#{i2+1}:xtic(1) lt #{i} " +
                    if at == 0
                        "title columnheader(#{i2})"
                    else
                        "notitle"
                    end
                end
                at += clusterNames.size + 2.0 / (titleNames.size + 1)
            end
            pagetitle = labelTranslation.translate(bench)
            if page
                pagetitle += "\\n" + labelTranslation.translate(page)
            end
            $gnuplot.print <<EOF
set title "#{pagetitle}"
plot \
#{gnuplot_print.join(", \\\n")}
#{data}
EOF
        end# }}}
    end #}}}
    if $argv.empty? or $argv.include? "mandelbrot" #{{{1
        tr = LabelTranslation.new
        mandeldat = Dir.glob("mandelbrot_*.dat").map { |filename| [filename, "Vc::" + tr.translate(filename["mandelbrot_".length..-5])] }

        if not mandeldat.empty?
            pdffile = 'mandelbrot.pdf'
            $pdfs << pdffile

            $gnuplot.print <<EOF
set ytics auto
set xtics 100

set terminal pdf color enhanced font "CM Sans,5" size 16cm,9cm
set pointsize 0.6
set output "#{pdffile}"
set style data linespoints
set key left top

set xlabel "width/3 = height/2 [pixels]"

#set tmargin 0.3
#set lmargin 8
#set rmargin 2.5
#set bmargin 3.06
#
#set xrange [0:700]

set title "Mandelbrot Benchmark"
set ylabel "runtime [10^9 cycles]"
plot \\
'#{mandeldat[0][0]}' using 1:($3/10**9) title "builtin", \\
#{(mandeldat.map {|x| "'#{x[0]}' using 1:($2/10**9) title \"#{x[1]}\""}).join(", \\\n")}

set key at -20,2.4
set ylabel "speedup"
plot \\
#{(mandeldat.map {|x| "'#{x[0]}' using 1:($3/$2) title \"#{x[1]} vs. builtin\""}).join(", \\\n")}
EOF
        end
    end #}}}1
    $gnuplot.close

    # all.pdf {{{1
    if $argv.empty?
        metain = if tmpdirs.empty?
            `a2ps -q -M a4 -l 120 --columns=1 --rows=1 metadata -o -|ps2pdf -sPAPERSIZE=a4 - metadata.pdf`
            `pdftk metadata.pdf #{$pdfs.join ' '} cat output tmp.pdf` or fail
            File.new 'metadata', 'r'
        else
            metadatapdfs = Array.new
            tmpdirs.each do |dir|
                metadatapdfs << "#{dir}/metadata.pdf"
                `a2ps -q -M a4 -l 120 --columns=1 --rows=1 #{dir}/metadata -o -|ps2pdf -sPAPERSIZE=a4 - #{dir}/metadata.pdf`
            end
            `pdftk #{metadatapdfs.join ' '} #{$pdfs.join ' '} cat output tmp.pdf` or fail
            $pdfs.each { |f| File.delete f }
            File.new "#{tmpdirs[0]}/metadata", 'r'
        end
        metaout = File.new 'tmp.txt', 'w'
        title = Array.new
        metain.readlines.each do |line|
            key, value = line.chomp.split(/\t+: +/, 2)
            title << value if ['compiler', 'target arch', 'hostname', 'model name'].include? key
        end
        metaout.puts "InfoBegin"
        metaout.puts "InfoKey: Title"
        metaout.puts "InfoValue: Vc Benchmarks - #{title.join ' '}"
        metaout.puts "InfoBegin"
        metaout.puts "InfoKey: Creator"
        metaout.puts "InfoValue: Vc http://compeng.uni-frankfurt.de/?vc"
        metaout.puts "InfoBegin"
        metaout.puts "InfoKey: Producer"
        metaout.puts "InfoValue: Vc's plot.rb and #{`gnuplot --version`}"
        metaout.puts "InfoBegin"
        metaout.puts "InfoKey: Author"
        metaout.puts "InfoValue: #{ENV['USER']}"
        metain.close
        metaout.close
        `pdftk tmp.pdf update_info tmp.txt output all.pdf`
        File.delete('tmp.txt')
        File.delete('tmp.pdf')
    end #}}}1
end# }}}

if $argv.include? '--compare'# {{{
    i = $argv.index('--compare')
    tarballs = $argv[i + 1, 2]
    $argv = $argv - (['--compare'] + tarballs)
    tarballs.map! { |f| File.expand_path f }

    # 1. untar the two tarballs at a temporary location
    if File.exist?(tarballs[0]) and File.exist?(tarballs[1])
        olddir = Dir.getwd
        Dir.mktmpdir('vc-benchmarks-plot') do |tmpdir0|
            system 'tar', '--strip-components=1', '-C', tmpdir0, '-xf', tarballs[0]
            Dir.mktmpdir('vc-benchmarks-plot') do |tmpdir1|
                system 'tar', '--strip-components=1', '-C', tmpdir1, '-xf', tarballs[1]
                processBenchmark [tmpdir0, tmpdir1]
            end
        end
    else
        fail "Error: could not find the specified tarballs"
    end
else
    processBenchmark
end# }}}

# vim: sw=4 et foldmethod=marker
