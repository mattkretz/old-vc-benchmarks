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
    typedef typename Vector::EntryType Scalar;
    typedef typename Vector::AsArg AsArg;
    enum { Repetitions = 1024*1024 };

    static Vc_ALWAYS_INLINE Vector add(AsArg a, AsArg b) { return a + b; }
    static Vc_ALWAYS_INLINE Vector sub(AsArg a, AsArg b) { return a - b; }
    static Vc_ALWAYS_INLINE Vector mul(AsArg a, AsArg b) { return a * b; }
    static Vc_ALWAYS_INLINE Vector div(AsArg a, AsArg b) { return a / b; }

    template<typename Op> static Vc_ALWAYS_INLINE void runImpl1(const char *name, Op op)
    {
        const Vector arg0 = Vector::Random();
        Vector arg1 = Vector::Random();
        benchmark_loop(Benchmark(name, Vector::Size * Repetitions, "Op")) {
            for (int i = 0; i < Repetitions; ++i) {
                Vector x0 = op(arg0, arg1);
                keepResultsDirty(arg1);
                keepResults(x0);
            }
        }
    }
    template<typename Op> static Vc_ALWAYS_INLINE void runImpl2(const char *name, Op op)
    {
        const Vector arg0 = Vector::Random();
        Vector arg1 = Vector::Random();
        Vector arg2 = Vector::Random();
        benchmark_loop(Benchmark(name, 2 * Vector::Size * Repetitions, "Op")) {
            for (int i = 0; i < Repetitions; ++i) {
                Vector x0 = op(arg0, arg1);
                keepResultsDirty(arg1);
                Vector x1 = op(arg0, arg2);
                keepResultsDirty(arg2);
                keepResults(x0, x1);
            }
        }
    }
    template<typename Op> static Vc_ALWAYS_INLINE void runImpl4(const char *name, Op op)
    {
        const Vector arg0 = Vector::Random();
        Vector arg1 = Vector::Random();
        Vector arg2 = Vector::Random();
        Vector arg3 = Vector::Random();
        Vector arg4 = Vector::Random();
        benchmark_loop(Benchmark(name, 4 * Vector::Size * Repetitions, "Op")) {
            for (int i = 0; i < Repetitions; ++i) {
                Vector x0 = op(arg0, arg1);
                keepResultsDirty(arg1);
                Vector x1 = op(arg0, arg2);
                keepResultsDirty(arg2);
                Vector x2 = op(arg0, arg3);
                keepResultsDirty(arg3);
                Vector x3 = op(arg0, arg4);
                keepResultsDirty(arg4);
                keepResults(x0, x1, x2, x3);
            }
        }
    }
    template<typename Op> static Vc_ALWAYS_INLINE void runImpl8(const char *name, Op op)
    {
        const Vector arg0 = Vector::Random();
        Vector arg1 = Vector::Random();
        Vector arg2 = Vector::Random();
        Vector arg3 = Vector::Random();
        Vector arg4 = Vector::Random();
        Vector arg5 = Vector::Random();
        Vector arg6 = Vector::Random();
        Vector arg7 = Vector::Random();
        Vector arg8 = Vector::Random();
        benchmark_loop(Benchmark(name, 8 * Vector::Size * Repetitions, "Op")) {
            for (int i = 0; i < Repetitions; ++i) {
                Vector x0 = op(arg0, arg1); keepResultsDirty(arg1);
                Vector x1 = op(arg0, arg2); keepResultsDirty(arg2);
                Vector x2 = op(arg0, arg3); keepResultsDirty(arg3);
                Vector x3 = op(arg0, arg4); keepResultsDirty(arg4);
                Vector x4 = op(arg0, arg5); keepResultsDirty(arg5);
                Vector x5 = op(arg0, arg6); keepResultsDirty(arg6);
                Vector x6 = op(arg0, arg7); keepResultsDirty(arg7);
                Vector x7 = op(arg0, arg8); keepResultsDirty(arg8);
                keepResults(x0, x1, x2, x3, x4, x5, x6, x7);
            }
        }
    }

    static void run()
    {
        Benchmark::setColumnData("unrolling", "not unrolled");
        runImpl1("add", add);
        runImpl1("sub", sub);
        runImpl1("mul", mul);
        runImpl1("div", div);
        Benchmark::setColumnData("unrolling", "2x unrolled");
        runImpl2("add", add);
        runImpl2("sub", sub);
        runImpl2("mul", mul);
        runImpl2("div", div);
        Benchmark::setColumnData("unrolling", "4x unrolled");
        runImpl4("add", add);
        runImpl4("sub", sub);
        runImpl4("mul", mul);
        runImpl4("div", div);
        Benchmark::setColumnData("unrolling", "8x unrolled");
        runImpl8("add", add);
        runImpl8("sub", sub);
        runImpl8("mul", mul);
        runImpl8("div", div);
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
