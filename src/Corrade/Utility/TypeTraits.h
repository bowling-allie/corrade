#ifndef Corrade_Utility_TypeTraits_h
#define Corrade_Utility_TypeTraits_h
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
 * @brief Macros @ref CORRADE_HAS_TYPE(), alias @ref Corrade::Utility::IsIterable
 */

#include <type_traits>

#include "Corrade/configure.h"

namespace Corrade { namespace Utility {

/** @hideinitializer
@brief Whether the @ref std::is_trivially_copyable family of type traits is supported by the standard library
@m_since_latest

The @ref std::is_trivially_constructible,
@ref std::is_trivially_default_constructible,
@ref std::is_trivially_copy_constructible,
@ref std::is_trivially_move_constructible,
@ref std::is_trivially_assignable,
@ref std::is_trivially_copy_assignable,
@ref std::is_trivially_move_assignable family of traits is not implemented
[on libstdc++ below version 5](https://gcc.gnu.org/gcc-5/changes.html).
Instead, legacy @cpp std::has_trivial_default_constructor @ce,
@cpp std::has_trivial_copy_constructor @ce and
@cpp std::has_trivial_copy_assign @ce are available, with slightly different
semantics (see e.g. https://stackoverflow.com/q/12754886 for more information).
From libstdc++ 5 onwards these are marked as deprecated and
[libstdc++ 7 removes them](https://gcc.gnu.org/gcc-7/changes.html), so
alternatively there are @cpp __has_trivial_constructor() @ce,
@cpp __has_trivial_copy() @ce and @cpp __has_trivial_assign() @ce builtins that
don't produce any deprecated warnings and are available until GCC 9 / Clang 10
at least --- however note that for SFINAE you need to wrap them in
@ref std::integral_constant as otherwise GCC would throw errors similar to the
following:

@m_class{m-console-wrap}

@code{.shell-session}
error: use of built-in trait ‘__has_trivial_copy(T)’ in function signature; use library traits instead
@endcode

This macro is defined if the standard variants are available. Unfortunately,
when libstdc++ is used through Clang, there's no way to check for its version
until libstdc++ 7, which added the `_GLIBCXX_RELEASE` macro. That means, when
using Clang with libstdc++ 5 or 6, it will still report those traits as being
unavailable. Both libc++ and MSVC STL have these traits in all versions
supported by Corrade, so there the macro is defined always.
@see @ref CORRADE_TARGET_LIBSTDCXX, @ref CORRADE_TARGET_LIBCXX,
    @ref CORRADE_TARGET_DINKUMWARE
*/
#if defined(DOXYGEN_GENERATING_OUTPUT) || !defined(CORRADE_TARGET_LIBSTDCXX) || __GNUC__ >= 5 || _GLIBCXX_RELEASE >= 7
#define CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
#endif

/** @hideinitializer
@brief Macro for creating traits class that checks for type expression validity
@param className        Resulting class name
@param ...              Type expression to check. Variadic parameter to allow
    unrestricted usage of template expressions containing commas.

Defines a traits class checking whether an expression is valid. You can use
@cpp T @ce to reference the type which is being checked; the type expression
can contain

Usage examples: checking for presence of a @cpp key_type @ce member
@cpp typedef @ce:

@snippet Utility.cpp CORRADE_HAS_TYPE-type

Checking for presence of @cpp size() @ce member function:

@snippet Utility.cpp CORRADE_HAS_TYPE-function
*/
/* Two overloaded get() functions return type of different size. Templated
   get() is used when T has given attribute, non-templated otherwise. Bool
   value then indicates whether the templated version was called or not. */
#define CORRADE_HAS_TYPE(className, ...)                                    \
template<class U> class className {                                         \
    template<class T> static char get(T&&, __VA_ARGS__* = nullptr);         \
    static short get(...);                                                  \
    public:                                                                 \
        enum: bool { value = sizeof(get(std::declval<U>())) == sizeof(char) }; \
}

namespace Implementation {
    CORRADE_HAS_TYPE(HasMemberBegin, decltype(std::declval<T>().begin()));
    CORRADE_HAS_TYPE(HasMemberEnd, decltype(std::declval<T>().end()));
    CORRADE_HAS_TYPE(HasBegin, decltype(begin(std::declval<T>())));
    CORRADE_HAS_TYPE(HasEnd, decltype(end(std::declval<T>())));
    CORRADE_HAS_TYPE(HasMemberCStr, decltype(std::declval<T>().c_str()));
}

/**
@brief Traits class for checking whether given type is iterable

Equivalent to @ref std::true_type if the class is has either @cpp begin() @ce /
@cpp end() @ce members, is usable with free @cpp begin() @ce / @cpp end() @ce
functions or has @ref std::begin() / @ref std::end() overloads. Otherwise
equivalent to @ref std::false_type.

Used together with @ref IsStringLike by @ref Debug to decide whether given type
should be printed as a container of its contents or as a whole.
@todoc use the ellipsis macro once m.css has it
*/
/* When using {}, MSVC 2015 complains that even the explicitly defaulted
   constructor doesn't exist */
template<class T> using IsIterable = std::integral_constant<bool,
    #ifndef DOXYGEN_GENERATING_OUTPUT
    (Implementation::HasMemberBegin<T>::value || Implementation::HasBegin<T>::value) &&
    (Implementation::HasMemberEnd<T>::value || Implementation::HasEnd<T>::value)
    #else
    implementation-specific
    #endif
    >;

/**
@brief Traits class for checking whether given type is iterable
@m_since{2019,10}

Equivalent to @ref std::true_type if the class is has a @cpp c_str() @ce
member. Otherwise equivalent to @ref std::false_type. Useful for dispatching
on the @ref std::string type without having to include or
@ref StlForwardString.h "forward-declare" it.

Used together with @ref IsIterable by @ref Debug to decide whether given type
should be printed as a container of its contents or as a whole.
@todoc use the ellipsis macro once m.css has it
*/
template<class T> using IsStringLike = std::integral_constant<bool,
    #ifndef DOXYGEN_GENERATING_OUTPUT
    Implementation::HasMemberCStr<T>::value
    #else
    implementation-specific
    #endif
    >;

}}

#endif
