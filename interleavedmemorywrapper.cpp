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
using Vc::sfloat_v;
using Vc::double_v;
using Vc::int_v;
using Vc::short_v;

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
template<> float_v SomeData<float_v>::x[8] = {};
template<> sfloat_v SomeData<sfloat_v>::x[8] = {};
template<> double_v SomeData<double_v>::x[8] = {};
template<> int_v SomeData<int_v>::x[8] = {};
template<> short_v SomeData<short_v>::x[8] = {};

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

    template<int COUNT> static void runImpl(size_t MemorySize)
    {
        std::ostringstream str;
        str << COUNT;
        Benchmark::setColumnData("Member Count", str.str());
        typedef TestStruct_<COUNT> TestStruct;
        typedef std::vector<TestStruct> TestData;

        MemorySize /= sizeof(TestStruct);
        MemorySize &= ~(V::Size - 1);
        TestData data(MemorySize);
#ifndef VC_BENCHMARK_NO_MLOCK
        mlock(&data[0], MemorySize * sizeof(TestStruct));
#endif

        benchmark_loop(Benchmark("deinterleave (successive)", MemorySize * sizeof(TestStruct), "Byte")) {
            Vc::InterleavedMemoryWrapper<TestStruct, V> wrapper(&data[0]);
            for (size_t i = 0; i < MemorySize; i += V::Size) {
                deinterleave<COUNT, V>(wrapper, i);
            }
        }
        benchmark_loop(Benchmark("interleave (successive)", MemorySize * sizeof(TestStruct), "Byte")) {
            Vc::InterleavedMemoryWrapper<TestStruct, V> wrapper(&data[0]);
            for (size_t i = 0; i < MemorySize; i += V::Size) {
                interleave<COUNT, V>(wrapper, i);
            }
        }
        benchmark_loop(Benchmark("deinterleave (index vector)", MemorySize * sizeof(TestStruct), "Byte")) {
            Vc::InterleavedMemoryWrapper<TestStruct, V> wrapper(&data[0]);
            for (I i = I::IndexesFromZero(); i < MemorySize; i += V::Size) {
                deinterleave<COUNT, V>(wrapper, i);
            }
        }
        benchmark_loop(Benchmark("interleave (successive)", MemorySize * sizeof(TestStruct), "Byte")) {
            Vc::InterleavedMemoryWrapper<TestStruct, V> wrapper(&data[0]);
            for (I i = I::IndexesFromZero(); i < MemorySize; i += V::Size) {
                interleave<COUNT, V>(wrapper, i);
            }
        }
    }

    static void run(size_t MemorySize)
    {
        runImpl<2>(MemorySize);
        runImpl<3>(MemorySize);
        runImpl<4>(MemorySize);
        runImpl<5>(MemorySize);
        runImpl<6>(MemorySize);
        runImpl<7>(MemorySize);
        runImpl<8>(MemorySize);
    }

public:
    static void run()
    {
        using Vc::CpuId;
        Benchmark::setColumnData("MemorySize", "half L1");
        run(CpuId::L1Data() / 2);
        Benchmark::setColumnData("MemorySize", "L1");
        run(CpuId::L1Data());
        Benchmark::setColumnData("MemorySize", "half L2");
        run(CpuId::L2Data() / 2);
        Benchmark::setColumnData("MemorySize", "L2");
        run(CpuId::L2Data());
    }
};

int bmain()
{
    Benchmark::addColumn("MemorySize");
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
