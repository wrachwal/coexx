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

namespace coe { /////

// ---------------------------------------------------------------------------

class Kernel;

// ===========================================================================
// _TypeD

template<class> class _TypeI;

struct _TypeD {
    const std::type_info*   info;
    const _TypeD*           next;
    size_t                  pos;

    static const _TypeD* registry () { return _register(0); }

private:
    template<class> friend class _TypeI;
    template<class T>
    explicit _TypeD (T*) : info(&typeid(T)), next(0) { _register(this); }
    static const _TypeD* _register (_TypeD* type);
};

// ---------------------------------------------------------------------------
// _TypeI<T>

template<class T>
class _TypeI {
public:
    const _TypeD* data () const { return &_data; }

private:
    static const _TypeD _data;
};

template<class T>
const _TypeD _TypeI<T>::_data((T*)0);

// ===========================================================================
// _TypeDN

template<class>                                                         class _TypeI1;
template<class, class>                                                  class _TypeI2;
template<class, class, class>                                           class _TypeI3;
template<class, class, class, class>                                    class _TypeI4;
template<class, class, class, class, class>                             class _TypeI5;
template<class, class, class, class, class, class>                      class _TypeI6;
template<class, class, class, class, class, class, class>               class _TypeI7;
template<class, class, class, class, class, class, class, class>        class _TypeI8;
template<class, class, class, class, class, class, class, class, class> class _TypeI9;

struct _TypeDN {
    const _TypeD**  info;   // NULL-terminated array
    size_t          len;
    const _TypeDN*  next;
    size_t          pos;

    static const _TypeDN* registry () { return _register(0); }

private:
    template<class>                                                  friend class _TypeI1;
    template<class, class>                                           friend class _TypeI2;
    template<class, class, class>                                    friend class _TypeI3;
    template<class, class, class, class>                             friend class _TypeI4;
    template<class, class, class, class, class>                      friend class _TypeI5;
    template<class, class, class, class, class, class>               friend class _TypeI6;
    template<class, class, class, class, class, class, class>        friend class _TypeI7;
    template<class, class, class, class, class, class, class, class> friend class _TypeI8;
    template<class, class, class, class, class, class, class, class, class>
                                                                     friend class _TypeI9;

    template<class T1> explicit
    _TypeDN (T1*) : len(1), next(0), pos(0)
        {
            static const _TypeD* _tab[] = {
                _TypeI<T1>().data(),
                0
            };
            info = _tab;
            _register(this);
        }

    template<class T1, class T2>
    _TypeDN (T1*, T2*) : len(2), next(0), pos(0)
        {
            static const _TypeD* _tab[] = {
                _TypeI<T1>().data(),
                _TypeI<T2>().data(),
                0
            };
            info = _tab;
            _register(this);
        }

    template<class T1, class T2, class T3>
    _TypeDN (T1*, T2*, T3*) : len(3), next(0), pos(0)
        {
            static const _TypeD* _tab[] = {
                _TypeI<T1>().data(),
                _TypeI<T2>().data(),
                _TypeI<T3>().data(),
                0
            };
            info = _tab;
            _register(this);
        }

    template<class T1, class T2, class T3, class T4>
    _TypeDN (T1*, T2*, T3*, T4*) : len(4), next(0), pos(0)
        {
            static const _TypeD* _tab[] = {
                _TypeI<T1>().data(),
                _TypeI<T2>().data(),
                _TypeI<T3>().data(),
                _TypeI<T4>().data(),
                0
            };
            info = _tab;
            _register(this);
        }

    template<class T1, class T2, class T3, class T4, class T5>
    _TypeDN (T1*, T2*, T3*, T4*, T5*) : len(5), next(0), pos(0)
        {
            static const _TypeD* _tab[] = {
                _TypeI<T1>().data(),
                _TypeI<T2>().data(),
                _TypeI<T3>().data(),
                _TypeI<T4>().data(),
                _TypeI<T5>().data(),
                0
            };
            info = _tab;
            _register(this);
        }

    template<class T1, class T2, class T3, class T4, class T5,
             class T6>
    _TypeDN (T1*, T2*, T3*, T4*, T5*, T6*) : len(6), next(0), pos(0)
        {
            static const _TypeD* _tab[] = {
                _TypeI<T1>().data(),
                _TypeI<T2>().data(),
                _TypeI<T3>().data(),
                _TypeI<T4>().data(),
                _TypeI<T5>().data(),
                _TypeI<T6>().data(),
                0
            };
            info = _tab;
            _register(this);
        }

    template<class T1, class T2, class T3, class T4, class T5,
             class T6, class T7>
    _TypeDN (T1*, T2*, T3*, T4*, T5*, T6*, T7*) : len(7), next(0), pos(0)
        {
            static const _TypeD* _tab[] = {
                _TypeI<T1>().data(),
                _TypeI<T2>().data(),
                _TypeI<T3>().data(),
                _TypeI<T4>().data(),
                _TypeI<T5>().data(),
                _TypeI<T6>().data(),
                _TypeI<T7>().data(),
                0
            };
            info = _tab;
            _register(this);
        }

    template<class T1, class T2, class T3, class T4, class T5,
             class T6, class T7, class T8>
    _TypeDN (T1*, T2*, T3*, T4*, T5*, T6*, T7*, T8*) : len(8), next(0), pos(0)
        {
            static const _TypeD* _tab[] = {
                _TypeI<T1>().data(),
                _TypeI<T2>().data(),
                _TypeI<T3>().data(),
                _TypeI<T4>().data(),
                _TypeI<T5>().data(),
                _TypeI<T6>().data(),
                _TypeI<T7>().data(),
                _TypeI<T8>().data(),
                0
            };
            info = _tab;
            _register(this);
        }

    template<class T1, class T2, class T3, class T4, class T5,
             class T6, class T7, class T8, class T9>
    _TypeDN (T1*, T2*, T3*, T4*, T5*, T6*, T7*, T8*, T9*) : len(9), next(0), pos(0)
        {
            static const _TypeD* _tab[] = {
                _TypeI<T1>().data(),
                _TypeI<T2>().data(),
                _TypeI<T3>().data(),
                _TypeI<T4>().data(),
                _TypeI<T5>().data(),
                _TypeI<T6>().data(),
                _TypeI<T7>().data(),
                _TypeI<T8>().data(),
                _TypeI<T9>().data(),
                0
            };
            info = _tab;
            _register(this);
        }

    static const _TypeDN* _register (_TypeDN* type);
};

// ------------------------------------

bool syntax_check (const _TypeDN* hT, const _TypeDN* xT, const _TypeDN* aT);

// ---------------------------------------------------------------------------
// _TypeI1<T1>

template<class T1>
class _TypeI1 {
public:
    const _TypeDN* data () const { return &_data; }
private:
    static const _TypeDN    _data;
};

template<class T1>
const _TypeDN _TypeI1<T1>::_data((T1*)0);

// ------------------------------------
// _TypeI2<T1, T2>

template<class T1, class T2>
class _TypeI2 {
public:
    const _TypeDN* data () const { return &_data; }
private:
    static const _TypeDN    _data;
};

template<class T1, class T2>
const _TypeDN _TypeI2<T1, T2>::_data((T1*)0, (T2*)0);

// ------------------------------------
// _TypeI3<T1, T2, T3>

template<class T1, class T2, class T3>
class _TypeI3 {
public:
    const _TypeDN* data () const { return &_data; }
private:
    static const _TypeDN    _data;
};

template<class T1, class T2, class T3>
const _TypeDN _TypeI3<T1, T2, T3>::_data((T1*)0, (T2*)0, (T3*)0);

// ------------------------------------
// _TypeI4<T1, T2, T3, T4>

template<class T1, class T2, class T3, class T4>
class _TypeI4 {
public:
    const _TypeDN* data () const { return &_data; }
private:
    static const _TypeDN    _data;
};

template<class T1, class T2, class T3, class T4>
const _TypeDN _TypeI4<T1, T2, T3, T4>::_data((T1*)0, (T2*)0, (T3*)0, (T4*)0);

// ------------------------------------
// _TypeI5<T1, T2, T3, T4, T5>

template<class T1, class T2, class T3, class T4, class T5>
class _TypeI5 {
public:
    const _TypeDN* data () const { return &_data; }
private:
    static const _TypeDN    _data;
};

template<class T1, class T2, class T3, class T4, class T5>
const _TypeDN _TypeI5<T1, T2, T3, T4, T5>::_data((T1*)0, (T2*)0, (T3*)0, (T4*)0, (T5*)0);

// ------------------------------------
// _TypeI6<T1, T2, T3, T4, T5, T6>

template<class T1, class T2, class T3, class T4, class T5,
         class T6>
class _TypeI6 {
public:
    const _TypeDN* data () const { return &_data; }
private:
    static const _TypeDN    _data;
};

template<class T1, class T2, class T3, class T4, class T5,
         class T6>
const _TypeDN _TypeI6<T1, T2, T3, T4, T5, T6>::_data
        ((T1*)0, (T2*)0, (T3*)0, (T4*)0, (T5*)0, (T6*)0);

// ------------------------------------
// _TypeI7<T1, T2, T3, T4, T5, T6, T7>

template<class T1, class T2, class T3, class T4, class T5,
         class T6, class T7>
class _TypeI7 {
public:
    const _TypeDN* data () const { return &_data; }
private:
    static const _TypeDN    _data;
};

template<class T1, class T2, class T3, class T4, class T5,
         class T6, class T7>
const _TypeDN _TypeI7<T1, T2, T3, T4, T5, T6, T7>::_data
        ((T1*)0, (T2*)0, (T3*)0, (T4*)0, (T5*)0, (T6*)0, (T7*)0);

// ------------------------------------
// _TypeI8<T1, T2, T3, T4, T5, T6, T7, T8>

template<class T1, class T2, class T3, class T4, class T5,
         class T6, class T7, class T8>
class _TypeI8 {
public:
    const _TypeDN* data () const { return &_data; }
private:
    static const _TypeDN    _data;
};

template<class T1, class T2, class T3, class T4, class T5,
         class T6, class T7, class T8>
const _TypeDN _TypeI8<T1, T2, T3, T4, T5, T6, T7, T8>::_data
        ((T1*)0, (T2*)0, (T3*)0, (T4*)0, (T5*)0, (T6*)0, (T7*)0, (T8*)0);

// ------------------------------------
// _TypeI9<T1, T2, T3, T4, T5, T6, T7, T8, T9>

template<class T1, class T2, class T3, class T4, class T5,
         class T6, class T7, class T8, class T9>
class _TypeI9 {
public:
    const _TypeDN* data () const { return &_data; }
private:
    static const _TypeDN    _data;
};

template<class T1, class T2, class T3, class T4, class T5,
         class T6, class T7, class T8, class T9>
const _TypeDN _TypeI9<T1, T2, T3, T4, T5, T6, T7, T8, T9>::_data
        ((T1*)0, (T2*)0, (T3*)0, (T4*)0, (T5*)0, (T6*)0, (T7*)0, (T8*)0, (T9*)0);

// ===========================================================================
// ValParam_<ARGS>

class ValParam;
#if 0
    ///XXX -- it was the first attempt
    template<class>         class ValParam1;
    template<class, class>  class ValParam2;
#else
    template<class ARGS, int N = Length<ARGS>::value> class ValParamA;
#endif

template<class ARGS>
class ValParam_ {
public:
    operator ValParam* () { return _ptr; }
    ValParam*    clear () { ValParam* tmp = _ptr; _ptr = 0; return tmp; }

    ValParam_ () : _ptr(0) {}
#if 0
    //XXX -- it was the first attempt
    ValParam_ (ValParam1<typename Nth<0, ARGS>::type>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParam2<typename Nth<0, ARGS>::type,
                         typename Nth<1, ARGS>::type>* ptr) : _ptr(ptr) {}
#else
    ValParam_ (ValParamA<ARGS, 1>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 2>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 3>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 4>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 5>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 6>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 7>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 8>* ptr) : _ptr(ptr) {}
    ValParam_ (ValParamA<ARGS, 9>* ptr) : _ptr(ptr) {}
#endif

private:
    ValParam*   _ptr;
};

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

    const _TypeDN* par_type () const { return _tdn; }

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

    template<class Obj, class A1>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, A1&))
        : _obj((_Obj*)&obj), _tdn(_TypeI1<A1>().data())
        { _fun.m1 = MFun1(fun); }

    template<class Obj, class A1, class A2>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, A1&, A2&))
        : _obj((_Obj*)&obj), _tdn(_TypeI2<A1, A2>().data())
        { _fun.m2 = MFun2(fun); }

    template<class Obj, class A1, class A2, class A3>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, A1&, A2&, A3&))
        : _obj((_Obj*)&obj), _tdn(_TypeI3<A1, A2, A3>().data())
        { _fun.m3 = MFun3(fun); }

    template<class Obj, class A1, class A2, class A3, class A4>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, A1&, A2&, A3&, A4&))
        : _obj((_Obj*)&obj), _tdn(_TypeI4<A1, A2, A3, A4>().data())
        { _fun.m4 = MFun4(fun); }

    template<class Obj, class A1, class A2, class A3, class A4, class A5>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, A1&, A2&, A3&, A4&, A5&))
        : _obj((_Obj*)&obj), _tdn(_TypeI5<A1, A2, A3, A4, A5>().data())
        { _fun.m5 = MFun5(fun); }

    template<class Obj, class A1, class A2, class A3, class A4, class A5,
                        class A6>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, A1&, A2&, A3&, A4&, A5&,
                                                  A6&))
        : _obj((_Obj*)&obj), _tdn(_TypeI6<A1, A2, A3, A4, A5, A6>().data())
        { _fun.m6 = MFun6(fun); }

    template<class Obj, class A1, class A2, class A3, class A4, class A5,
                        class A6, class A7>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, A1&, A2&, A3&, A4&, A5&,
                                                  A6&, A7&))
        : _obj((_Obj*)&obj), _tdn(_TypeI7<A1, A2, A3, A4, A5, A6, A7>().data())
        { _fun.m7 = MFun7(fun); }

    template<class Obj, class A1, class A2, class A3, class A4, class A5,
                        class A6, class A7, class A8>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, A1&, A2&, A3&, A4&, A5&,
                                                  A6&, A7&, A8&))
        : _obj((_Obj*)&obj), _tdn(_TypeI8<A1, A2, A3, A4, A5, A6, A7, A8>().data())
        { _fun.m8 = MFun8(fun); }

    template<class Obj, class A1, class A2, class A3, class A4, class A5,
                        class A6, class A7, class A8, class A9>
    HandlerX (Obj& obj, void (Obj::*fun)(Kernel&, A1&, A2&, A3&, A4&, A5&,
                                                  A6&, A7&, A8&, A9&))
        : _obj((_Obj*)&obj), _tdn(_TypeI9<A1, A2, A3, A4, A5, A6, A7, A8, A9>().data())
        { _fun.m9 = MFun9(fun); }

    // --------------------------------

    HandlerX (void (*fun)(Kernel&))
        : _obj(0), _tdn(0)
        { _fun.g0 = GFun0(fun); }

    template<class A1>
    HandlerX (void (*fun)(Kernel&, A1&))
        : _obj(0), _tdn(_TypeI1<A1>().data())
        { _fun.g1 = GFun1(fun); }

    template<class A1, class A2>
    HandlerX (void (*fun)(Kernel&, A1&, A2&))
        : _obj(0), _tdn(_TypeI2<A1, A2>().data())
        { _fun.g2 = GFun2(fun); }

    template<class A1, class A2, class A3>
    HandlerX (void (*fun)(Kernel&, A1&, A2&, A3&))
        : _obj(0), _tdn(_TypeI3<A1, A2, A3>().data())
        { _fun.g3 = GFun3(fun); }

    template<class A1, class A2, class A3, class A4>
    HandlerX (void (*fun)(Kernel&, A1&, A2&, A3&, A4&))
        : _obj(0), _tdn(_TypeI4<A1, A2, A3, A4>().data())
        { _fun.g4 = GFun4(fun); }

    template<class A1, class A2, class A3, class A4, class A5>
    HandlerX (void (*fun)(Kernel&, A1&, A2&, A3&, A4&, A5&))
        : _obj(0), _tdn(_TypeI5<A1, A2, A3, A4, A5>().data())
        { _fun.g5 = GFun5(fun); }

    template<class A1, class A2, class A3, class A4, class A5,
             class A6>
    HandlerX (void (*fun)(Kernel&, A1&, A2&, A3&, A4&, A5&, A6&))
        : _obj(0), _tdn(_TypeI6<A1, A2, A3, A4, A5, A6>().data())
        { _fun.g6 = GFun6(fun); }

    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7>
    HandlerX (void (*fun)(Kernel&, A1&, A2&, A3&, A4&, A5&, A6&, A7&))
        : _obj(0), _tdn(_TypeI7<A1, A2, A3, A4, A5, A6, A7>().data())
        { _fun.g7 = GFun7(fun); }

    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7, class A8>
    HandlerX (void (*fun)(Kernel&, A1&, A2&, A3&, A4&, A5&, A6&, A7&, A8&))
        : _obj(0), _tdn(_TypeI8<A1, A2, A3, A4, A5, A6, A7, A8>().data())
        { _fun.g8 = GFun8(fun); }

    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7, class A8, class A9>
    HandlerX (void (*fun)(Kernel&, A1&, A2&, A3&, A4&, A5&, A6&, A7&, A8&, A9&))
        : _obj(0), _tdn(_TypeI9<A1, A2, A3, A4, A5, A6, A7, A8, A9>().data())
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
    }               _fun;
    _Obj*           _obj;
    const _TypeDN*  _tdn;
};

// ------------------------------------

template<class ARGS>
class Handler_ {
public:
    operator HandlerX () const { return _handler; }

    Handler_ () {}

    template<class Obj>
    Handler_ (Obj& obj, void (Obj::*fun)(Kernel&, typename Nth<0, ARGS>::type&))
        : _handler(obj, fun) {}
    template<class Obj>
    Handler_ (Obj& obj, void (Obj::*fun)(Kernel&, typename Nth<0, ARGS>::type&,
                                                  typename Nth<1, ARGS>::type&))
        : _handler(obj, fun) {}

private:
    HandlerX    _handler;
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

