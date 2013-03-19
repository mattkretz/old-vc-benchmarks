/*  This file is part of the Vc library.

    Copyright (C) 2009-2012 Matthias Kretz <kretz@kde.org>

    Vc is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Vc is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Vc.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "benchmark.h"
#include <Vc/Vc>
#include <cstdlib>

using Vc::float_v;
using Vc::short_v;
using Vc::sfloat_v;
using Vc::double_v;

template<typename V> class GatherBenchmark
{
    typedef typename V::EntryType T;
    typedef typename V::IndexType I;
    typedef typename V::Mask M;
    enum {
        Repetitions = 1024 * 128
    };

    static T data[2048];

    NOINLINE static void fullMask(const int indexSpread)
    {
        benchmark_loop(Benchmark("full mask", Repetitions * V::Size * 4, "Value")) {
            M mask(Vc::One);
            I indexes = I::Random() & I(indexSpread);
            benchmark_restart();
            for (int i = 0; i < Repetitions; ++i) {
                asm("":"+m"(indexes)); keepResultsDirty(mask); V tmp0(&data[0], indexes, mask); keepResults(tmp0);
                asm("":"+m"(indexes)); keepResultsDirty(mask); V tmp1(&data[1], indexes, mask); keepResults(tmp1);
                asm("":"+m"(indexes)); keepResultsDirty(mask); V tmp2(&data[2], indexes, mask); keepResults(tmp2);
                asm("":"+m"(indexes)); keepResultsDirty(mask); V tmp3(&data[3], indexes, mask); keepResults(tmp3);
            }
        }
    }

    NOINLINE static void randomMask(const int indexSpread)
    {
        benchmark_loop(Benchmark("random mask", Repetitions * V::Size * 4, "Value")) {
            M mask0 = V::Random() < V::Random();
            M mask1 = V::Random() < V::Random();
            M mask2 = V::Random() < V::Random();
            M mask3 = V::Random() < V::Random();
            I indexes = I::Random() & I(indexSpread);
            benchmark_restart();
            for (int i = 0; i < Repetitions; ++i) {
                asm("":"+m"(indexes)); keepResultsDirty(mask0); V tmp0(&data[0], indexes, mask0); keepResults(tmp0);
                asm("":"+m"(indexes)); keepResultsDirty(mask1); V tmp1(&data[1], indexes, mask1); keepResults(tmp1);
                asm("":"+m"(indexes)); keepResultsDirty(mask2); V tmp2(&data[2], indexes, mask2); keepResults(tmp2);
                asm("":"+m"(indexes)); keepResultsDirty(mask3); V tmp3(&data[3], indexes, mask3); keepResults(tmp3);
            }
        }
    }

    NOINLINE static void zeroMask(const int indexSpread)
    {
        benchmark_loop(Benchmark("zero mask", Repetitions * V::Size * 4, "Value")) {
            M mask(Vc::Zero);
            I indexes = I::Random() & I(indexSpread);
            benchmark_restart();
            for (int i = 0; i < Repetitions; ++i) {
                asm("":"+m"(indexes)); keepResultsDirty(mask); V tmp0(&data[0], indexes, mask); keepResults(tmp0);
                asm("":"+m"(indexes)); keepResultsDirty(mask); V tmp1(&data[1], indexes, mask); keepResults(tmp1);
                asm("":"+m"(indexes)); keepResultsDirty(mask); V tmp2(&data[2], indexes, mask); keepResults(tmp2);
                asm("":"+m"(indexes)); keepResultsDirty(mask); V tmp3(&data[3], indexes, mask); keepResults(tmp3);
            }
        }
    }

    NOINLINE static void withoutMask(const int indexSpread)
    {
        benchmark_loop(Benchmark("without mask", Repetitions * V::Size * 4, "Value")) {
            I indexes = I::Random() & I(indexSpread);
            benchmark_restart();
            for (int i = 0; i < Repetitions; ++i) {
                asm("":"+m"(indexes)); V tmp0(&data[0], indexes); keepResults(tmp0);
                asm("":"+m"(indexes)); V tmp1(&data[1], indexes); keepResults(tmp1);
                asm("":"+m"(indexes)); V tmp2(&data[2], indexes); keepResults(tmp2);
                asm("":"+m"(indexes)); V tmp3(&data[3], indexes); keepResults(tmp3);
            }
        }
    }

public:
    static void run()
    {
        for (int i = 0; i <= 2048 - V::Size; i += V::Size) {
            V::Random().store(&data[i], Vc::Unaligned);
        }

        int indexSpreads[] = { 1, 4, 16, 1024 };
        for (int indexSpreadIt = 0; indexSpreadIt < sizeof(indexSpreads)/sizeof(int); ++indexSpreadIt) {
            const int indexSpread = indexSpreads[indexSpreadIt] - 1;
            std::stringstream ss;
            ss << indexSpread + 1;
            Benchmark::setColumnData("index spread", ss.str());

            fullMask(indexSpread);
            randomMask(indexSpread);
            zeroMask(indexSpread);
            withoutMask(indexSpread);
        }
    }
};

template<>  float GatherBenchmark< float_v>::data[2048] = {};
template<>  float GatherBenchmark<sfloat_v>::data[2048] = {};
template<>  short GatherBenchmark< short_v>::data[2048] = {};
template<> double GatherBenchmark<double_v>::data[2048] = {};

int bmain()
{
    Benchmark::addColumn("datatype");
    Benchmark::addColumn("index spread");

    Benchmark::setColumnData("datatype",  "float_v"); GatherBenchmark< float_v>::run();
    Benchmark::setColumnData("datatype",  "short_v"); GatherBenchmark< short_v>::run();
    Benchmark::setColumnData("datatype", "sfloat_v"); GatherBenchmark<sfloat_v>::run();
    Benchmark::setColumnData("datatype", "double_v"); GatherBenchmark<double_v>::run();

    return 0;
}
