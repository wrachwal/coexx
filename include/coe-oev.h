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

// ---------------------------------------------------------------------------
#define COE_X0(x)       /**/
#define COE_X1(x)                  x
#define COE_X2(x)       COE_X1(x), x
#define COE_X3(x)       COE_X2(x), x
#define COE_X4(x)       COE_X3(x), x
#define COE_X5(x)       COE_X4(x), x
#define COE_X6(x)       COE_X5(x), x
#define COE_X7(x)       COE_X6(x), x
#define COE_X8(x)       COE_X7(x), x
#define COE_X9(x)       COE_X8(x), x
#define COE_X10(x)      COE_X9(x), x
#define COE_X(i, x)     COE_X##i(x)
// ------------------------------------
#define COE_T0(t)       /**/
#define COE_T1(t)                  t##1
#define COE_T2(t)       COE_T1(t), t##2
#define COE_T3(t)       COE_T2(t), t##3
#define COE_T4(t)       COE_T3(t), t##4
#define COE_T5(t)       COE_T4(t), t##5
#define COE_T6(t)       COE_T5(t), t##6
#define COE_T7(t)       COE_T6(t), t##7
#define COE_T8(t)       COE_T7(t), t##8
#define COE_T9(t)       COE_T8(t), t##9
#define COE_T(i, t)     COE_T##i(t)
// ------------------------------------
#define COE_TA0(t, a)   /**/
#define COE_TA1(t, a)                  t##1 a##1
#define COE_TA2(t, a)   COE_TA1(t, a), t##2 a##2
#define COE_TA3(t, a)   COE_TA2(t, a), t##3 a##3
#define COE_TA4(t, a)   COE_TA3(t, a), t##4 a##4
#define COE_TA5(t, a)   COE_TA4(t, a), t##5 a##5
#define COE_TA6(t, a)   COE_TA5(t, a), t##6 a##6
#define COE_TA7(t, a)   COE_TA6(t, a), t##7 a##7
#define COE_TA8(t, a)   COE_TA7(t, a), t##8 a##8
#define COE_TA9(t, a)   COE_TA8(t, a), t##9 a##9
#define COE_TA(i, t, a) COE_TA##i(t, a)
// ------------------------------------
#define COE_PA0(p, a)   /**/
#define COE_PA1(p, a)                  p##1(a##1)
#define COE_PA2(p, a)   COE_PA1(p, a), p##2(a##2)
#define COE_PA3(p, a)   COE_PA2(p, a), p##3(a##3)
#define COE_PA4(p, a)   COE_PA3(p, a), p##4(a##4)
#define COE_PA5(p, a)   COE_PA4(p, a), p##5(a##5)
#define COE_PA6(p, a)   COE_PA5(p, a), p##6(a##6)
#define COE_PA7(p, a)   COE_PA6(p, a), p##7(a##7)
#define COE_PA8(p, a)   COE_PA7(p, a), p##8(a##8)
#define COE_PA9(p, a)   COE_PA8(p, a), p##9(a##9)
#define COE_PA(i, p, a) COE_PA##i(p, a)
// ---------------------------------------------------------------------------

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

template<class L> struct Car;
template<class H, class T>
struct Car<Cons<H, T> > {
    typedef H type;
};
template<>
struct Car<Nil> {
    typedef Nil type;
};

// ------------------------------------

template<class L> struct Cdr;
template<class H, class T>
struct Cdr<Cons<H, T> > {
    typedef T type;
};
template<>
struct Cdr<Nil> {
    typedef Nil type;
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

// ------------------------------------
// Reverse<List>

template<class, class> struct Reverse_Acc;
template<class Acc>
struct Reverse_Acc<Nil, Acc> {
    typedef Acc type;
};
template<class Head, class Tail, class Acc>
struct Reverse_Acc<Cons<Head, Tail>, Acc> : Reverse_Acc<Tail, Cons<Head, Acc> > {};
// ------
template<class List>
struct Reverse : Reverse_Acc<List, Nil> {};

// ------------------------------------

template<class Pred, class List> struct MemberIf;
template<class Pred, class H, class T>
struct MemberIf<Pred, Cons<H, T> > {
    typedef typename meta::If_<Pred::template apply<H>::value, H, typename MemberIf<Pred, T>::type>::type type;
};
template<class Pred>
struct MemberIf<Pred, Nil> {
    typedef Nil type;
};

// ------------------------------------
// Common<Lst1, Lst2>

template<class Lst1, class Lst2>
struct Common;
template<class H1, class T1, class H2, class T2>
struct Common<Cons<H1, T1>, Cons<H2, T2> > {
    typedef Nil type;
};
template<class H, class T1, class T2>
struct Common<Cons<H, T1>, Cons<H, T2> > {
    typedef Cons<H, typename Common<T1, T2>::type> type;
};
template<class Lst>
struct Common<Lst, Nil> {
    typedef Nil type;
};
template<class Lst>
struct Common<Nil, Lst> {
    typedef Nil type;
};
template<>
struct Common<Nil, Nil> {
    typedef Nil type;
};

// ---------------------------------------------------------------------------

template<COE_T(1, class A)> struct List1 {
    typedef Cons<A1, Nil> type;
};
template<COE_T(2, class A)> struct List2 {
    typedef Cons<A1, Cons<A2, Nil> > type;
};
template<COE_T(3, class A)> struct List3 {
    typedef Cons<A1, Cons<A2, Cons<A3, Nil> > > type;
};
template<COE_T(4, class A)> struct List4 {
    typedef Cons<A1, Cons<A2, Cons<A3, Cons<A4, Nil> > > > type;
};
template<COE_T(5, class A)> struct List5 {
    typedef Cons<A1, Cons<A2, Cons<A3, Cons<A4, Cons<A5, Nil> > > > > type;
};
template<COE_T(6, class A)> struct List6 {
    typedef Cons<A1, Cons<A2, Cons<A3, Cons<A4, Cons<A5, Cons<A6, Nil> > > > > > type;
};
template<COE_T(7, class A)> struct List7 {
    typedef Cons<A1, Cons<A2, Cons<A3, Cons<A4, Cons<A5, Cons<A6, Cons<A7, Nil> > > > > > > type;
};
template<COE_T(8, class A)> struct List8 {
    typedef Cons<A1, Cons<A2, Cons<A3, Cons<A4, Cons<A5, Cons<A6, Cons<A7, Cons<A8, Nil> > > > > > > > type;
};
template<COE_T(9, class A)> struct List9 {
    typedef Cons<A1, Cons<A2, Cons<A3, Cons<A4, Cons<A5, Cons<A6, Cons<A7, Cons<A8, Cons<A9, Nil> > > > > > > > > type;
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

class Kernel;

template<>                              struct event<COE_X(10, void)> {
    typedef Nil args_type;
};
template<COE_T(1, class A)> struct event<COE_T(1, A), COE_X(9, void)> {
    typedef typename List1<COE_T(1, A)>::type args_type;
};
template<COE_T(2, class A)> struct event<COE_T(2, A), COE_X(8, void)> {
    typedef typename List2<COE_T(2, A)>::type args_type;
};
template<COE_T(3, class A)> struct event<COE_T(3, A), COE_X(7, void)> {
    typedef typename List3<COE_T(3, A)>::type args_type;
};
template<COE_T(4, class A)> struct event<COE_T(4, A), COE_X(6, void)> {
    typedef typename List4<COE_T(4, A)>::type args_type;
};
template<COE_T(5, class A)> struct event<COE_T(5, A), COE_X(5, void)> {
    typedef typename List5<COE_T(5, A)>::type args_type;
};
template<COE_T(6, class A)> struct event<COE_T(6, A), COE_X(4, void)> {
    typedef typename List6<COE_T(6, A)>::type args_type;
};
template<COE_T(7, class A)> struct event<COE_T(7, A), COE_X(3, void)> {
    typedef typename List7<COE_T(7, A)>::type args_type;
};
template<COE_T(8, class A)> struct event<COE_T(8, A), COE_X(2, void)> {
    typedef typename List8<COE_T(8, A)>::type args_type;
};
template<COE_T(9, class A)> struct event<COE_T(9, A), COE_X(1, void)> {
    typedef typename List9<COE_T(9, A)>::type args_type;
};

// ---------------------------------------------------------------------------
// handler_type<ARGS>

template<class ARGS, int N = Length<ARGS>::value>
struct handler_type;

template<class ARGS>
struct handler_type<ARGS, 0> {
    typedef void (*fun_type)(Kernel&);
    template<class Obj> struct mem_fun {
        typedef void (Obj::*type)(Kernel&);
    };
};

template<class ARGS>
struct handler_type<ARGS, 1> {
    typedef typename Nth<0, ARGS>::type A1;
    typedef void (*fun_type)(Kernel&, COE_TA(1, A, &a));
    template<class Obj> struct mem_fun {
        typedef void (Obj::*type)(Kernel&, COE_TA(1, A, &a));
    };
};

template<class ARGS>
struct handler_type<ARGS, 2> {
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef void (*fun_type)(Kernel&, COE_TA(2, A, &a));
    template<class Obj> struct mem_fun {
        typedef void (Obj::*type)(Kernel&, COE_TA(2, A, &a));
    };
};

template<class ARGS>
struct handler_type<ARGS, 3> {
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef void (*fun_type)(Kernel&, COE_TA(3, A, &a));
    template<class Obj> struct mem_fun {
        typedef void (Obj::*type)(Kernel&, COE_TA(3, A, &a));
    };
};

template<class ARGS>
struct handler_type<ARGS, 4> {
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef void (*fun_type)(Kernel&, COE_TA(4, A, &a));
    template<class Obj> struct mem_fun {
        typedef void (Obj::*type)(Kernel&, COE_TA(4, A, &a));
    };
};

template<class ARGS>
struct handler_type<ARGS, 5> {
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef void (*fun_type)(Kernel&, COE_TA(5, A, &a));
    template<class Obj> struct mem_fun {
        typedef void (Obj::*type)(Kernel&, COE_TA(5, A, &a));
    };
};

template<class ARGS>
struct handler_type<ARGS, 6> {
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    typedef void (*fun_type)(Kernel&, COE_TA(6, A, &a));
    template<class Obj> struct mem_fun {
        typedef void (Obj::*type)(Kernel&, COE_TA(6, A, &a));
    };
};

template<class ARGS>
struct handler_type<ARGS, 7> {
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    typedef typename Nth<6, ARGS>::type A7;
    typedef void (*fun_type)(Kernel&, COE_TA(7, A, &a));
    template<class Obj> struct mem_fun {
        typedef void (Obj::*type)(Kernel&, COE_TA(7, A, &a));
    };
};

template<class ARGS>
struct handler_type<ARGS, 8> {
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    typedef typename Nth<6, ARGS>::type A7;
    typedef typename Nth<7, ARGS>::type A8;
    typedef void (*fun_type)(Kernel&, COE_TA(8, A, &a));
    template<class Obj> struct mem_fun {
        typedef void (Obj::*type)(Kernel&, COE_TA(8, A, &a));
    };
};

template<class ARGS>
struct handler_type<ARGS, 9> {
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    typedef typename Nth<6, ARGS>::type A7;
    typedef typename Nth<7, ARGS>::type A8;
    typedef typename Nth<8, ARGS>::type A9;
    typedef void (*fun_type)(Kernel&, COE_TA(9, A, &a));
    template<class Obj> struct mem_fun {
        typedef void (Obj::*type)(Kernel&, COE_TA(9, A, &a));
    };
};

// ===========================================================================

} ///// namespace coe

#endif

