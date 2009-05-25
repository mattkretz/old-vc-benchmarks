/*
    Copyright (C) 2009 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#include "../vector.h"
#include "vecio.h"
#include "unittest.h"
#include <iostream>

using namespace Vc;

enum {
    VectorSizeFactor = short_v::Size / int_v::Size
};

void testSigned()
{
    for (int start = -32000; start < 32000; start += 5) {
        int_v a[VectorSizeFactor];
        for (int i = 0; i < VectorSizeFactor; ++i) {
            a[i] = int_v(IndexesFromZero) + int_v::Size * i + start;
        }
        short_v b(a);
        COMPARE(b, short_v(IndexesFromZero) + start);

        int_v c[VectorSizeFactor];
        b.expand(c);
        for (int i = 0; i < VectorSizeFactor; ++i) {
            COMPARE(c[i], int_v(IndexesFromZero) + int_v::Size * i + start);
        }
    }
}

void testUnsigned()
{
#ifdef __SSE4_1__
    for (uint start = 0; start < 64000; start += 5) {
#else
    for (uint start = 0; start < 32000; start += 5) {
#endif
        uint_v a[VectorSizeFactor];
        for (uint i = 0; i < VectorSizeFactor; ++i) {
            a[i] = uint_v(IndexesFromZero) + uint_v::Size * i + start;
        }
        ushort_v b(a);
        COMPARE(b, ushort_v(IndexesFromZero) + start);

        uint_v c[VectorSizeFactor];
        b.expand(c);
        for (uint i = 0; i < VectorSizeFactor; ++i) {
            COMPARE(c[i], uint_v(IndexesFromZero) + uint_v::Size * i + start);
        }
    }
    for (uint start = 32000; start < 64000; start += 5) {
        ushort_v b(IndexesFromZero);
        b += start;
        COMPARE(b, ushort_v(IndexesFromZero) + start);

        uint_v c[VectorSizeFactor];
        b.expand(c);
        for (uint i = 0; i < VectorSizeFactor; ++i) {
            COMPARE(c[i], uint_v(IndexesFromZero) + uint_v::Size * i + start);
        }
    }
}

int main()
{
    runTest(testSigned);
    runTest(testUnsigned);
    return 0;
}