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

#include <Vc/Vc>
#include "benchmark.h"
#include "random.h"
#include <Vc/cpuid.h>

#include <cstdlib>

using namespace Vc;

template<typename Vector> struct Helper
{
    typedef typename Vector::Mask Mask;
    typedef typename Vector::EntryType Scalar;

    enum {
        Repetitions = 1024 * 32
    };

    static void run()
    {
        benchmark_loop(Benchmark("Vc sort", Repetitions, "Call")) {
            Vector input = Vector::Random();
            benchmark_restart();
            for (int i = 0; i < Repetitions; ++i) {
                keepResultsDirty(input);
                Vector tmp = input.sorted();
                keepResults(tmp);
            }
        }
        benchmark_loop(Benchmark("std::sort", Repetitions, "Call")) {
            const Vector rnd = Vector::Random();
            Scalar data[Vector::Size];
            rnd.store(&data[0], Vc::Unaligned);
            benchmark_restart();
            for (int i = 0; i < Repetitions; ++i) {
                asm("":"+m"(data));
                std::sort(&data[0], &data[Vector::Size]);
            }
        }
    }
};

int bmain()
{
    Benchmark::addColumn("datatype");
    Benchmark::setColumnData("datatype", "float_v" ); Helper<float_v >::run();
    Benchmark::setColumnData("datatype", "sfloat_v"); Helper<sfloat_v>::run();
    Benchmark::setColumnData("datatype", "double_v"); Helper<double_v>::run();
    Benchmark::setColumnData("datatype", "int_v"   ); Helper<int_v   >::run();
    Benchmark::setColumnData("datatype", "uint_v"  ); Helper<uint_v  >::run();
    Benchmark::setColumnData("datatype", "short_v" ); Helper<short_v >::run();
    Benchmark::setColumnData("datatype", "ushort_v"); Helper<ushort_v>::run();
    return 0;
}
