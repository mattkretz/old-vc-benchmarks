/*  This file is part of the Vc library. {{{

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

}}}*/

#include <Vc/Vc>
#include "benchmark.h"
#include <Vc/cpuid.h>

#include <cstdlib>

using namespace Vc;

template<typename Vector> struct Helper
{
    typedef typename Vector::Mask Mask;
    typedef typename Vector::EntryType Scalar;

    enum {
        Repetitions = 1024 * 1024,
        opPerSecondFactor = Repetitions * Vector::Size
    };

    Vc_ALWAYS_INLINE void benchmarkFunction(const char *name, Vector (*fun)(const Vector &, const Vector &))/*{{{*/
    {
        Vector a = Vector::Random();
        Vector b = Vector::Random();
        benchmark_loop(Benchmark(name, opPerSecondFactor, "Op")) {
            for (int i = 0; i < Repetitions; ++i) {
                keepResultsDirty(a);
                keepResultsDirty(b);
                Vector tmp = fun(a, b);
                keepResults(tmp);
            }
        }
    }

    Vc_ALWAYS_INLINE void benchmarkFunction(const char *name, Vector (*fun)(const Vector &))
    {
        Vector a = Vector::Random();
        benchmark_loop(Benchmark(name, opPerSecondFactor, "Op")) {
            for (int i = 0; i < Repetitions; ++i) {
                asm("":"+m"(a));
                Vector tmp = fun(a);
                keepResults(tmp);
            }
        }
    }

    Vc_ALWAYS_INLINE void benchmarkFunction(const char *name, Vector (*fun)(Vector))
    {
        Vector a = Vector::Random();
        benchmark_loop(Benchmark(name, opPerSecondFactor, "Op")) {
            for (int i = 0; i < Repetitions; ++i) {
                keepResultsDirty(a);
                Vector tmp = fun(a);
                keepResults(tmp);
            }
        }
    }/*}}}*/

    void run()/*{{{*/
    {
        benchmarkFunction("round", Vc::round);
        benchmarkFunction( "sqrt", Vc::sqrt);
        benchmarkFunction("rsqrt", Vc::rsqrt);
        benchmarkFunction("reciprocal", Vc::reciprocal);
        benchmarkFunction(  "abs", Vc::abs);
        benchmarkFunction(  "sin", Vc::sin);
        benchmarkFunction(  "cos", Vc::cos);
        benchmarkFunction( "asin", Vc::asin);
        benchmarkFunction("floor", Vc::floor);
        benchmarkFunction( "ceil", Vc::ceil);
        benchmarkFunction(  "exp", Vc::exp);
        benchmarkFunction(  "log", Vc::log);
        benchmarkFunction( "log2", Vc::log2);
        benchmarkFunction("log10", Vc::log10);
        benchmarkFunction( "atan", Vc::atan);
        benchmarkFunction("atan2", Vc::atan2);
    }/*}}}*/
};

int bmain()/*{{{*/
{
    Benchmark::addColumn("datatype");
    Benchmark::setColumnData("datatype", "float_v");
    Helper<float_v>().run();
    Benchmark::setColumnData("datatype", "sfloat_v");
    Helper<sfloat_v>().run();
    Benchmark::setColumnData("datatype", "double_v");
    Helper<double_v>().run();
    return 0;
}/*}}}*/

// vim: foldmethod=marker
