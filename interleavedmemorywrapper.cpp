/*{{{
    Copyright (C) 2013 Matthias Kretz <kretz@kde.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

}}}*/

#include "benchmark.h"
#include <vector>
#include <Vc/cpuid.h>

using Vc::float_v;
using sfloat_v = Vc::SimdArray<float, short_v::size()>;
using Vc::double_v;
using Vc::int_v;
using Vc::short_v;

namespace std
{
    int_v sqrt(int_v::AsArg x) {
        return static_cast<int_v>(sqrt(static_cast<float_v>(x)));
    }
    short_v sqrt(short_v::AsArg x) {
        return static_cast<short_v>(sqrt(static_cast<sfloat_v>(x)));
    }
} // namespace std

template<int Count, typename V, typename T, typename I> struct NormalizeHelper;
template<typename V, typename T, typename I> struct NormalizeHelper<2, V, T, I> { static void impl(T &wrapper, const I &i) { V a, b;                   (a, b                  ) = wrapper[i]; const V factor = V::One() / std::sqrt(a*a + b*b                                    ); wrapper[i] = (a*factor, b*factor                                                            ); } };
template<typename V, typename T, typename I> struct NormalizeHelper<3, V, T, I> { static void impl(T &wrapper, const I &i) { V a, b, c;                (a, b, c               ) = wrapper[i]; const V factor = V::One() / std::sqrt(a*a + b*b + c*c                              ); wrapper[i] = (a*factor, b*factor, c*factor                                                  ); } };
template<typename V, typename T, typename I> struct NormalizeHelper<4, V, T, I> { static void impl(T &wrapper, const I &i) { V a, b, c, d;             (a, b, c, d            ) = wrapper[i]; const V factor = V::One() / std::sqrt(a*a + b*b + c*c + d*d                        ); wrapper[i] = (a*factor, b*factor, c*factor, d*factor                                        ); } };
template<typename V, typename T, typename I> struct NormalizeHelper<5, V, T, I> { static void impl(T &wrapper, const I &i) { V a, b, c, d, e;          (a, b, c, d, e         ) = wrapper[i]; const V factor = V::One() / std::sqrt(a*a + b*b + c*c + d*d + e*e                  ); wrapper[i] = (a*factor, b*factor, c*factor, d*factor, e*factor                              ); } };
template<typename V, typename T, typename I> struct NormalizeHelper<6, V, T, I> { static void impl(T &wrapper, const I &i) { V a, b, c, d, e, f;       (a, b, c, d, e, f      ) = wrapper[i]; const V factor = V::One() / std::sqrt(a*a + b*b + c*c + d*d + e*e + f*f            ); wrapper[i] = (a*factor, b*factor, c*factor, d*factor, e*factor, f*factor                    ); } };
template<typename V, typename T, typename I> struct NormalizeHelper<7, V, T, I> { static void impl(T &wrapper, const I &i) { V a, b, c, d, e, f, g;    (a, b, c, d, e, f, g   ) = wrapper[i]; const V factor = V::One() / std::sqrt(a*a + b*b + c*c + d*d + e*e + f*f + g*g      ); wrapper[i] = (a*factor, b*factor, c*factor, d*factor, e*factor, f*factor, g*factor          ); } };
template<typename V, typename T, typename I> struct NormalizeHelper<8, V, T, I> { static void impl(T &wrapper, const I &i) { V a, b, c, d, e, f, g, h; (a, b, c, d, e, f, g, h) = wrapper[i]; const V factor = V::One() / std::sqrt(a*a + b*b + c*c + d*d + e*e + f*f + g*g + h*h); wrapper[i] = (a*factor, b*factor, c*factor, d*factor, e*factor, f*factor, g*factor, h*factor); } };
template<int Count, typename V, typename T, typename I> static void normalize(T &wrapper, const I &i) { NormalizeHelper<Count, V, T, I>::impl(wrapper, i); }

template<int Count, typename V, typename T, typename I> struct DeinterleaveHelper;
template<typename V, typename T, typename I> struct DeinterleaveHelper<2, V, T, I> { static void impl(const T &wrapper, const I &i) { V a, b;                   (a, b) = wrapper[i];                   keepResults(a, b); } };
template<typename V, typename T, typename I> struct DeinterleaveHelper<3, V, T, I> { static void impl(const T &wrapper, const I &i) { V a, b, c;                (a, b, c) = wrapper[i];                keepResults(a, b, c); } };
template<typename V, typename T, typename I> struct DeinterleaveHelper<4, V, T, I> { static void impl(const T &wrapper, const I &i) { V a, b, c, d;             (a, b, c, d) = wrapper[i];             keepResults(a, b, c, d); } };
template<typename V, typename T, typename I> struct DeinterleaveHelper<5, V, T, I> { static void impl(const T &wrapper, const I &i) { V a, b, c, d, e;          (a, b, c, d, e) = wrapper[i];          keepResults(a, b, c, d, e); } };
template<typename V, typename T, typename I> struct DeinterleaveHelper<6, V, T, I> { static void impl(const T &wrapper, const I &i) { V a, b, c, d, e, f;       (a, b, c, d, e, f) = wrapper[i];       keepResults(a, b, c, d, e, f); } };
template<typename V, typename T, typename I> struct DeinterleaveHelper<7, V, T, I> { static void impl(const T &wrapper, const I &i) { V a, b, c, d, e, f, g;    (a, b, c, d, e, f, g) = wrapper[i];    keepResults(a, b, c, d, e, f, g); } };
template<typename V, typename T, typename I> struct DeinterleaveHelper<8, V, T, I> { static void impl(const T &wrapper, const I &i) { V a, b, c, d, e, f, g, h; (a, b, c, d, e, f, g, h) = wrapper[i]; keepResults(a, b, c, d, e, f, g, h); } };
template<int Count, typename V, typename T, typename I> static void deinterleave(const T &wrapper, const I &i) { DeinterleaveHelper<Count, V, T, I>::impl(wrapper, i); }

template<typename V> struct SomeData { static V x[8]; };
template<> float_v SomeData<float_v>::x[8] = { float_v::One() };
template<> sfloat_v SomeData<sfloat_v>::x[8] = { sfloat_v::One() };
template<> double_v SomeData<double_v>::x[8] = { double_v::One() };
template<> int_v SomeData<int_v>::x[8] = { int_v::One() };
template<> short_v SomeData<short_v>::x[8] = { short_v::One() };

template<int Count, typename V, typename T, typename I> struct InterleaveHelper;
template<typename V, typename T, typename I> struct InterleaveHelper<2, V, T, I> { static void impl(T &wrapper, const I &i) { wrapper[i] = (SomeData<V>::x[0], SomeData<V>::x[1]); } };
template<typename V, typename T, typename I> struct InterleaveHelper<3, V, T, I> { static void impl(T &wrapper, const I &i) { wrapper[i] = (SomeData<V>::x[0], SomeData<V>::x[1], SomeData<V>::x[2]); } };
template<typename V, typename T, typename I> struct InterleaveHelper<4, V, T, I> { static void impl(T &wrapper, const I &i) { wrapper[i] = (SomeData<V>::x[0], SomeData<V>::x[1], SomeData<V>::x[2], SomeData<V>::x[3]); } };
template<typename V, typename T, typename I> struct InterleaveHelper<5, V, T, I> { static void impl(T &wrapper, const I &i) { wrapper[i] = (SomeData<V>::x[0], SomeData<V>::x[1], SomeData<V>::x[2], SomeData<V>::x[3], SomeData<V>::x[4]); } };
template<typename V, typename T, typename I> struct InterleaveHelper<6, V, T, I> { static void impl(T &wrapper, const I &i) { wrapper[i] = (SomeData<V>::x[0], SomeData<V>::x[1], SomeData<V>::x[2], SomeData<V>::x[3], SomeData<V>::x[4], SomeData<V>::x[5]); } };
template<typename V, typename T, typename I> struct InterleaveHelper<7, V, T, I> { static void impl(T &wrapper, const I &i) { wrapper[i] = (SomeData<V>::x[0], SomeData<V>::x[1], SomeData<V>::x[2], SomeData<V>::x[3], SomeData<V>::x[4], SomeData<V>::x[5], SomeData<V>::x[6]); } };
template<typename V, typename T, typename I> struct InterleaveHelper<8, V, T, I> { static void impl(T &wrapper, const I &i) { wrapper[i] = (SomeData<V>::x[0], SomeData<V>::x[1], SomeData<V>::x[2], SomeData<V>::x[3], SomeData<V>::x[4], SomeData<V>::x[5], SomeData<V>::x[6], SomeData<V>::x[7]); } };
template<int Count, typename V, typename T, typename I> static void interleave(T &wrapper, const I &i) { InterleaveHelper<Count, V, T, I>::impl(wrapper, i); }

template<typename V> class Runner
{
    typedef typename V::EntryType T;
    typedef typename V::IndexType I;

    template<int COUNT> struct TestStruct_
    {
        enum { Count = COUNT };
        TestStruct_() {
            for (int i = 0; i < Count; ++i) {
                x[i] = T(i + 1);
            }
        }
        T x[Count];
    };

    enum { Repetitions = 1024 * 1024 };

    template<int COUNT> static void runImpl()
    {
        std::ostringstream str;
        str << COUNT;
        Benchmark::setColumnData("Member Count", str.str());
        typedef TestStruct_<COUNT> TestStruct;
        typedef std::vector<TestStruct> TestData;

        TestData data(1024);
#ifndef VC_BENCHMARK_NO_MLOCK
        mlock(&data[0], 1024 * sizeof(TestStruct));
#endif

        Vc::InterleavedMemoryWrapper<TestStruct, V> wrapper(&data[0]);
#if VC_VERSION_NUMBER >= VC_VERSION_CHECK(0,7,70)
        benchmark_loop(Benchmark("deinterleave (successive)", Repetitions * sizeof(TestStruct), "Byte")) {
            for (int i = 0; i < Repetitions; i += V::Size) {
                for (int j = 0; j < V::Size; ++j) {
                    asm("":"+m"(data[j]));
                }
                deinterleave<COUNT, V>(wrapper, 0);
            }
        }
        benchmark_loop(Benchmark("interleave (successive)", Repetitions * sizeof(TestStruct), "Byte")) {
            for (int i = 0; i < Repetitions; i += V::Size) {
                for (int j = 0; j < V::Size; ++j) {
                    asm("":"+m"(data[j]));
                }
                interleave<COUNT, V>(wrapper, 0);
            }
        }
#endif
        benchmark_loop(Benchmark("deinterleave (index vector)", Repetitions * sizeof(TestStruct), "Byte")) {
            for (int i = 0; i < Repetitions; i += V::Size) {
                for (int j = 0; j < V::Size; ++j) {
                    asm("":"+m"(data[j]));
                }
                deinterleave<COUNT, V>(wrapper, I::IndexesFromZero());
            }
        }
        benchmark_loop(Benchmark("interleave (index vector)", Repetitions * sizeof(TestStruct), "Byte")) {
            for (int i = 0; i < Repetitions; i += V::Size) {
                for (int j = 0; j < V::Size; ++j) {
                    asm("":"+m"(data[j]));
                }
                interleave<COUNT, V>(wrapper, I::IndexesFromZero());
            }
        }
#if VC_VERSION_NUMBER >= VC_VERSION_CHECK(0,7,70)
        benchmark_loop(Benchmark("normalize interleaved vectors (successive)", Repetitions * sizeof(TestStruct), "Byte")) {
            for (int i = 0; i < Repetitions; i += V::Size) {
                for (int j = 0; j < V::Size; ++j) {
                    asm("":"+m"(data[j]));
                }
                normalize<COUNT, V>(wrapper, 0);
            }
        }
#endif
        benchmark_loop(Benchmark("normalize interleaved vectors (index vector)", Repetitions * sizeof(TestStruct), "Byte")) {
            for (int i = 0; i < Repetitions; i += V::Size) {
                for (int j = 0; j < V::Size; ++j) {
                    asm("":"+m"(data[j]));
                }
                normalize<COUNT, V>(wrapper, I::IndexesFromZero());
            }
        }
        benchmark_loop(Benchmark("normalize interleaved vectors (manually)", Repetitions * sizeof(TestStruct), "Byte")) {
            for (int i = 0; i < Repetitions; i += V::Size) {
                for (int j = 0; j < V::Size; ++j) {
                    asm("":"+m"(data[j]));
                }
                V x[COUNT];
                for (int j = 0; j < V::Size; ++j) {
                    for (int k = 0; k < COUNT; ++k) {
                        x[k][j] = data[0 + j].x[k];
                    }
                }
                V sum = x[0] * x[0];
                for (int k = 1; k < COUNT; ++k) {
                    sum += x[k] * x[k];
                }
                const V factor = V::One() / std::sqrt(sum);
                for (int k = 0; k < COUNT; ++k) {
                    x[k] *= factor;
                }
                for (int j = 0; j < V::Size; ++j) {
                    for (int k = 0; k < COUNT; ++k) {
                        data[0 + j].x[k] = x[k][j];
                    }
                }
            }
        }
        benchmark_loop(Benchmark("normalize vectors (baseline)", Repetitions * sizeof(TestStruct), "Byte")) {
            for (int i = 0; i < Repetitions; i += V::Size) {
                for (int k = 0; k < COUNT; ++k) {
                    asm("":"+m"(SomeData<V>::x[k])); // important: otherwise the compiler will use common
                    // subexpression elimination to pull the first three loops out of the
                    // surrounding loop
                }
                V x[COUNT];
                for (int k = 0; k < COUNT; ++k) {
                    x[k] = SomeData<V>::x[k];
                }
                V sum = x[0] * x[0];
                for (int k = 1; k < COUNT; ++k) {
                    sum += x[k] * x[k];
                }
                const V factor = V::One() / std::sqrt(sum);
                for (int k = 0; k < COUNT; ++k) {
                    x[k] *= factor;
                }
                for (int k = 0; k < COUNT; ++k) {
                    keepResults(x[k]);
                }
            }
        }
    }

public:
    static void run()
    {
        runImpl<2>();
        runImpl<3>();
        runImpl<4>();
        runImpl<5>();
        runImpl<6>();
        runImpl<7>();
        runImpl<8>();
    }
};

int bmain()
{
    Benchmark::addColumn("datatype");
    Benchmark::addColumn("Member Count");

    Benchmark::setColumnData("datatype", "float_v");
    Runner<float_v>::run();
    Benchmark::setColumnData("datatype", "double_v");
    Runner<double_v>::run();
    Benchmark::setColumnData("datatype", "sfloat_v");
    Runner<sfloat_v>::run();
    Benchmark::setColumnData("datatype", "int_v");
    Runner<int_v>::run();
    Benchmark::setColumnData("datatype", "short_v");
    Runner<short_v>::run();
    return 0;
}
