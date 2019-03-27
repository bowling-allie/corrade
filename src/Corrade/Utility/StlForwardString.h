#ifndef Corrade_Utility_StlForwardString_h
#define Corrade_Utility_StlForwardString_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/** @file
@brief Forward declaration for @ref std::string

On @ref CORRADE_TARGET_LIBCXX "libc++" and
@ref CORRADE_TARGET_LIBSTDCXX "libstdc++" includes a lightweight
implementation-specific STL header containing just the forward declaration of
@ref std::string. On @ref CORRADE_TARGET_DINKUMWARE "MSVC STL" and other
implementations where forward declaration is not possible or is unknown is
equivalent to @cpp #include <string> @ce.
@see @ref Corrade/Utility/StlForwardArray.h,
    @ref Corrade/Utility/StlForwardTuple.h,
    @ref Corrade/Utility/StlForwardVector.h
*/

#include "Corrade/configure.h"

#ifdef CORRADE_TARGET_LIBCXX
/* https://github.com/llvm-mirror/libcxx/blob/8c58c2293739d3d090c721827e4217c113ced89f/include/iosfwd#L190-L195 */
#include <iosfwd>
#elif defined(CORRADE_TARGET_LIBSTDCXX)
/* https://github.com/gcc-mirror/gcc/blob/2a4787da69071b5d5bc178795accca264073b7e4/libstdc%2B%2B-v3/include/bits/stringfwd.h#L68-L73 */
#include <bits/stringfwd.h>
#else
/* Including the full definition otherwise. MSVC has the full definition in
   <xstring>, which prevents any forward declaration */
#include <string>
#endif

#endif