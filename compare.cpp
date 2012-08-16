/*  This file is part of the Vc library.

    Copyright (C) 2009-2011 Matthias Kretz <kretz@kde.org>

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
        Factor2 = 128
    };
    public:
        static void run()
        {
            const int Factor = CpuId::L1Data() / (sizeof(Vector) * 2); // half L1
            Vector *a = new Vector[Factor + 3];
#ifndef VC_BENCHMARK_NO_MLOCK
            mlock(a, (Factor + 3) * sizeof(Vector));
#endif
            for (int i = 0; i < Factor + 3; ++i) {
                a[i] = PseudoRandom<Vector>::next();
            }

            typedef typename Vector::Mask M;

            {
                Benchmark timer("operator==", Vector::Size * Factor * Factor2 * 6.0, "Op");
                while (timer.wantsMoreDataPoints()) {
                    timer.Start();
                    for (int j = 0; j < Factor2; ++j) {
                        for (int i = 0; i < Factor; ++i) {
                            const M &tmp0 = a[i + 0] == a[i + 1]; keepResults(tmp0);
                            const M &tmp1 = a[i + 0] == a[i + 2]; keepResults(tmp1);
                            const M &tmp2 = a[i + 0] == a[i + 3]; keepResults(tmp2);
                            const M &tmp3 = a[i + 1] == a[i + 2]; keepResults(tmp3);
                            const M &tmp4 = a[i + 1] == a[i + 3]; keepResults(tmp4);
                            const M &tmp5 = a[i + 2] == a[i + 3]; keepResults(tmp5);
                        }
                    }
                    timer.Stop();
                }
                timer.Print();
            }
            {
                Benchmark timer("operator<", Vector::Size * Factor * Factor2 * 6.0, "Op");
                while (timer.wantsMoreDataPoints()) {
                    timer.Start();
                    for (int j = 0; j < Factor2; ++j) {
                        for (int i = 0; i < Factor; ++i) {
                            const M &tmp0 = a[i + 0] < a[i + 1]; keepResults(tmp0);
                            const M &tmp1 = a[i + 0] < a[i + 2]; keepResults(tmp1);
                            const M &tmp2 = a[i + 0] < a[i + 3]; keepResults(tmp2);
                            const M &tmp3 = a[i + 1] < a[i + 2]; keepResults(tmp3);
                            const M &tmp4 = a[i + 1] < a[i + 3]; keepResults(tmp4);
                            const M &tmp5 = a[i + 2] < a[i + 3]; keepResults(tmp5);
                        }
                    }
                    timer.Stop();
                }
                timer.Print();
            }
            {
                Benchmark timer("(operator<).isFull()", Vector::Size * Factor * Factor2 * 6.0, "Op");
                while (timer.wantsMoreDataPoints()) {
                    timer.Start();
                    for (int j = 0; j < Factor2; ++j) {
                        for (int i = 0; i < Factor; ++i) {
                            if ((a[i + 0] < a[i + 1]).isFull()) doNothingButDontOptimize();
                            if ((a[i + 0] < a[i + 2]).isFull()) doNothingButDontOptimize();
                            if ((a[i + 0] < a[i + 3]).isFull()) doNothingButDontOptimize();
                            if ((a[i + 1] < a[i + 2]).isFull()) doNothingButDontOptimize();
                            if ((a[i + 1] < a[i + 3]).isFull()) doNothingButDontOptimize();
                            if ((a[i + 2] < a[i + 3]).isFull()) doNothingButDontOptimize();
                        }
                    }
                    timer.Stop();
                }
                timer.Print();
            }
            {
                Benchmark timer("!(operator<).isEmpty()", Vector::Size * Factor * Factor2 * 6.0, "Op");
                while (timer.wantsMoreDataPoints()) {
                    timer.Start();
                    for (int j = 0; j < Factor2; ++j) {
                        for (int i = 0; i < Factor; ++i) {
                            if (!(a[i + 0] < a[i + 1]).isEmpty()) doNothingButDontOptimize();
                            if (!(a[i + 0] < a[i + 2]).isEmpty()) doNothingButDontOptimize();
                            if (!(a[i + 0] < a[i + 3]).isEmpty()) doNothingButDontOptimize();
                            if (!(a[i + 1] < a[i + 2]).isEmpty()) doNothingButDontOptimize();
                            if (!(a[i + 1] < a[i + 3]).isEmpty()) doNothingButDontOptimize();
                            if (!(a[i + 2] < a[i + 3]).isEmpty()) doNothingButDontOptimize();
                        }
                    }
                    timer.Stop();
                }
                timer.Print();
            }
            delete[] a;
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
