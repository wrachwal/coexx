// coe-oev.h

/*****************************************************************************
Copyright (c) 2008-2011 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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

#ifndef __COE_OEV_H
#define __COE_OEV_H

namespace coe { /////

// ===========================================================================
// Lisp

struct Nil {};

// ------------------------------------

template<class Car, class Cdr>
struct Cons {
    typedef Car car_type;
    typedef Cdr cdr_type;
};

// ------------------------------------

template<class> struct Length;
template<>
struct Length<Nil> {
    enum { value = 0 };
};
template<class Head, class Tail>
struct Length<Cons<Head, Tail> > {
    enum { value = 1 + Length<Tail>::value };
};

// ------------------------------------

template<int N, class Lst> struct Nth;
template<int N>
struct Nth<N, Nil> {
    typedef Nil type;
};
template<class Car, class Cdr>
struct Nth<0, Cons<Car, Cdr> > {
    typedef Car type;
};
template<int N, class Car, class Cdr>
struct Nth<N, Cons<Car, Cdr> > {
    typedef typename Nth<N - 1, Cdr>::type type;
};

// ---------------------------------------------------------------------------

template<class A1>
struct List1 {
    typedef Cons<A1,
                 Nil> type;
};

template<class A1,
         class A2>
struct List2 {
    typedef Cons<A1,
            Cons<A2,
                 Nil> > type;
};

template<class A1,
         class A2,
         class A3>
struct List3 {
    typedef Cons<A1,
            Cons<A2,
            Cons<A3,
                 Nil> > > type;
};

template<class A1,
         class A2,
         class A3,
         class A4>
struct List4 {
    typedef Cons<A1,
            Cons<A2,
            Cons<A3,
            Cons<A4,
                 Nil> > > > type;
};

template<class A1,
         class A2,
         class A3,
         class A4,
         class A5>
struct List5 {
    typedef Cons<A1,
            Cons<A2,
            Cons<A3,
            Cons<A4,
            Cons<A5,
                 Nil> > > > > type;
};

template<class A1,
         class A2,
         class A3,
         class A4,
         class A5,
         class A6>
struct List6 {
    typedef Cons<A1,
            Cons<A2,
            Cons<A3,
            Cons<A4,
            Cons<A5,
            Cons<A6,
                 Nil> > > > > > type;
};

template<class A1,
         class A2,
         class A3,
         class A4,
         class A5,
         class A6,
         class A7>
struct List7 {
    typedef Cons<A1,
            Cons<A2,
            Cons<A3,
            Cons<A4,
            Cons<A5,
            Cons<A6,
            Cons<A7,
                 Nil> > > > > > > type;
};

template<class A1,
         class A2,
         class A3,
         class A4,
         class A5,
         class A6,
         class A7,
         class A8>
struct List8 {
    typedef Cons<A1,
            Cons<A2,
            Cons<A3,
            Cons<A4,
            Cons<A5,
            Cons<A6,
            Cons<A7,
            Cons<A8,
                 Nil> > > > > > > > type;
};

template<class A1,
         class A2,
         class A3,
         class A4,
         class A5,
         class A6,
         class A7,
         class A8,
         class A9>
struct List9 {
    typedef Cons<A1,
            Cons<A2,
            Cons<A3,
            Cons<A4,
            Cons<A5,
            Cons<A6,
            Cons<A7,
            Cons<A8,
            Cons<A9,
                 Nil> > > > > > > > > type;
};

// ===========================================================================
// event<[A1...A9]>

template<class A1 = void,
         class A2 = void,
         class A3 = void,
         class A4 = void,
         class A5 = void,
         class A6 = void,
         class A7 = void,
         class A8 = void,
         class A9 = void,
         class A_ = void>
struct event;

// ------------------------------------

template<>
struct event<void, void, void, void, void, void, void, void, void, void> {
    typedef Nil args_type;
};

template<class A1>
struct event<A1, void, void, void, void, void, void, void, void, void> {
    typedef typename List1<A1>::type args_type;
};

template<class A1, class A2>
struct event<A1, A2, void, void, void, void, void, void, void, void> {
    typedef typename List2<A1, A2>::type args_type;
};

template<class A1, class A2, class A3>
struct event<A1, A2, A3, void, void, void, void, void, void, void> {
    typedef typename List3<A1, A2, A3>::type args_type;
};

template<class A1, class A2, class A3, class A4>
struct event<A1, A2, A3, A4, void, void, void, void, void, void> {
    typedef typename List4<A1, A2, A3, A4>::type args_type;
};

template<class A1, class A2, class A3, class A4, class A5>
struct event<A1, A2, A3, A4, A5, void, void, void, void, void> {
    typedef typename List5<A1, A2, A3, A4, A5>::type args_type;
};

template<class A1, class A2, class A3, class A4, class A5, class A6>
struct event<A1, A2, A3, A4, A5, A6, void, void, void, void> {
    typedef typename List6<A1, A2, A3, A4, A5, A6>::type args_type;
};

template<class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct event<A1, A2, A3, A4, A5, A6, A7, void, void, void> {
    typedef typename List7<A1, A2, A3, A4, A5, A6, A7>::type args_type;
};

template<class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct event<A1, A2, A3, A4, A5, A6, A7, A8, void, void> {
    typedef typename List8<A1, A2, A3, A4, A5, A6, A7, A8>::type args_type;
};

template<class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct event<A1, A2, A3, A4, A5, A6, A7, A8, A9, void> {
    typedef typename List9<A1, A2, A3, A4, A5, A6, A7, A8, A9>::type args_type;
};

// ===========================================================================

} ///// namespace coe

#endif

