// coe--local.h

/*****************************************************************************
Copyright (c) 2008-2010 Waldemar Rachwal <waldemar.rachwal@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*****************************************************************************/

#ifndef __COE__LOCAL_H
#define __COE__LOCAL_H

namespace coe { /////

// ===========================================================================
// _Noncopyable

class _Noncopyable {
public:
    _Noncopyable () {}
private:
    _Noncopyable            (const _Noncopyable&);
    _Noncopyable& operator= (const _Noncopyable&);
};

// ===========================================================================
// _SafeBool<T, D>

template<class T, class D>
struct _SafeBool {
    typedef D T::*Type;
};

// ---------------------------------------------------------------------------
// _SafeBoolBase<T>
//  (to be derived by classes that don't define own == and != operators)

template<class T>
class _SafeBoolBase {
    bool this_class_does_not_support_comparisons () const;
};

// ------------------------------------

template<class T1, class T2>
bool operator== (const _SafeBoolBase<T1>& lhs, const _SafeBoolBase<T2>&)
    { return /*compile-time error*/lhs.this_class_does_not_support_comparisons(); }

template<class T1, class T2>
bool operator!= (const _SafeBoolBase<T1>& lhs, const _SafeBoolBase<T2>&)
    { return /*compile-time error*/lhs.this_class_does_not_support_comparisons(); }

// ===========================================================================
// tiny metaprogramming pieces

namespace meta {

template<class X, class Y> struct IsSame_       { enum { value = false }; };
template<class X>          struct IsSame_<X, X> { enum { value = true  }; };

template<bool C, class T, class E> struct If_;
template        <class T, class E> struct If_<true,  T, E> { typedef T type; };
template        <class T, class E> struct If_<false, T, E> { typedef E type; };

} // namespace meta

// ===========================================================================

} ///// namespace coe

#endif

