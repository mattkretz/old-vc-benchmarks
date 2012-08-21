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

#ifndef RANDOM_H
#define RANDOM_H

#include <Vc/Vc>
#include <cstdlib>
#include <limits>

// this is not a random number generator
template<typename Vector> class PseudoRandom
{
    public:
        static Vector next();
};

template<typename V> V inline PseudoRandom<V>::next() {
    return V::Random();
}

#endif // RANDOM_H
