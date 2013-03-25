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
using Vc::int_v;

template<typename V> class ScatterBenchmark
{
    typedef typename V::EntryType T;
    typedef typename V::IndexType I;
    typedef typename V::Mask M;
    enum {
        Repetitions = 1024 * 128
    };
    static T data[2048];

public:
    static void run()
    {
        int indexSpreads[] = { 1, 4, 16, 1024 };
        for (int indexSpreadIt = 0; indexSpreadIt < sizeof(indexSpreads)/sizeof(int); ++indexSpreadIt) {
            const int indexSpread = indexSpreads[indexSpreadIt] - 1;
            std::stringstream ss;
            ss << indexSpread + 1;
            Benchmark::setColumnData("index spread", ss.str());

            benchmark_loop(Benchmark("full mask", Repetitions * V::Size * 4, "Value")) {
                M mask(Vc::One);
                I indexes = I::Random() & I(indexSpread);
                V rnd = V::Random();
                benchmark_restart();
                for (int i = 0; i < Repetitions; ++i) {
                    asm("":"+m"(indexes)); keepResultsDirty(mask); keepResultsDirty(rnd); rnd.scatter(&data[0], indexes, mask);
                    asm("":"+m"(indexes)); keepResultsDirty(mask); keepResultsDirty(rnd); rnd.scatter(&data[1], indexes, mask);
                    asm("":"+m"(indexes)); keepResultsDirty(mask); keepResultsDirty(rnd); rnd.scatter(&data[2], indexes, mask);
                    asm("":"+m"(indexes)); keepResultsDirty(mask); keepResultsDirty(rnd); rnd.scatter(&data[3], indexes, mask);
                }
            }
            benchmark_loop(Benchmark("random mask", Repetitions * V::Size * 4, "Value")) {
                M mask0 = V::Random() < V::Random();
                M mask1 = V::Random() < V::Random();
                M mask2 = V::Random() < V::Random();
                M mask3 = V::Random() < V::Random();
                I indexes = I::Random() & I(indexSpread);
                V rnd = V::Random();
                benchmark_restart();
                for (int i = 0; i < Repetitions; ++i) {
                    asm("":"+m"(indexes)); keepResultsDirty(mask0); keepResultsDirty(rnd); rnd.scatter(&data[0], indexes, mask0);
                    asm("":"+m"(indexes)); keepResultsDirty(mask1); keepResultsDirty(rnd); rnd.scatter(&data[1], indexes, mask1);
                    asm("":"+m"(indexes)); keepResultsDirty(mask2); keepResultsDirty(rnd); rnd.scatter(&data[2], indexes, mask2);
                    asm("":"+m"(indexes)); keepResultsDirty(mask3); keepResultsDirty(rnd); rnd.scatter(&data[3], indexes, mask3);
                }
            }
            benchmark_loop(Benchmark("zero mask", Repetitions * V::Size * 4, "Value")) {
                M mask(Vc::Zero);
                I indexes = I::Random() & I(indexSpread);
                V rnd = V::Random();
                benchmark_restart();
                for (int i = 0; i < Repetitions; ++i) {
                    asm("":"+m"(indexes)); keepResultsDirty(mask); keepResultsDirty(rnd); rnd.scatter(&data[0], indexes, mask);
                    asm("":"+m"(indexes)); keepResultsDirty(mask); keepResultsDirty(rnd); rnd.scatter(&data[1], indexes, mask);
                    asm("":"+m"(indexes)); keepResultsDirty(mask); keepResultsDirty(rnd); rnd.scatter(&data[2], indexes, mask);
                    asm("":"+m"(indexes)); keepResultsDirty(mask); keepResultsDirty(rnd); rnd.scatter(&data[3], indexes, mask);
                }
            }
            benchmark_loop(Benchmark("without mask", Repetitions * V::Size * 4, "Value")) {
                I indexes = I::Random() & I(indexSpread);
                V rnd = V::Random();
                benchmark_restart();
                for (int i = 0; i < Repetitions; ++i) {
                    asm("":"+m"(indexes)); keepResultsDirty(rnd); rnd.scatter(&data[0], indexes);
                    asm("":"+m"(indexes)); keepResultsDirty(rnd); rnd.scatter(&data[1], indexes);
                    asm("":"+m"(indexes)); keepResultsDirty(rnd); rnd.scatter(&data[2], indexes);
                    asm("":"+m"(indexes)); keepResultsDirty(rnd); rnd.scatter(&data[3], indexes);
                }
            }
        }
    }
};

template<> float ScatterBenchmark<float_v>::data[2048] = { 0.f };
template<> short ScatterBenchmark<short_v>::data[2048] = { 0 };
template<> int   ScatterBenchmark<  int_v>::data[2048] = { 0 };
template<> float ScatterBenchmark<sfloat_v>::data[2048] = { 0.f };
template<> double ScatterBenchmark<double_v>::data[2048] = { 0. };

int bmain()
{
    Benchmark::addColumn("datatype");
    Benchmark::addColumn("index spread");

    Benchmark::setColumnData("datatype",  "float_v"); ScatterBenchmark< float_v>::run();
    Benchmark::setColumnData("datatype",  "short_v"); ScatterBenchmark< short_v>::run();
    Benchmark::setColumnData("datatype", "sfloat_v"); ScatterBenchmark<sfloat_v>::run();
    Benchmark::setColumnData("datatype", "double_v"); ScatterBenchmark<double_v>::run();
    Benchmark::setColumnData("datatype",    "int_v"); ScatterBenchmark<   int_v>::run();

    return 0;
}
