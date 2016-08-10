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
#include <Vc/limits>
#include "benchmark.h"
#include <Vc/cpuid.h>

#include <cstdlib>

using namespace Vc;
using sfloat_v = Vc::SimdArray<float, short_v::size()>;

template<typename Vector> struct Arithmetics
{
    enum : int { Repetitions = 1024*1024 };

    template<typename Op> static Vc_ALWAYS_INLINE void runImpl1(const char *name)
    {
        Op op;
        const Vector rnd = Vector::Random();
        benchmark_loop(Benchmark(name, Vector::Size * Repetitions, "Op")) {
            Vector arg0 = rnd;
            Vector arg1 = rnd;
            for (int i = 0; i < Repetitions; ++i) {
                keepResults(arg0);  // ensure it's in a register
                Vector r0 = op(arg0, arg1);
                // GCC 5.2 (at least) requires the following order, otherwise it introduces two
                // unnecessary vmovaps instructions into the measured loop
                keepResultsDirty(arg1);
                keepResults(r0);
            }
        }
    }
    template<typename Op> static Vc_ALWAYS_INLINE void runImpl2(const char *name)
    {
        Op op;
        const Vector arg0 = Vector::Random();
        benchmark_loop(Benchmark(name, 2 * Vector::Size * Repetitions, "Op")) {
            Vector arg1 = arg0;
            for (int i = 0; i < Repetitions; ++i) {
                keepResults(arg0);  // ensure it's in a register
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
            }
        }
    }
    template<typename Op> static Vc_ALWAYS_INLINE void runImpl4(const char *name)
    {
        Op op;
        const Vector arg0 = Vector::Random();
        benchmark_loop(Benchmark(name, 4 * Vector::Size * Repetitions, "Op")) {
            Vector arg1 = arg0;
            for (int i = 0; i < Repetitions; ++i) {
                keepResults(arg0);  // ensure it's in a register
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
            }
        }
    }
    template<typename Op> static Vc_ALWAYS_INLINE void runImpl8(const char *name)
    {
        Op op;
        const Vector arg0 = Vector::Random();
        benchmark_loop(Benchmark(name, 8 * Vector::Size * Repetitions, "Op")) {
            Vector arg1 = arg0;
            for (int i = 0; i < Repetitions; ++i) {
                keepResults(arg0);  // ensure it's in a register
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
                keepResultsDirty(arg1); keepResults(op(arg0, arg1));
            }
        }
    }

    static void run()
    {
        Benchmark::setColumnData("unrolling", "not unrolled");
        runImpl1<std::plus<>>("add");
        runImpl1<std::minus<>>("sub");
        runImpl1<std::multiplies<>>("mul");
        runImpl1<std::divides<>>("div");
        Benchmark::setColumnData("unrolling", "2x unrolled");
        runImpl2<std::plus<>>("add");
        runImpl2<std::minus<>>("sub");
        runImpl2<std::multiplies<>>("mul");
        runImpl2<std::divides<>>("div");
        Benchmark::setColumnData("unrolling", "4x unrolled");
        runImpl4<std::plus<>>("add");
        runImpl4<std::minus<>>("sub");
        runImpl4<std::multiplies<>>("mul");
        runImpl4<std::divides<>>("div");
        Benchmark::setColumnData("unrolling", "8x unrolled");
        runImpl8<std::plus<>>("add");
        runImpl8<std::minus<>>("sub");
        runImpl8<std::multiplies<>>("mul");
        runImpl8<std::divides<>>("div");
    }
};

int bmain()
{
    Benchmark::addColumn("datatype");
    Benchmark::addColumn("unrolling");

    Benchmark::setColumnData("datatype", "float_v");
    Arithmetics<float_v>::run();

    Benchmark::setColumnData("datatype", "double_v");
    Arithmetics<double_v>::run();

    Benchmark::setColumnData("datatype", "int_v");
    Arithmetics<int_v>::run();

    Benchmark::setColumnData("datatype", "uint_v");
    Arithmetics<uint_v>::run();

    Benchmark::setColumnData("datatype", "short_v");
    Arithmetics<short_v>::run();

    Benchmark::setColumnData("datatype", "ushort_v");
    Arithmetics<ushort_v>::run();

    Benchmark::setColumnData("datatype", "sfloat_v");
    Arithmetics<sfloat_v>::run();

    return 0;
}
