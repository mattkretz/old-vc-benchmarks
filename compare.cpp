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
#include <Vc/cpuid.h>
#include <cstdio>
#include <cstdlib>

using namespace Vc;

static inline void doNothingButDontOptimize()
{
#ifdef __GNUC__
    asm volatile("");
#else
    __asm {};
#endif
}

template<typename Vector> class DoCompares
{
    enum {
        Repetitions = 1024 * 1024
    };
    public:
        static void run()
        {
            typedef typename Vector::Mask M;

            Vector arg0 = Vector::Random();
            Vector arg1 = Vector::Random();
            Vector arg2 = Vector::Random();
            Vector arg3 = Vector::Random();

            benchmark_loop(Benchmark("operator==", Vector::Size * Repetitions * 6.0, "Op")) {
                for (int i = 0; i < Repetitions; ++i) {
                    keepResultsDirty(arg0);
                    keepResultsDirty(arg1);
                    keepResultsDirty(arg2);
                    keepResultsDirty(arg3);
                    const M tmp0 = arg0 == arg1; keepResults(tmp0);
                    const M tmp1 = arg0 == arg2; keepResults(tmp1);
                    const M tmp2 = arg0 == arg3; keepResults(tmp2);
                    const M tmp3 = arg1 == arg2; keepResults(tmp3);
                    const M tmp4 = arg1 == arg3; keepResults(tmp4);
                    const M tmp5 = arg2 == arg3; keepResults(tmp5);
                }
            }
            benchmark_loop(Benchmark("operator<", Vector::Size * Repetitions * 6.0, "Op")) {
                for (int i = 0; i < Repetitions; ++i) {
                    keepResultsDirty(arg0);
                    keepResultsDirty(arg1);
                    keepResultsDirty(arg2);
                    keepResultsDirty(arg3);
                    const M &tmp0 = arg0 < arg1; keepResults(tmp0);
                    const M &tmp1 = arg0 < arg2; keepResults(tmp1);
                    const M &tmp2 = arg0 < arg3; keepResults(tmp2);
                    const M &tmp3 = arg1 < arg2; keepResults(tmp3);
                    const M &tmp4 = arg1 < arg3; keepResults(tmp4);
                    const M &tmp5 = arg2 < arg3; keepResults(tmp5);
                }
            }
            benchmark_loop(Benchmark("(operator<).isFull()", Vector::Size * Repetitions * 6.0, "Op")) {
                for (int i = 0; i < Repetitions; ++i) {
                    keepResultsDirty(arg0);
                    keepResultsDirty(arg1);
                    keepResultsDirty(arg2);
                    keepResultsDirty(arg3);
                    if ((arg0 < arg1).isFull()) doNothingButDontOptimize();
                    if ((arg0 < arg2).isFull()) doNothingButDontOptimize();
                    if ((arg0 < arg3).isFull()) doNothingButDontOptimize();
                    if ((arg1 < arg2).isFull()) doNothingButDontOptimize();
                    if ((arg1 < arg3).isFull()) doNothingButDontOptimize();
                    if ((arg2 < arg3).isFull()) doNothingButDontOptimize();
                }
            }
            benchmark_loop(Benchmark("!(operator<).isEmpty()", Vector::Size * Repetitions * 6.0, "Op")) {
                for (int i = 0; i < Repetitions; ++i) {
                    keepResultsDirty(arg0);
                    keepResultsDirty(arg1);
                    keepResultsDirty(arg2);
                    keepResultsDirty(arg3);
                    if (!(arg0 < arg1).isEmpty()) doNothingButDontOptimize();
                    if (!(arg0 < arg2).isEmpty()) doNothingButDontOptimize();
                    if (!(arg0 < arg3).isEmpty()) doNothingButDontOptimize();
                    if (!(arg1 < arg2).isEmpty()) doNothingButDontOptimize();
                    if (!(arg1 < arg3).isEmpty()) doNothingButDontOptimize();
                    if (!(arg2 < arg3).isEmpty()) doNothingButDontOptimize();
                }
            }
        }
};

int bmain()
{
    Benchmark::addColumn("datatype");

    Benchmark::setColumnData("datatype", "double_v");
    DoCompares<double_v>::run();
    Benchmark::setColumnData("datatype", "float_v");
    DoCompares<float_v>::run();
    Benchmark::setColumnData("datatype", "int_v");
    DoCompares<int_v>::run();
    Benchmark::setColumnData("datatype", "uint_v");
    DoCompares<uint_v>::run();
    Benchmark::setColumnData("datatype", "short_v");
    DoCompares<short_v>::run();
    Benchmark::setColumnData("datatype", "ushort_v");
    DoCompares<ushort_v>::run();
    Benchmark::setColumnData("datatype", "sfloat_v");
    DoCompares<sfloat_v>::run();

    return 0;
}
