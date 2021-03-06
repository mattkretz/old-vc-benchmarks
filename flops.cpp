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
#include <cstdio>
#include <cstdlib>

// FIXME: is this standard?
#ifdef __x86_64__
#define VC_64BIT
#else
#define VC_32BIT
#endif

using namespace Vc;

enum {
    Factor = 2000000 / float_v::Size
};

static float randomF(float min, float max)
{
    const float delta = max - min;
    return min + delta * rand() / RAND_MAX;
}

static float randomF12() { return randomF(1.f, 2.f); }

int bmain()
{
    int blackHole = true;
    // asm reference
#ifdef __GNUC__
    {
        Benchmark timer("asm reference", 2 * 8 * float_v::Size * Factor, "FLOP");
        while (timer.wantsMoreDataPoints()) {
#if VC_IMPL_FMA4
            __m256 x[6] = { _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()) };
            const __m256 y = _mm256_set1_ps(randomF12());
            int i = Factor * 8 / 6;
            asm volatile("":: "x"(x[0]), "x"(x[1]), "x"(x[2]), "x"(x[3]), "x"(x[4]), "x"(x[5]));
            timer.Start();
            ///////////////////////////////////////
            asm(
                    //".align 32\n\t"
                    "0: "
                    "vfmaddps %7,%0,%7,%0\n\t"
                    "vfmaddps %7,%1,%7,%1\n\t"
                    "vfmaddps %7,%2,%7,%2\n\t"
                    "vfmaddps %7,%3,%7,%3\n\t"
                    "vfmaddps %7,%4,%7,%4\n\t"
                    "vfmaddps %7,%5,%7,%5\n\t"
                    "dec      %6\n\t"
                    "jne 0b"         "\n\t"
                    : "+x"(x[0]), "+x"(x[1]), "+x"(x[2]), "+x"(x[3]), "+x"(x[4]), "+x"(x[5]), "+r"(i)
                    : "x"(y)
                       );
            ///////////////////////////////////////
            timer.Stop();

            asm volatile("":: "x"(x[0]), "x"(x[1]), "x"(x[2]), "x"(x[3]), "x"(x[4]), "x"(x[5]));

#elif VC_IMPL_AVX
            __m256 x[8] = { _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()) };
            const __m256 y = _mm256_set1_ps(randomF12());

            timer.Start();
            ///////////////////////////////////////
            int i = Factor;
#ifdef VC_64BIT
            __asm__(
                    ".align 16\n\t0: "
                    "vmulps  %9,%0,%0"   "\n\t"
                    "vmulps  %9,%1,%1"   "\n\t"
                    "vmulps  %9,%2,%2"   "\n\t"
                    "vmulps  %9,%7,%7"   "\n\t"
                    "vaddps  %9,%0,%0"   "\n\t"
                    "vmulps  %9,%3,%3"   "\n\t"
                    "vaddps  %9,%1,%1"   "\n\t"
                    "vmulps  %9,%4,%4"   "\n\t"
                    "vaddps  %9,%2,%2"   "\n\t"
                    "vaddps  %9,%7,%7"   "\n\t"
                    "vmulps  %9,%5,%5"   "\n\t"
                    "vaddps  %9,%3,%3"   "\n\t"
                    "vaddps  %9,%4,%4"   "\n\t"
                    "vmulps  %9,%6,%6"   "\n\t"
                    "vaddps  %9,%5,%5"   "\n\t"
                    "vaddps  %9,%6,%6"   "\n\t"
                    "dec     %8"      "\n\t"
                    "jne 0b"          "\n\t"
                    : "+x"(x[0]), "+x"(x[1]), "+x"(x[2]), "+x"(x[3]), "+x"(x[4]), "+x"(x[5]), "+x"(x[6]), "+x"(x[7]), "+r"(i)
                    : "x"(y)
                       );
#else
            __asm__(
                    ".align 16\n\t0: "
                    "vmulps  %[y],%0,%0"   "\n\t"
                    "vmulps  (%[x7]),%[y],%%ymm7"   "\n\t"
                    "vmulps  %[y],%1,%1"   "\n\t"
                    "vmulps  %[y],%2,%2"   "\n\t"
                    "vaddps  %[y],%0,%0"   "\n\t"
                    "vmulps  %[y],%3,%3"   "\n\t"
                    "vaddps  %[y],%1,%1"   "\n\t"
                    "vmulps  %[y],%4,%4"   "\n\t"
                    "vaddps  %[y],%2,%2"   "\n\t"
                    "vaddps  %[y],%%ymm7,%%ymm7"   "\n\t"
                    "vmovaps     %%ymm7,(%[x7])"   "\n\t"
                    "vmulps  %[y],%5,%5"   "\n\t"
                    "vaddps  %[y],%3,%3"   "\n\t"
                    "vaddps  %[y],%4,%4"   "\n\t"
                    "vmulps  (%[x6]),%[y],%%ymm7"   "\n\t"
                    "vaddps  %[y],%5,%5"   "\n\t"
                    "vaddps  %[y],%%ymm7,%%ymm7"   "\n\t"
                    "vmovaps     %%ymm7,(%[x6])"   "\n\t"
                    "sub     $1,%[r]"      "\n\t"
                    "jne 0b"         "\n\t"
                    : "+x"(x[0]), "+x"(x[1]), "+x"(x[2]), "+x"(x[3]), "+x"(x[4]), "+x"(x[5]), [r]"+r"(i)
                    : [x6]"r"(&x[6]), [x7]"r"(&x[7]), [y]"x"(y)
                    : "xmm7");
#endif
            ///////////////////////////////////////
            timer.Stop();

            const int k = _mm256_movemask_ps(_mm256_add_ps(_mm256_add_ps(_mm256_add_ps(x[0], x[1]), _mm256_add_ps(x[2], x[3])), _mm256_add_ps(_mm256_add_ps(x[4], x[5]), _mm256_add_ps(x[7], x[6]))));
            blackHole &= k;
#elif VC_IMPL_SSE
            __m128 x[8] = { _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()) };
            const __m128 y = _mm_set1_ps(randomF12());

            timer.Start();
            ///////////////////////////////////////
            int i = Factor;
#ifdef VC_64BIT
            __asm__(
                    ".align 16\n\t0: "
                    "mulps  %9,%0"   "\n\t"
                    "sub    $1,%8"   "\n\t"
                    "mulps  %9,%1"   "\n\t"
                    "mulps  %9,%2"   "\n\t"
                    "mulps  %9,%7"   "\n\t"
                    "addps  %9,%0"   "\n\t"
                    "mulps  %9,%3"   "\n\t"
                    "addps  %9,%1"   "\n\t"
                    "mulps  %9,%4"   "\n\t"
                    "addps  %9,%2"   "\n\t"
                    "addps  %9,%7"   "\n\t"
                    "mulps  %9,%5"   "\n\t"
                    "addps  %9,%3"   "\n\t"
                    "addps  %9,%4"   "\n\t"
                    "mulps  %9,%6"   "\n\t"
                    "addps  %9,%5"   "\n\t"
                    "addps  %9,%6"   "\n\t"
                    "jne 0b"         "\n\t"
                    : "+x"(x[0]), "+x"(x[1]), "+x"(x[2]), "+x"(x[3]), "+x"(x[4]), "+x"(x[5]), "+x"(x[6]), "+x"(x[7]), "+r"(i)
                    : "x"(y)
                       );
#else
            __m128 tmp;
            __asm__(
                    ".align 16\n\t0: "
                    "mulps  %10,%0"   "\n\t"
                    "sub    $1,%9"   "\n\t"
                    "mulps  %10,%1"   "\n\t"
                    "mulps  %10,%2"   "\n\t"
                    "movaps  %7,%8"   "\n\t"
                    "mulps  %10,%8"   "\n\t"
                    "addps  %10,%0"   "\n\t"
                    "mulps  %10,%3"   "\n\t"
                    "addps  %10,%1"   "\n\t"
                    "mulps  %10,%4"   "\n\t"
                    "addps  %10,%2"   "\n\t"
                    "addps  %10,%8"   "\n\t"
                    "movaps  %8,%7"   "\n\t"
                    "mulps  %10,%5"   "\n\t"
                    "addps  %10,%3"   "\n\t"
                    "addps  %10,%4"   "\n\t"
                    "movaps  %6,%8"   "\n\t"
                    "mulps  %10,%8"   "\n\t"
                    "addps  %10,%5"   "\n\t"
                    "addps  %10,%8"   "\n\t"
                    "movaps  %8,%6"   "\n\t"
                    "jne 0b"         "\n\t"
                    : "+x"(x[0]), "+x"(x[1]), "+x"(x[2]), "+x"(x[3]), "+x"(x[4]), "+x"(x[5]), "+m"(x[6]), "+m"(x[7]), "+x"(tmp), "+r"(i)
                    : "x"(y)
                       );
#endif
            ///////////////////////////////////////
            timer.Stop();

            const int k = _mm_movemask_ps(_mm_add_ps(_mm_add_ps(_mm_add_ps(x[0], x[1]), _mm_add_ps(x[2], x[3])), _mm_add_ps(_mm_add_ps(x[4], x[5]), _mm_add_ps(x[7], x[6]))));
            blackHole &= k;
#else
            float x[8] = { randomF12(), randomF12(), randomF12(), randomF12(), randomF12(), randomF12(), randomF12(), randomF12() };
            const float y = randomF12();

            timer.Start();
            ///////////////////////////////////////
            int i = Factor;
#ifdef VC_64BIT
            __asm__(
                    ".align 16\n\t0: "
                    "mulss  %9,%0"   "\n\t"
                    "sub    $1,%8"   "\n\t"
                    "mulss  %9,%1"   "\n\t"
                    "mulss  %9,%2"   "\n\t"
                    "mulss  %9,%7"   "\n\t"
                    "addss  %9,%0"   "\n\t"
                    "mulss  %9,%3"   "\n\t"
                    "addss  %9,%1"   "\n\t"
                    "mulss  %9,%4"   "\n\t"
                    "addss  %9,%2"   "\n\t"
                    "addss  %9,%7"   "\n\t"
                    "mulss  %9,%5"   "\n\t"
                    "addss  %9,%3"   "\n\t"
                    "addss  %9,%4"   "\n\t"
                    "mulss  %9,%6"   "\n\t"
                    "addss  %9,%5"   "\n\t"
                    "addss  %9,%6"   "\n\t"
                    "jne 0b"         "\n\t"
                    : "+x"(x[0]), "+x"(x[1]), "+x"(x[2]), "+x"(x[3]), "+x"(x[4]), "+x"(x[5]), "+x"(x[6]), "+x"(x[7]), "+r"(i)
                    : "x"(y)
                    );
#else
            float tmp;
            __asm__(
                    ".align 16\n\t0: "
                    "mulss  %10,%0"   "\n\t"
                    "sub    $1,%9"   "\n\t"
                    "mulss  %10,%1"   "\n\t"
                    "mulss  %10,%2"   "\n\t"
                    "movss  %7,%8"    "\n\t"
                    "mulss  %10,%8"   "\n\t"
                    "addss  %10,%0"   "\n\t"
                    "mulss  %10,%3"   "\n\t"
                    "addss  %10,%1"   "\n\t"
                    "mulss  %10,%4"   "\n\t"
                    "addss  %10,%2"   "\n\t"
                    "addss  %10,%8"   "\n\t"
                    "movss  %7,%8"    "\n\t"
                    "mulss  %10,%5"   "\n\t"
                    "addss  %10,%3"   "\n\t"
                    "addss  %10,%4"   "\n\t"
                    "movss  %6,%8"    "\n\t"
                    "mulss  %10,%8"   "\n\t"
                    "addss  %10,%5"   "\n\t"
                    "addss  %10,%8"   "\n\t"
                    "movss  %8,%6"    "\n\t"
                    "jne 0b"         "\n\t"
                    : "+x"(x[0]), "+x"(x[1]), "+x"(x[2]), "+x"(x[3]), "+x"(x[4]), "+x"(x[5]), "+m"(x[6]), "+m"(x[7]), "+x"(tmp), "+r"(i)
                    : "x"(y)
                    );
#endif
            ///////////////////////////////////////
            timer.Stop();

            const int k = (x[0] < x[1]) && (x[2] < x[3]) && (x[4] < x[5]) && (x[7] < x[6]);
            blackHole &= k;
#endif
        }
        timer.Print();
    }
#endif

    {
        Benchmark timer("class", 2 * 8 * float_v::Size * Factor, "FLOP");
        while (timer.wantsMoreDataPoints()) {
            const float_v alpha(-randomF(.1f, .2f));
            const float_v y = randomF12();
            float_v x[8] = { randomF12(), randomF12(), randomF12(), randomF12(), randomF12(), randomF12(), randomF12(), randomF12() };

            // force the x vectors to registers, otherwise GCC decides to work on the stack and
            // lose half of the performance
            //forceToRegisters(x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7]);

            timer.Start();
            ///////////////////////////////////////

            for (int i = 0; i < Factor; ++i) {
                    x[0] = y * x[0] + y;
                    x[1] = y * x[1] + y;
                    x[2] = y * x[2] + y;
                    x[3] = y * x[3] + y;
                    x[4] = y * x[4] + y;
                    x[5] = y * x[5] + y;
                    x[6] = y * x[6] + y;
                    x[7] = y * x[7] + y;
            }

            ///////////////////////////////////////
            timer.Stop();

            const int k = all_of((x[0] < x[1]) && (x[2] < x[3]) && (x[4] < x[5]) && (x[7] < x[6]));
            blackHole &= k;
        }
        timer.Print();
    }


    // intrinsics reference
    {
        Benchmark timer("intrinsics reference", 2 * 8 * float_v::Size * Factor, "FLOP");
        while (timer.wantsMoreDataPoints()) {
#if VC_IMPL_AVX
            __m256 x[8] = { _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()), _mm256_set1_ps(randomF12()) };
            const __m256 y = _mm256_set1_ps(randomF12());

            timer.Start();
            ///////////////////////////////////////

            for (int i = 0; i < Factor; ++i) {
#if VC_IMPL_FMA4
                    x[0] = _mm256_macc_ps(y, x[0], y);
                    x[1] = _mm256_macc_ps(y, x[1], y);
                    x[2] = _mm256_macc_ps(y, x[2], y);
                    x[3] = _mm256_macc_ps(y, x[3], y);
                    x[4] = _mm256_macc_ps(y, x[4], y);
                    x[5] = _mm256_macc_ps(y, x[5], y);
                    x[6] = _mm256_macc_ps(y, x[6], y);
                    x[7] = _mm256_macc_ps(y, x[7], y);
#else
                    x[0] = _mm256_add_ps(_mm256_mul_ps(y, x[0]), y);
                    x[1] = _mm256_add_ps(_mm256_mul_ps(y, x[1]), y);
                    x[2] = _mm256_add_ps(_mm256_mul_ps(y, x[2]), y);
                    x[3] = _mm256_add_ps(_mm256_mul_ps(y, x[3]), y);
                    x[4] = _mm256_add_ps(_mm256_mul_ps(y, x[4]), y);
                    x[5] = _mm256_add_ps(_mm256_mul_ps(y, x[5]), y);
                    x[6] = _mm256_add_ps(_mm256_mul_ps(y, x[6]), y);
                    x[7] = _mm256_add_ps(_mm256_mul_ps(y, x[7]), y);
#endif
            }

            ///////////////////////////////////////
            timer.Stop();

            const int k = _mm256_movemask_ps(_mm256_add_ps(_mm256_add_ps(_mm256_add_ps(x[0], x[1]), _mm256_add_ps(x[2], x[3])), _mm256_add_ps(_mm256_add_ps(x[4], x[5]), _mm256_add_ps(x[7], x[6]))));
            blackHole &= k;
#elif VC_IMPL_SSE
            __m128 x[8] = { _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()), _mm_set1_ps(randomF12()) };
            const __m128 y = _mm_set1_ps(randomF12());

            timer.Start();
            ///////////////////////////////////////

            for (int i = 0; i < Factor; ++i) {
                    x[0] = _mm_add_ps(_mm_mul_ps(y, x[0]), y);
                    x[1] = _mm_add_ps(_mm_mul_ps(y, x[1]), y);
                    x[2] = _mm_add_ps(_mm_mul_ps(y, x[2]), y);
                    x[3] = _mm_add_ps(_mm_mul_ps(y, x[3]), y);
                    x[4] = _mm_add_ps(_mm_mul_ps(y, x[4]), y);
                    x[5] = _mm_add_ps(_mm_mul_ps(y, x[5]), y);
                    x[6] = _mm_add_ps(_mm_mul_ps(y, x[6]), y);
                    x[7] = _mm_add_ps(_mm_mul_ps(y, x[7]), y);
            }

            ///////////////////////////////////////
            timer.Stop();

            const int k = _mm_movemask_ps(_mm_add_ps(_mm_add_ps(_mm_add_ps(x[0], x[1]), _mm_add_ps(x[2], x[3])), _mm_add_ps(_mm_add_ps(x[4], x[5]), _mm_add_ps(x[7], x[6]))));
            blackHole &= k;
#else
            float x[8] = { randomF12(), randomF12(), randomF12(), randomF12(), randomF12(), randomF12(), randomF12(), randomF12() };
            const float y = randomF12();

            timer.Start();
            ///////////////////////////////////////

            for (int i = 0; i < Factor; ++i) {
                    x[0] = y * x[0] + y;
                    x[1] = y * x[1] + y;
                    x[2] = y * x[2] + y;
                    x[3] = y * x[3] + y;
                    x[4] = y * x[4] + y;
                    x[5] = y * x[5] + y;
                    x[6] = y * x[6] + y;
                    x[7] = y * x[7] + y;
            }

            ///////////////////////////////////////
            timer.Stop();

            const int k = (x[0] < x[1]) && (x[2] < x[3]) && (x[4] < x[5]) && (x[7] < x[6]);
            blackHole &= k;
#endif
        }
        timer.Print();
    }
    if (blackHole != 0) {
        std::cout << std::endl;
    }
    return 0;
}
