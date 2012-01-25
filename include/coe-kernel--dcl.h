// coe-kernel--dcl.h

/*****************************************************************************
Copyright (c) 2008-2019 Waldemar Rachwał <waldemar.rachwal@gmail.com>

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

#if !defined(__COE_KERNEL_H) || defined(__COE_KERNEL__DCL_H)
#error "Never include this header directly!"
#endif

#ifndef __COE_KERNEL__DCL_H
#define __COE_KERNEL__DCL_H

#include "coe-oev.h"
#include "coe-meta.h"

namespace coe { /////

// ---------------------------------------------------------------------------

class Kernel;

// ---------------------------------------------------------------------------
// ArgI

struct ArgI {
    TypeInfo    type;
};

template<class Type>
struct init_meta_info<Type, ArgI> {
    void operator() (ArgI& info) const
        {
            info.type = typeid(Type);
        }
};

// ---------------------------------------------------------------------------
// ArgListI

struct ArgListI {
    const Meta<ArgI>**  arg;
    size_t              len;
};

template<class List> struct assign_arg_info;
template<>
struct assign_arg_info<Nil> {
    static void apply (const Meta<ArgI>**) {}
};
template<class Head, class Tail>
struct assign_arg_info<Cons<Head, Tail> > {
    static void apply (const Meta<ArgI>** argi)
        {
            *argi = Ctti<Head, ArgI>::meta();
            assign_arg_info<Tail>::apply(++argi);
        }
};

template<class List>
struct init_meta_info<List, ArgListI> {
    void operator() (ArgListI& info) const
        {
            static const Meta<ArgI>*    args[Length<List>::value + 1];
            info.arg = args;
            info.len = Length<List>::value;
            assign_arg_info<List>::apply(args);
        }
};

// ------------------------------------

bool syntax_check (const Meta<ArgListI>* hT,
                   const Meta<ArgListI>* xT,
                   const Meta<ArgListI>* aT);

// ===========================================================================
// ValParam_<ARGS>

class ValParam;
    template<class ARGS, int N = Length<ARGS>::value> class ValParamA;

template<class ARGS>
class ValParam_ {
public:
    operator ValParam* () { return _ptr; }
    ValParam*    clear () { ValParam* tmp = _ptr; _ptr = 0; return tmp; }

    ValParam_ () : _ptr(0) {}
    ValParam_ (ValParamA<ARGS, 1>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 2>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 3>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 4>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 5>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 6>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 7>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 8>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 9>* ptr) : _ptr(ptr) {}

private:
    ValParam*   _ptr;
};

// ===========================================================================
// RefParam_<ARGS>

class RefParam;
template<class ARGS, int N = Length<ARGS>::value> class RefParam_;

// ===========================================================================
// HandlerX

class HandlerX {
public:
    HandlerX () : _obj(0), _tdn(0) { _fun.g0 = 0; }

    class _Obj; class A;    // nowhere defined

    operator _SafeBool<HandlerX, _Obj*>::Type () const
        {
            return _obj ? (_fun.m0 ? &HandlerX::_obj : 0)
                        : (_fun.g0 ? &HandlerX::_obj : 0);
        }

    bool operator== (const HandlerX& rhs) const
        {
            return  _obj == rhs._obj
                && (_obj ? _fun.m0 == rhs._fun.m0
                         : _fun.g0 == rhs._fun.g0);
        }
    bool operator!= (const HandlerX& rhs) const { return ! operator==(rhs); }

    const Meta<ArgListI>* par_type () const { return _tdn; }

    void execute (Kernel& kernel, void* arg[]) const;

    // --------------------------------

    typedef void (_Obj::*MFun0) (Kernel&);
    typedef void       (*GFun0) (Kernel&);
    typedef void (_Obj::*MFun1) (Kernel&, A&);
    typedef void       (*GFun1) (Kernel&, A&);
    typedef void (_Obj::*MFun2) (Kernel&, A&, A&);
    typedef void       (*GFun2) (Kernel&, A&, A&);
    typedef void (_Obj::*MFun3) (Kernel&, A&, A&, A&);
    typedef void       (*GFun3) (Kernel&, A&, A&, A&);
    typedef void (_Obj::*MFun4) (Kernel&, A&, A&, A&, A&);
    typedef void       (*GFun4) (Kernel&, A&, A&, A&, A&);
    typedef void (_Obj::*MFun5) (Kernel&, A&, A&, A&, A&, A&);
    typedef void       (*GFun5) (Kernel&, A&, A&, A&, A&, A&);
    typedef void (_Obj::*MFun6) (Kernel&, A&, A&, A&, A&, A&, A&);
    typedef void       (*GFun6) (Kernel&, A&, A&, A&, A&, A&, A&);
    typedef void (_Obj::*MFun7) (Kernel&, A&, A&, A&, A&, A&, A&, A&);
    typedef void       (*GFun7) (Kernel&, A&, A&, A&, A&, A&, A&, A&);
    typedef void (_Obj::*MFun8) (Kernel&, A&, A&, A&, A&, A&, A&, A&, A&);
    typedef void       (*GFun8) (Kernel&, A&, A&, A&, A&, A&, A&, A&, A&);
    typedef void (_Obj::*MFun9) (Kernel&, A&, A&, A&, A&, A&, A&, A&, A&, A&);
    typedef void       (*GFun9) (Kernel&, A&, A&, A&, A&, A&, A&, A&, A&, A&);

    // --------------------------------

    template<class Obj>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&))
        : _obj((_Obj*)&obj), _tdn(0)
        { _fun.m0 = MFun0(fun); }

    template<class Obj, COE_T(1, class A)>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(1, A, &a)))
        : _obj((_Obj*)&obj), _tdn(Ctti<typename List1<COE_T(1, A)>::type, ArgListI>::meta())
        { _fun.m1 = MFun1(fun); }

    template<class Obj, COE_T(2, class A)>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(2, A, &a)))
        : _obj((_Obj*)&obj), _tdn(Ctti<typename List2<COE_T(2, A)>::type, ArgListI>::meta())
        { _fun.m2 = MFun2(fun); }

    template<class Obj, COE_T(3, class A)>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(3, A, &a)))
        : _obj((_Obj*)&obj), _tdn(Ctti<typename List3<COE_T(3, A)>::type, ArgListI>::meta())
        { _fun.m3 = MFun3(fun); }

    template<class Obj, COE_T(4, class A)>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(4, A, &a)))
        : _obj((_Obj*)&obj), _tdn(Ctti<typename List4<COE_T(4, A)>::type, ArgListI>::meta())
        { _fun.m4 = MFun4(fun); }

    template<class Obj, COE_T(5, class A)>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(5, A, &a)))
        : _obj((_Obj*)&obj), _tdn(Ctti<typename List5<COE_T(5, A)>::type, ArgListI>::meta())
        { _fun.m5 = MFun5(fun); }

    template<class Obj, COE_T(6, class A)>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(6, A, &a)))
        : _obj((_Obj*)&obj), _tdn(Ctti<typename List6<COE_T(6, A)>::type, ArgListI>::meta())
        { _fun.m6 = MFun6(fun); }

    template<class Obj, COE_T(7, class A)>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(7, A, &a)))
        : _obj((_Obj*)&obj), _tdn(Ctti<typename List7<COE_T(7, A)>::type, ArgListI>::meta())
        { _fun.m7 = MFun7(fun); }

    template<class Obj, COE_T(8, class A)>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(8, A, &a)))
        : _obj((_Obj*)&obj), _tdn(Ctti<typename List8<COE_T(8, A)>::type, ArgListI>::meta())
        { _fun.m8 = MFun8(fun); }

    template<class Obj, COE_T(9, class A)>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(9, A, &a)))
        : _obj((_Obj*)&obj), _tdn(Ctti<typename List9<COE_T(9, A)>::type, ArgListI>::meta())
        { _fun.m9 = MFun9(fun); }

    // --------------------------------

    HandlerX (void (*fun)(Kernel&))
        : _obj(0), _tdn(0)
        { _fun.g0 = GFun0(fun); }

    template<COE_T(1, class A)>
    HandlerX (void (*fun)(Kernel&, COE_TA(1, A, &a)))
        : _obj(0), _tdn(Ctti<typename List1<COE_T(1, A)>::type, ArgListI>::meta())
        { _fun.g1 = GFun1(fun); }

    template<COE_T(2, class A)>
    HandlerX (void (*fun)(Kernel&, COE_TA(2, A, &a)))
        : _obj(0), _tdn(Ctti<typename List2<COE_T(2, A)>::type, ArgListI>::meta())
        { _fun.g2 = GFun2(fun); }

    template<COE_T(3, class A)>
    HandlerX (void (*fun)(Kernel&, COE_TA(3, A, &a)))
        : _obj(0), _tdn(Ctti<typename List3<COE_T(3, A)>::type, ArgListI>::meta())
        { _fun.g3 = GFun3(fun); }

    template<COE_T(4, class A)>
    HandlerX (void (*fun)(Kernel&, COE_TA(4, A, &a)))
        : _obj(0), _tdn(Ctti<typename List4<COE_T(4, A)>::type, ArgListI>::meta())
        { _fun.g4 = GFun4(fun); }

    template<COE_T(5, class A)>
    HandlerX (void (*fun)(Kernel&, COE_TA(5, A, &a)))
        : _obj(0), _tdn(Ctti<typename List5<COE_T(5, A)>::type, ArgListI>::meta())
        { _fun.g5 = GFun5(fun); }

    template<COE_T(6, class A)>
    HandlerX (void (*fun)(Kernel&, COE_TA(6, A, &a)))
        : _obj(0), _tdn(Ctti<typename List6<COE_T(6, A)>::type, ArgListI>::meta())
        { _fun.g6 = GFun6(fun); }

    template<COE_T(7, class A)>
    HandlerX (void (*fun)(Kernel&, COE_TA(7, A, &a)))
        : _obj(0), _tdn(Ctti<typename List7<COE_T(7, A)>::type, ArgListI>::meta())
        { _fun.g7 = GFun7(fun); }

    template<COE_T(8, class A)>
    HandlerX (void (*fun)(Kernel&, COE_TA(8, A, &a)))
        : _obj(0), _tdn(Ctti<typename List8<COE_T(8, A)>::type, ArgListI>::meta())
        { _fun.g8 = GFun8(fun); }

    template<COE_T(9, class A)>
    HandlerX (void (*fun)(Kernel&, COE_TA(9, A, &a)))
        : _obj(0), _tdn(Ctti<typename List9<COE_T(9, A)>::type, ArgListI>::meta())
        { _fun.g9 = GFun9(fun); }

private:
    union {
        MFun0 m0; GFun0 g0;
        MFun1 m1; GFun1 g1;
        MFun2 m2; GFun2 g2;
        MFun3 m3; GFun3 g3;
        MFun4 m4; GFun4 g4;
        MFun5 m5; GFun5 g5;
        MFun6 m6; GFun6 g6;
        MFun7 m7; GFun7 g7;
        MFun8 m8; GFun8 g8;
        MFun9 m9; GFun9 g9;
    }                       _fun;
    _Obj*                   _obj;
    const Meta<ArgListI>*   _tdn;
};

// ---------------------------------------------------------------------------
// Handler_<ARGS, N>

template<class ARGS, int N = Length<ARGS>::value> class Handler_;

// ------------------------------------

template<class ARGS>
class Handler_<ARGS, 1> {
    HandlerX    _handler;
public:
    operator HandlerX () const { return _handler; }
    typedef typename Nth<0, ARGS>::type A1;
    Handler_ () {}
    Handler_ (void (*fun)(Kernel&, COE_TA(1, A, &a))) : _handler(fun) {}
    template<class Obj>
    Handler_ (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(1, A, &a))) : _handler(obj, fun) {}
};

template<class ARGS>
class Handler_<ARGS, 2> {
    HandlerX    _handler;
public:
    operator HandlerX () const { return _handler; }
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    Handler_ () {}
    Handler_ (void (*fun)(Kernel&, COE_TA(2, A, &a))) : _handler(fun) {}
    template<class Obj>
    Handler_ (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(2, A, &a))) : _handler(obj, fun) {}
};

template<class ARGS>
class Handler_<ARGS, 3> {
    HandlerX    _handler;
public:
    operator HandlerX () const { return _handler; }
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    Handler_ () {}
    Handler_ (void (*fun)(Kernel&, COE_TA(3, A, &a))) : _handler(fun) {}
    template<class Obj>
    Handler_ (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(3, A, &a))) : _handler(obj, fun) {}
};

template<class ARGS>
class Handler_<ARGS, 4> {
    HandlerX    _handler;
public:
    operator HandlerX () const { return _handler; }
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    Handler_ () {}
    Handler_ (void (*fun)(Kernel&, COE_TA(4, A, &a))) : _handler(fun) {}
    template<class Obj>
    Handler_ (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(4, A, &a))) : _handler(obj, fun) {}
};

template<class ARGS>
class Handler_<ARGS, 5> {
    HandlerX    _handler;
public:
    operator HandlerX () const { return _handler; }
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    Handler_ () {}
    Handler_ (void (*fun)(Kernel&, COE_TA(5, A, &a))) : _handler(fun) {}
    template<class Obj>
    Handler_ (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(5, A, &a))) : _handler(obj, fun) {}
};

template<class ARGS>
class Handler_<ARGS, 6> {
    HandlerX    _handler;
public:
    operator HandlerX () const { return _handler; }
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    Handler_ () {}
    Handler_ (void (*fun)(Kernel&, COE_TA(6, A, &a))) : _handler(fun) {}
    template<class Obj>
    Handler_ (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(6, A, &a))) : _handler(obj, fun) {}
};

template<class ARGS>
class Handler_<ARGS, 7> {
    HandlerX    _handler;
public:
    operator HandlerX () const { return _handler; }
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    typedef typename Nth<6, ARGS>::type A7;
    Handler_ () {}
    Handler_ (void (*fun)(Kernel&, COE_TA(7, A, &a))) : _handler(fun) {}
    template<class Obj>
    Handler_ (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(7, A, &a))) : _handler(obj, fun) {}
};

template<class ARGS>
class Handler_<ARGS, 8> {
    HandlerX    _handler;
public:
    operator HandlerX () const { return _handler; }
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    typedef typename Nth<6, ARGS>::type A7;
    typedef typename Nth<7, ARGS>::type A8;
    Handler_ () {}
    Handler_ (void (*fun)(Kernel&, COE_TA(8, A, &a))) : _handler(fun) {}
    template<class Obj>
    Handler_ (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(8, A, &a))) : _handler(obj, fun) {}
};

template<class ARGS>
class Handler_<ARGS, 9> {
    HandlerX    _handler;
public:
    operator HandlerX () const { return _handler; }
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    typedef typename Nth<6, ARGS>::type A7;
    typedef typename Nth<7, ARGS>::type A8;
    typedef typename Nth<8, ARGS>::type A9;
    Handler_ () {}
    Handler_ (void (*fun)(Kernel&, COE_TA(9, A, &a))) : _handler(fun) {}
    template<class Obj>
    Handler_ (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(9, A, &a))) : _handler(obj, fun) {}
};

// ---------------------------------------------------------------------------
// Handler0

class Handler0 {
public:
    Handler0 () : _obj(0) { _fun.g0 = 0; }

    typedef HandlerX::_Obj _Obj;    // nowhere defined

    operator _SafeBool<Handler0, _Obj*>::Type () const
        {
            return _obj ? (_fun.m0 ? &Handler0::_obj : 0)
                        : (_fun.g0 ? &Handler0::_obj : 0);
        }

    bool operator== (const Handler0& rhs) const
        {
            return  _obj == rhs._obj
                && (_obj ? _fun.m0 == rhs._fun.m0
                         : _fun.g0 == rhs._fun.g0);
        }
    bool operator!= (const Handler0& rhs) const { return ! operator==(rhs); }

    operator HandlerX () const
        {
            return _obj ? HandlerX(*_obj, _fun.m0) : HandlerX(_fun.g0);
        }

    void execute (Kernel& kernel) const;

    // --------------------------------

    typedef void (_Obj::*MFun0) (Kernel&);
    typedef void       (*GFun0) (Kernel&);

    template<class Obj>
    Handler0 (Obj& obj, void (Obj::*fun)(Kernel&))
        : _obj((_Obj*)&obj)
        { _fun.m0 = MFun0(fun); }

    Handler0 (void (*fun)(Kernel&))
        : _obj(0)
        { _fun.g0 = GFun0(fun); }

private:
    union {
        MFun0 m0;
        GFun0 g0;
    }       _fun;
    _Obj*   _obj;
};

// ===========================================================================

} ///// namespace coe

#endif

