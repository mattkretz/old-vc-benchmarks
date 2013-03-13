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

#include <cstdlib>

using namespace Vc;

// to test
// a) conditional (masked) assignment
// b) masked ops (like flops.cpp but with masks)

// global (not file-static!) variable keeps the compiler from identifying the benchmark as dead code
int blackHole = 1;

int nextPowerOf2(int x)
{
    if ((x & (x - 1)) == 0) {
        return x;
    }
    int shift = 1;
    while (x >> shift) {
        ++shift;
    }
    return 1 << shift;
}

template<typename Vector> struct CondAssignment
{
    typedef typename Vector::Mask Mask;
    typedef typename Vector::EntryType Scalar;

    enum {
        Repetitions = 1024 * 1024,
        OuterFactor = 100
    };
    static void run()
    {
        const Vector one(One);

        benchmark_loop(Benchmark("Conditional Assignment", Vector::Size * Repetitions * 4, "Op")) {
            Mask mask0 = Vector::Random() < Vector::Random();
            Mask mask1 = Vector::Random() < Vector::Random();
            Mask mask2 = Vector::Random() < Vector::Random();
            Mask mask3 = Vector::Random() < Vector::Random();
            Vector tmp0 = Vector::Random();
            Vector tmp1 = Vector::Random();
            Vector tmp2 = Vector::Random();
            Vector tmp3 = Vector::Random();
            benchmark_restart();
            for (int i = 0; i < Repetitions; ++i) {
                keepResultsDirty(mask0, mask1, mask2, mask3);
                tmp0(mask0) = one;
                tmp1(mask1) = one;
                tmp2(mask2) = one;
                tmp3(mask3) = one;
            }
            keepResults(tmp0, tmp1, tmp2, tmp3);
        }
        benchmark_loop(Benchmark("Masked Pre-Increment", Vector::Size * Repetitions * 4, "Op")) {
            Mask mask0 = Vector::Random() < Vector::Random();
            Mask mask1 = Vector::Random() < Vector::Random();
            Mask mask2 = Vector::Random() < Vector::Random();
            Mask mask3 = Vector::Random() < Vector::Random();
            Vector tmp0 = Vector::Random();
            Vector tmp1 = Vector::Random();
            Vector tmp2 = Vector::Random();
            Vector tmp3 = Vector::Random();
            benchmark_restart();
            for (int i = 0; i < Repetitions; ++i) {
                keepResultsDirty(mask0, mask1, mask2, mask3);
                ++tmp0(mask0);
                ++tmp1(mask1);
                ++tmp2(mask2);
                ++tmp3(mask3);
            }
            keepResults(tmp0, tmp1, tmp2, tmp3);
        }
        benchmark_loop(Benchmark("Masked Post-Decrement", Vector::Size * Repetitions * 4, "Op")) {
            Mask mask0 = Vector::Random() < Vector::Random();
            Mask mask1 = Vector::Random() < Vector::Random();
            Mask mask2 = Vector::Random() < Vector::Random();
            Mask mask3 = Vector::Random() < Vector::Random();
            Vector tmp0 = Vector::Random();
            Vector tmp1 = Vector::Random();
            Vector tmp2 = Vector::Random();
            Vector tmp3 = Vector::Random();
            benchmark_restart();
            for (int i = 0; i < Repetitions; ++i) {
                keepResultsDirty(mask0, mask1, mask2, mask3);
                tmp0(mask0)--;
                tmp1(mask1)--;
                tmp2(mask2)--;
                tmp3(mask3)--;
            }
            keepResults(tmp0, tmp1, tmp2, tmp3);
        }
        const Vector x(3);
        benchmark_loop(Benchmark("Masked Multiply-Masked Add", Vector::Size * Repetitions * 8, "Op")) {
            Mask mask0 = Vector::Random() < Vector::Random();
            Mask mask1 = Vector::Random() < Vector::Random();
            Mask mask2 = Vector::Random() < Vector::Random();
            Mask mask3 = Vector::Random() < Vector::Random();
            Vector tmp0 = Vector::Random();
            Vector tmp1 = Vector::Random();
            Vector tmp2 = Vector::Random();
            Vector tmp3 = Vector::Random();
            benchmark_restart();
            for (int i = 0; i < Repetitions; ++i) {
                keepResultsDirty(mask0, mask1, mask2, mask3);
                tmp0(mask0) *= x;
                tmp1(mask1) *= x;
                tmp2(mask2) *= x;
                tmp3(mask3) *= x;
                tmp0(mask0) += one;
                tmp1(mask1) += one;
                tmp2(mask2) += one;
                tmp3(mask3) += one;
            }
            keepResults(tmp0, tmp1, tmp2, tmp3);
        }
        benchmark_loop(Benchmark("Masked Multiply-Add", Vector::Size * Repetitions * 8, "Op")) {
            Mask mask0 = Vector::Random() < Vector::Random();
            Mask mask1 = Vector::Random() < Vector::Random();
            Mask mask2 = Vector::Random() < Vector::Random();
            Mask mask3 = Vector::Random() < Vector::Random();
            Vector tmp0 = Vector::Random();
            Vector tmp1 = Vector::Random();
            Vector tmp2 = Vector::Random();
            Vector tmp3 = Vector::Random();
            benchmark_restart();
            for (int i = 0; i < Repetitions; ++i) {
                keepResultsDirty(mask0, mask1, mask2, mask3);
                tmp0(mask0) = tmp0 * x + one;
                tmp1(mask1) = tmp1 * x + one;
                tmp2(mask2) = tmp2 * x + one;
                tmp3(mask3) = tmp3 * x + one;
            }
            keepResults(tmp0, tmp1, tmp2, tmp3);
        }
        benchmark_loop(Benchmark("Masked Division", Vector::Size * Repetitions * 4, "Op")) {
            Mask mask0 = Vector::Random() < Vector::Random();
            Mask mask1 = Vector::Random() < Vector::Random();
            Mask mask2 = Vector::Random() < Vector::Random();
            Mask mask3 = Vector::Random() < Vector::Random();
            Vector tmp0 = Vector::Random();
            Vector tmp1 = Vector::Random();
            Vector tmp2 = Vector::Random();
            Vector tmp3 = Vector::Random();
            benchmark_restart();
            for (int i = 0; i < Repetitions; ++i) {
                keepResultsDirty(mask0, mask1, mask2, mask3);
                tmp0(mask0) /= x;
                tmp1(mask1) /= x;
                tmp2(mask2) /= x;
                tmp3(mask3) /= x;
            }
            keepResults(tmp0, tmp1, tmp2, tmp3);
        }
    }
};

int bmain()
{
    Benchmark::addColumn("datatype");
    Benchmark::setColumnData("datatype", "double_v");
    CondAssignment<double_v>::run();
    Benchmark::setColumnData("datatype", "float_v");
    CondAssignment<float_v>::run();
    Benchmark::setColumnData("datatype", "short_v");
    CondAssignment<short_v>::run();
    Benchmark::setColumnData("datatype", "ushort_v");
    CondAssignment<ushort_v>::run();
    Benchmark::setColumnData("datatype", "int_v");
    CondAssignment<int_v>::run();
    Benchmark::setColumnData("datatype", "uint_v");
    CondAssignment<uint_v>::run();
#if VC_IMPL_SSE
    Benchmark::setColumnData("datatype", "sfloat_v");
    CondAssignment<sfloat_v>::run();
#endif
    return 0;
}
