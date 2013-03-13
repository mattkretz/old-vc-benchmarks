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

#include <cstdlib>

using namespace Vc;

// to benchmark the performance of gathers there are some different interesting cases:
// 1) gathers from the same memory location (basically broadcasts)
// 2) gathers from the same cache line (random access but localized to 64 or 128 bytes) (64 on
//    most CPUs)
// 3) random access on memory that fits into L1 but is larger than a cache line
// 4) random access on memory that fits into L2 (per core) but is larger than L1
// 4b) random access on memory that fits into all of L2 but is larger than the per core L2
// 5) random access on memory that fits into L3 but is larger than L2
// 6) random access on memory that fits into memory but is larger than L3
// 7) random access on memory that fits into virtual memory but is larger than physical memory

// Intel Core 2 Quad (Q6600) has 8MB L2

template<typename Vector> class FullMaskHelper
{
    protected:
        typedef typename Vector::Mask IndexMask;
        static const IndexMask fullMask;
};

template<> const float_m FullMaskHelper<float_v>::fullMask = float_m(One);
template<> const short_m FullMaskHelper<short_v>::fullMask = short_m(One);
template<> const sfloat_m FullMaskHelper<sfloat_v>::fullMask = sfloat_m(One);
template<> const double_m FullMaskHelper<double_v>::fullMask = double_m(One);

enum {
    BaseFactor = 1600000
};

template<typename Vector, class GatherImpl> class GatherBase : public FullMaskHelper<Vector>
{
    public:
        typedef typename Vector::IndexType IndexVector;
        typedef typename Vector::Mask IndexMask;
        typedef typename Vector::EntryType Scalar;

        enum {
            Factor = BaseFactor / Vector::Size
        };

        GatherBase(const char *name, const unsigned int size, const Scalar *_data, double multiplier = 4.)
            : timer(name, multiplier * Vector::Size * Factor, "Values"),
            indexesCount(Factor * 4),
            m_data(new const Scalar *[indexesCount])
        {
            IndexAndMask *const tmp = new IndexAndMask[indexesCount];
#ifndef VC_BENCHMARK_NO_MLOCK
            mlock(m_data, indexesCount * sizeof(Scalar *));
            mlock(tmp, indexesCount * sizeof(IndexAndMask));
#endif
            IndexVector::Random();
            IndexVector::Random();
            const IndexVector indexMask((size - 1) & 0xffff);
            const unsigned int maxIndex = 0xffff;
            //const unsigned int maxIndex = ~0u >> ((4 - sizeof(typename IndexVector::EntryType)) * 8);
            const unsigned int maxDataOffset = maxIndex > size ? 1 : size - maxIndex;
            for (int i = 0; i < indexesCount; ++i) {
                m_data[i] = _data + (rand() % maxDataOffset);
                tmp[i].index = IndexVector::Random() & indexMask;
                tmp[i].mask = (IndexVector::Random() & IndexVector(One)) > 0;
            }
            m_data[0] = _data;
            im = tmp;

            while (timer.wantsMoreDataPoints()) {
                timer.Start();
                static_cast<const GatherImpl *>(this)->run();
                timer.Stop();
            }
            timer.Print();
        }

        ~GatherBase()
        {
            delete[] m_data;
            delete[] im;
        }

    protected:
        inline const Scalar *data(int i = 0) const {
            return m_data[i];
        }
        Benchmark timer;
        const int indexesCount;
        const Scalar **const m_data;
        const struct IndexAndMask : public VectorAlignedBase {
            IndexVector index;
            IndexMask mask;
        } *im;
};

static int g_L1ArraySize = 0;
static int g_L2ArraySize = 0;
static int g_L3ArraySize = 0;
static int g_CacheLineArraySize = 0;
static int g_MaxArraySize = 0;

template<typename Vector> struct GatherBenchmark
{
    typedef typename Vector::EntryType Scalar;

    enum {
        Factor = BaseFactor / Vector::Size
    };

    class FullMaskedGather : public GatherBase<Vector, FullMaskedGather>
    {
        typedef GatherBase<Vector, FullMaskedGather> Base;
        using Base::data;
        using Base::fullMask;
        using Base::im;

        public:
            FullMaskedGather(const char *name, const unsigned int _size, const Scalar *_data)
                : Base(name, _size, _data, 4.)
            {}

            inline void run() const
            {
                for (int i = 0; i < Factor; ++i) {
                    const int ii = i * 4;
                    keepResults(im[ii + 0].mask); Vector tmp0(data(ii + 0), im[ii + 0].index, fullMask); keepResultsDirty(tmp0);
                    keepResults(im[ii + 1].mask); Vector tmp1(data(ii + 1), im[ii + 1].index, fullMask); keepResultsDirty(tmp1);
                    keepResults(im[ii + 2].mask); Vector tmp2(data(ii + 2), im[ii + 2].index, fullMask); keepResultsDirty(tmp2);
                    keepResults(im[ii + 3].mask); Vector tmp3(data(ii + 3), im[ii + 3].index, fullMask); keepResultsDirty(tmp3);
                }
            }
    };

    class MaskedGather : public GatherBase<Vector, MaskedGather>
    {
        typedef GatherBase<Vector, MaskedGather> Base;
        using Base::data;
        using Base::fullMask;
        using Base::im;

        public:
            MaskedGather(const char *name, const unsigned int _size, const Scalar *_data)
                : GatherBase<Vector, MaskedGather>(name, _size, _data, 2.)
            {}

            inline void run() const
            {
                for (int i = 0; i < Factor; ++i) {
                    const int ii = i * 4;
                    keepResults(fullMask); Vector tmp0(data(ii + 0), im[ii + 0].index, im[ii + 0].mask); keepResultsDirty(tmp0);
                    keepResults(fullMask); Vector tmp1(data(ii + 1), im[ii + 1].index, im[ii + 1].mask); keepResultsDirty(tmp1);
                    keepResults(fullMask); Vector tmp2(data(ii + 2), im[ii + 2].index, im[ii + 2].mask); keepResultsDirty(tmp2);
                    keepResults(fullMask); Vector tmp3(data(ii + 3), im[ii + 3].index, im[ii + 3].mask); keepResultsDirty(tmp3);
                }
            }
    };

    class Gather : public GatherBase<Vector, Gather>
    {
        typedef GatherBase<Vector, Gather> Base;
        using Base::data;
        using Base::fullMask;
        using Base::im;

        public:
            Gather(const char *name, const unsigned int _size, const Scalar *_data)
                : GatherBase<Vector, Gather>(name, _size, _data)
            {}

            inline void run() const
            {
                for (int i = 0; i < Factor; ++i) {
                    const int ii = i * 4;
                    keepResults(fullMask); keepResults(im[ii + 0].mask); Vector tmp0(data(ii + 0), im[ii + 0].index); keepResultsDirty(tmp0);
                    keepResults(fullMask); keepResults(im[ii + 1].mask); Vector tmp1(data(ii + 1), im[ii + 1].index); keepResultsDirty(tmp1);
                    keepResults(fullMask); keepResults(im[ii + 2].mask); Vector tmp2(data(ii + 2), im[ii + 2].index); keepResultsDirty(tmp2);
                    keepResults(fullMask); keepResults(im[ii + 3].mask); Vector tmp3(data(ii + 3), im[ii + 3].index); keepResultsDirty(tmp3);
                }
            }
    };

    static void randomize(Scalar *const p, unsigned int size)
    {
        static const double ONE_OVER_RAND_MAX = 1. / RAND_MAX;
        for (unsigned int i = 0; i < size; ++i) {
            p[i] = static_cast<Scalar>(static_cast<double>(std::rand()) * ONE_OVER_RAND_MAX);
        }
    }

    static void run()
    {
        const int MaxArraySize = g_MaxArraySize / sizeof(Scalar);
        const int L3ArraySize = g_L3ArraySize / sizeof(Scalar);
        const int L2ArraySize = g_L2ArraySize / sizeof(Scalar);
        const int L1ArraySize = g_L1ArraySize / sizeof(Scalar);
        const int CacheLineArraySize = g_CacheLineArraySize / sizeof(Scalar);
        const int ArrayCount = 2 * MaxArraySize;

        Scalar *const _data = new Scalar[ArrayCount];
#ifndef VC_BENCHMARK_NO_MLOCK
        mlock(_data, ArrayCount * sizeof(Scalar));
#endif
        randomize(_data, ArrayCount);

        // the last parts of _data are still hot, so we start at the beginning
        Scalar *data = _data;
        Benchmark::setColumnData("mask", "random mask");
        MaskedGather("Memory", MaxArraySize, data);

        // now the last parts of _data should be cold, let's go there
        data += ArrayCount - MaxArraySize;
        Benchmark::setColumnData("mask", "not masked");
        Gather("Memory", MaxArraySize, data);

        data = _data;
        Benchmark::setColumnData("mask", "masked with one");
        FullMaskedGather("Memory", MaxArraySize, data);

        data += ArrayCount - MaxArraySize;

        Benchmark::setColumnData("mask", "not masked");
        if (L3ArraySize > 0) Gather("L3", L3ArraySize, data);
        Gather("L2", L2ArraySize, data);
        Gather("L1", L1ArraySize, data);
        Gather("Cacheline", CacheLineArraySize, data);
        Gather("Broadcast", 1, data);

        Benchmark::setColumnData("mask", "random mask");
        if (L3ArraySize > 0) MaskedGather("L3", L3ArraySize, data);
        MaskedGather("L2", L2ArraySize, data);
        MaskedGather("L1", L1ArraySize, data);
        MaskedGather("Cacheline", CacheLineArraySize, data);
        MaskedGather("Broadcast", 1, data);

        Benchmark::setColumnData("mask", "masked with one");
        if (L3ArraySize > 0) FullMaskedGather("L3", L3ArraySize, data);
        FullMaskedGather("L2", L2ArraySize, data);
        FullMaskedGather("L1", L1ArraySize, data);
        FullMaskedGather("Cacheline", CacheLineArraySize, data);
        FullMaskedGather("Broadcast", 1, data);

        delete[] _data;
    }
};

#include <Vc/cpuid.h>

int bmain()
{
    Benchmark::addColumn("datatype");
    Benchmark::addColumn("mask");
    Benchmark::addColumn("L1.size");
    Benchmark::addColumn("L2.size");
    Benchmark::addColumn("L3.size");
    Benchmark::addColumn("Cacheline.size");

    g_L1ArraySize = CpuId::L1Data();
    g_L2ArraySize = CpuId::L2Data();
    g_L3ArraySize = CpuId::L3Data();
    g_CacheLineArraySize = CpuId::L1DataLineSize();
    g_MaxArraySize = std::max(g_L2ArraySize, g_L3ArraySize) * 8;
    {
        std::ostringstream str;
        str << g_L1ArraySize;
        Benchmark::setColumnData("L1.size", str.str());
    }
    {
        std::ostringstream str;
        str << g_L2ArraySize;
        Benchmark::setColumnData("L2.size", str.str());
    }
    {
        std::ostringstream str;
        str << g_L3ArraySize;
        Benchmark::setColumnData("L3.size", str.str());
    }
    {
        std::ostringstream str;
        str << g_CacheLineArraySize;
        Benchmark::setColumnData("Cacheline.size", str.str());
    }

    // divide by 2 since other parts of the program also affect the caches
    g_L1ArraySize /= 2;
    g_L2ArraySize /= 2;
    g_L3ArraySize /= 2;

    Benchmark::setColumnData("datatype", "float_v");
    GatherBenchmark<float_v>::run();
    Benchmark::setColumnData("datatype", "short_v");
    GatherBenchmark<short_v>::run();
    Benchmark::setColumnData("datatype", "sfloat_v");
    GatherBenchmark<sfloat_v>::run();
    Benchmark::setColumnData("datatype", "double_v");
    GatherBenchmark<double_v>::run();

    return 0;
}
