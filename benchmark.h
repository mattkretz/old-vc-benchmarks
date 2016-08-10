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

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <Vc/Vc>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <list>
#include <vector>
#include <algorithm>
#include <time.h>
#include <cstring>
#include <string>
#include <fstream>
#ifndef VC_BENCHMARK_NO_MLOCK
#include <sys/mman.h>
#endif
#ifdef _MSC_VER
#include <windows.h>
#include <float.h>
#else
#include <cmath>
#endif
#ifdef __APPLE__
#include <mach/mach_time.h>
// method to get monotonic mac time, inspired by
// http://www.wand.net.nz/~smr26/wordpress/2009/01/19/monotonic-time-in-mac-os-x/
#endif

#include "tsc.h"

#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#endif

class Benchmark
{
    friend int main(int, char**);
    class FileWriter
    {
        public:
            FileWriter(const std::string &filename);
            ~FileWriter();

            void declareData(const std::string &name, const std::list<std::string> &header);
            void addDataLine(const std::list<std::string> &data);

            void addColumn(const std::string &name);
            void setColumnData(const std::string &name, const std::string &data);
            void finalize() { m_finalized = true; }

        private:
            std::ofstream m_file;
            bool m_finalized;
            std::string m_currentName;
            std::list<std::string> m_header;
            struct ExtraColumn
            {
                ExtraColumn(const std::string &n) : name(n) {}
                std::string name;
                std::string data;
                inline bool operator==(const ExtraColumn &rhs) const { return name == rhs.name; }
                inline bool operator==(const std::string &rhs) const { return name == rhs; }
            };
            std::list<ExtraColumn> m_extraColumns;
    };
public:
    static void addColumn(const std::string &name);
    static void setColumnData(const std::string &name, const std::string &data);
    static void finalize();

    explicit Benchmark(const std::string &name, double factor = 0., const std::string &X = std::string());
    void changeInterpretation(double factor, const char *X);

    bool wantsMoreDataPoints() const;
    Vc_ALWAYS_INLINE_L bool Start() Vc_ALWAYS_INLINE_R;
    void Mark();
    Vc_ALWAYS_INLINE_L void Stop() Vc_ALWAYS_INLINE_R;
    bool Print();

private:
    void printMiddleLine() const;
    void printBottomLine() const;

    const std::string fName;
    double fFactor;
    std::string fX;
#ifdef _MSC_VER
    __int64 fRealTime;
#elif defined(__APPLE__)
    uint64_t fRealTime;
#else
    struct timespec fRealTime;
#ifdef VC_USE_CPU_TIME
    struct timespec fCpuTime;
#endif
#endif
    double m_mean[3];
    double m_stddev[3];
    TimeStampCounter fTsc;
    int m_dataPointsCount;
    static FileWriter *s_fileWriter;
    bool m_skip;

    static const char greenEsc  [8];
    static const char cyanEsc   [8];
    static const char reverseEsc[5];
    static const char normalEsc [5];
};

Vc_ALWAYS_INLINE bool Benchmark::Start()
{
#ifdef _MSC_VER
    QueryPerformanceCounter((LARGE_INTEGER *)&fRealTime);
#elif defined(__APPLE__)
    fRealTime = mach_absolute_time();
#else
    clock_gettime( CLOCK_MONOTONIC, &fRealTime );
#ifdef VC_USE_CPU_TIME
    clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &fCpuTime );
#endif
#endif
    fTsc.Start();
    return true;
}

#ifndef _MSC_VER
static inline double convertTimeSpec(const struct timespec &ts)
{
    return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) * 1e-9;
}
#endif

Vc_ALWAYS_INLINE void Benchmark::Stop()
{
    fTsc.Stop();
#ifdef _MSC_VER
    __int64 real = 0, freq = 0;
    QueryPerformanceCounter((LARGE_INTEGER *)&real);
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
    const double elapsedRealTime = static_cast<double>(real - fRealTime) / freq;
#elif defined(__APPLE__)
    uint64_t real = mach_absolute_time();
    static mach_timebase_info_data_t info = {0,0};

    if (info.denom == 0) {
        mach_timebase_info(&info);
    }

    uint64_t nanos = (real - fRealTime ) * (info.numer / info.denom);
    const double elapsedRealTime = nanos * 1e-9;
#else
    struct timespec real;
    clock_gettime( CLOCK_MONOTONIC, &real );
#ifdef VC_USE_CPU_TIME
    struct timespec cpu;
    clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &cpu );
    const double elapsedCpuTime = convertTimeSpec(cpu ) - convertTimeSpec(fCpuTime);
    m_mean[2] += elapsedCpuTime;
    m_stddev[2] += elapsedCpuTime * elapsedCpuTime;
#endif
    const double elapsedRealTime = convertTimeSpec(real) - convertTimeSpec(fRealTime);
#endif
    m_mean[0] += elapsedRealTime;
    m_mean[1] += fTsc.Cycles();
    m_stddev[0] += elapsedRealTime * elapsedRealTime;
    m_stddev[1] += fTsc.Cycles() * fTsc.Cycles();
    ++m_dataPointsCount;
}

int bmain();
extern const char *printHelp2;

#define SET_HELP_TEXT(str) \
    int _set_help_text_init() { \
        printHelp2 = str; \
        return 0; \
    } \
    int _set_help_text_init_ = _set_help_text_init()

#ifdef __GNUC__
#  define VC_IS_UNLIKELY(x) __builtin_expect(x, 0)
#  define VC_IS_LIKELY(x) __builtin_expect(x, 1)
#else
#  define VC_IS_UNLIKELY(x) x
#  define VC_IS_LIKELY(x) x
#endif

#define benchmark_loop(_bm_obj) \
    for (Benchmark _bm_obj_local = _bm_obj; \
            VC_IS_LIKELY(_bm_obj_local.wantsMoreDataPoints() && _bm_obj_local.Start()) || VC_IS_UNLIKELY(_bm_obj_local.Print()); \
            _bm_obj_local.Stop())

#define benchmark_restart _bm_obj_local.Start

template<int S> struct KeepResultsType;
template<> struct KeepResultsType<1> { typedef   char Type; };
template<> struct KeepResultsType<2> { typedef  short Type; };
template<> struct KeepResultsType<4> { typedef    int Type; };
template<> struct KeepResultsType<8> { typedef double Type; };

template <typename T,
          int S,
          bool = Vc::Traits::isSimdArray<T>::value || Vc::Traits::isSimdMaskArray<T>::value,
          bool = Vc::Traits::isAtomicSimdArray<T>::value || Vc::Traits::isAtomicSimdMaskArray<T>::value,
          bool = Vc::is_simd_vector<T>::value || Vc::is_simd_mask<T>::value,
          bool = Vc::Scalar::is_vector<T>::value || Vc::Scalar::is_mask<T>::value>
struct KeepResultsHelper {
    static_assert(std::is_arithmetic<T>::value || sizeof(T) < 16, "The unspecialized KeepResultsHelper only works for builtin types.");
    template <typename U = void>
    static decltype(std::declval<T &>().data()) &cast(T &x) {
        return reinterpret_cast<decltype(std::declval<T &>().data()) &>(x);
    }
    template <typename U = void>
    static T &cast(typename std::enable_if<std::is_same<U, void>::value &&
                   std::is_arithmetic<T>::value, T &>::type x) { return x; }
    static Vc_INTRINSIC void keepDirty(T &tmp0) {
#ifdef __GNUC__
        asm volatile("":"+r"(cast(tmp0)));
#else
        blackHole[0] = tmp0;
#endif
    }
    static Vc_INTRINSIC void keepDirty(T &tmp0, T &tmp1, T &tmp2, T &tmp3) {
#ifdef __GNUC__
        asm volatile("":"+r"(cast(tmp0)),
                        "+r"(cast(tmp1)),
                        "+r"(cast(tmp2)),
                        "+r"(cast(tmp3)));
#else
        blackHole[0] = tmp0;
        blackHole[1] = tmp1;
        blackHole[2] = tmp2;
        blackHole[3] = tmp3;
#endif
    }
    static Vc_INTRINSIC void keep(const T &tmp0, const T &tmp1, const T &tmp2, const T &tmp3,
            const T &tmp4, const T &tmp5, const T &tmp6, const T &tmp7) {
#ifdef __GNUC__
#ifdef __x86_64__
        asm volatile(""::"r"(tmp0), "r"(tmp1), "r"(tmp2), "r"(tmp3),
                "r"(tmp4), "r"(tmp5), "r"(tmp6), "r"(tmp7));
#else
        asm volatile(""::"r"(tmp0), "r"(tmp1));
        asm volatile(""::"r"(tmp2), "r"(tmp3));
        asm volatile(""::"r"(tmp4), "r"(tmp5));
        asm volatile(""::"r"(tmp6), "r"(tmp7));
#endif
#else // __GNUC__
        blackHole[0] = tmp0;
        blackHole[1] = tmp1;
        blackHole[2] = tmp2;
        blackHole[3] = tmp3;
        blackHole[4] = tmp4;
        blackHole[5] = tmp5;
        blackHole[6] = tmp6;
        blackHole[7] = tmp7;
#endif // __GNUC__
    }
#ifndef __GNUC__
    static T blackHole[8];
#endif
};
template <typename T, int S>
struct KeepResultsHelper<T, S, true, true, true, false> {
    static Vc_INTRINSIC void keepDirty(T &tmp0) {
        auto &a = internal_data(tmp0);
        KeepResultsHelper<std::decay_t<decltype(a)>, sizeof(a)>::keepDirty(a);
    }
    static Vc_INTRINSIC void keep(const T &tmp0, const T &tmp1,
                                  const T &tmp2, const T &tmp3,
                                  const T &tmp4, const T &tmp5,
                                  const T &tmp6, const T &tmp7) {
        auto &a0 = internal_data(tmp0);
        auto &a1 = internal_data(tmp1);
        auto &a2 = internal_data(tmp2);
        auto &a3 = internal_data(tmp3);
        auto &a4 = internal_data(tmp4);
        auto &a5 = internal_data(tmp5);
        auto &a6 = internal_data(tmp6);
        auto &a7 = internal_data(tmp7);
        KeepResultsHelper<std::decay_t<decltype(a0)>, sizeof(a0)>::keep(a0, a1, a2, a3, a4, a5, a6, a7);
    }
};
template <typename T, int S>
struct KeepResultsHelper<T, S, true, false, true, false> {
    static Vc_INTRINSIC void keepDirty(T &tmp0) {
        auto &a = internal_data0(tmp0);
        auto &b = internal_data1(tmp0);
        KeepResultsHelper<std::decay_t<decltype(a)>, sizeof(a)>::keepDirty(a);
        KeepResultsHelper<std::decay_t<decltype(b)>, sizeof(b)>::keepDirty(b);
    }
    static Vc_INTRINSIC void keep(const T &tmp0, const T &tmp1,
                                  const T &tmp2, const T &tmp3,
                                  const T &tmp4, const T &tmp5,
                                  const T &tmp6, const T &tmp7) {
        auto &a0 = internal_data0(tmp0);
        auto &a1 = internal_data0(tmp1);
        auto &a2 = internal_data0(tmp2);
        auto &a3 = internal_data0(tmp3);
        auto &a4 = internal_data0(tmp4);
        auto &a5 = internal_data0(tmp5);
        auto &a6 = internal_data0(tmp6);
        auto &a7 = internal_data0(tmp7);
        KeepResultsHelper<std::decay_t<decltype(a0)>, sizeof(a0)>::keep(a0, a1, a2, a3, a4, a5, a6, a7);

        auto &b0 = internal_data1(tmp0);
        auto &b1 = internal_data1(tmp1);
        auto &b2 = internal_data1(tmp2);
        auto &b3 = internal_data1(tmp3);
        auto &b4 = internal_data1(tmp4);
        auto &b5 = internal_data1(tmp5);
        auto &b6 = internal_data1(tmp6);
        auto &b7 = internal_data1(tmp7);
        KeepResultsHelper<std::decay_t<decltype(b0)>, sizeof(b0)>::keep(b0, b1, b2, b3, b4, b5, b6, b7);
    }
};
template <typename T> struct Intrinsic;
#ifdef __SSE2__
template <typename T> struct Intrinsic<Vc::Vector<T, Vc::VectorAbi::Sse>> {
    using type = __m128;
};
template <typename T> struct Intrinsic<Vc::Mask<T, Vc::VectorAbi::Sse>> {
    using type = __m128;
};
#ifdef __AVX__
template <typename T> struct Intrinsic<Vc::Vector<T, Vc::VectorAbi::Avx>> {
    using type = __m256;
};
template <typename T> struct Intrinsic<Vc::Mask<T, Vc::VectorAbi::Avx>> {
    using type = __m256;
};
#endif
template <typename T, std::size_t N, typename V>
struct Intrinsic<Vc::SimdArray<T, N, V, N>> {
    using type = typename Intrinsic<V>::type;
};
template <typename T, std::size_t N, typename V>
struct Intrinsic<Vc::SimdMaskArray<T, N, V, N>> {
    using type = typename Intrinsic<V>::type;
};
template <typename T, int S>
struct KeepResultsHelper<T, S, false, false, true, false> {
    using Intrin = typename Intrinsic<T>::type;
    static Vc_INTRINSIC Intrin &cast(T &x) {
        return reinterpret_cast<Intrin &>(x);
    }
    static Vc_INTRINSIC void keepDirty(T &tmp0) {
        asm volatile("":"+x"(cast(tmp0)));
    }
    static Vc_INTRINSIC void keepDirty(T &tmp0, T &tmp1, T &tmp2, T &tmp3) {
        asm volatile("":"+x"(cast(tmp0)), "+x"(cast(tmp1)),
                        "+x"(cast(tmp2)), "+x"(cast(tmp3)));
    }
    static Vc_INTRINSIC void keep(const T &tmp0, const T &tmp1, const T &tmp2, const T &tmp3,
            const T &tmp4, const T &tmp5, const T &tmp6, const T &tmp7) {
#ifdef __x86_64__
        asm volatile(""::"x"(tmp0), "x"(tmp1), "x"(tmp2), "x"(tmp3),
                         "x"(tmp4), "x"(tmp5), "x"(tmp6), "x"(tmp7));
#else
        asm volatile(""::"x"(tmp0), "x"(tmp1), "x"(tmp2), "x"(tmp3));
        asm volatile(""::"x"(tmp4), "x"(tmp5), "x"(tmp6), "x"(tmp7));
#endif
    }
};
#endif

template<typename T> static Vc_INTRINSIC void keepResultsDirty(T &tmp0)
{
    KeepResultsHelper<T, sizeof(T)>::keepDirty(tmp0);
}

template<typename T> static Vc_INTRINSIC void keepResultsDirty(T &tmp0, T &tmp1, T &tmp2, T &tmp3)
{
    KeepResultsHelper<T, sizeof(T)>::keepDirty(tmp0, tmp1, tmp2, tmp3);
}

template<typename T> static Vc_INTRINSIC void keepResults(T tmp0)
{
    KeepResultsHelper<T, sizeof(T)>::keep(tmp0, tmp0, tmp0, tmp0, tmp0, tmp0, tmp0, tmp0);
}

template<typename T> static Vc_INTRINSIC void keepResults(T tmp0, T tmp1)
{
    KeepResultsHelper<T, sizeof(T)>::keep(tmp0, tmp1, tmp0, tmp1, tmp0, tmp1, tmp0, tmp1);
}

template<typename T> static Vc_INTRINSIC void keepResults(T tmp0, T tmp1, T tmp2)
{
    KeepResultsHelper<T, sizeof(T)>::keep(tmp0, tmp1, tmp2, tmp1, tmp0, tmp2, tmp0, tmp1);
}

template<typename T> static Vc_INTRINSIC void keepResults(T tmp0, T tmp1, T tmp2, T tmp3)
{
    KeepResultsHelper<T, sizeof(T)>::keep(tmp0, tmp1, tmp2, tmp3, tmp0, tmp1, tmp2, tmp3);
}

template<typename T> static Vc_INTRINSIC void keepResults(T tmp0, T tmp1, T tmp2, T tmp3, T tmp4)
{
    KeepResultsHelper<T, sizeof(T)>::keep(tmp0, tmp1, tmp2, tmp3, tmp4, tmp1, tmp0, tmp1);
}

template<typename T> static Vc_INTRINSIC void keepResults(T tmp0, T tmp1, T tmp2, T tmp3, T tmp4, T tmp5)
{
    KeepResultsHelper<T, sizeof(T)>::keep(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp0, tmp1);
}

template<typename T> static Vc_INTRINSIC void keepResults(T tmp0, T tmp1, T tmp2, T tmp3, T tmp4, T tmp5, T tmp6)
{
    KeepResultsHelper<T, sizeof(T)>::keep(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp1);
}

template<typename T> static Vc_INTRINSIC void keepResults(T tmp0, T tmp1, T tmp2, T tmp3, T tmp4, T tmp5, T tmp6, T tmp7)
{
    KeepResultsHelper<T, sizeof(T)>::keep(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7);
}

namespace Vc_1 {
template<typename... Ts> Vc_INTRINSIC void forceToRegisters(Ts &...xs) {
    keepResults(xs...);
}
}  // namespace Vc

#endif // BENCHMARK_H
