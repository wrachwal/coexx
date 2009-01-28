// coe-kernel--imp.h

/*****************************************************************************
Copyright (c) 2008, 2009 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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

#if !defined(__COE_KERNEL_H) || defined(__COE_KERNEL__IMP_H)
#error "!!! coe-kernel--imp.h is a private header."
#endif

#ifndef __COE_KERNEL__IMP_H
#define __COE_KERNEL__IMP_H

namespace coe { /////

// ---------------------------------------------------------------------------

class EventArg;
    class ValParam;
    //  class ValParam1<A1> ... ValParam5<A1 .. A5>
    class RefParam;
    //  class RefParam1     ... RefParam5
class StateCmd;
    //  class MFunCmd0 ... MFunCmd5
    //  class GFunCmd0 ... GFunCmd5

// ===========================================================================
// ArgTV

struct ArgTV {
    void set (const std::type_info* ti, void* pv)
        {
            _ti = ti;
            _pv = pv;
        }
    void set (const std::type_info* ti)
        {
            _ti = ti;
        }
    const std::type_info* _ti;
    void*                 _pv;
};

// ===========================================================================
// EventArg

class EventArg {
public:
    //
    // static operator new/delete...
    //
    virtual ~EventArg ();
    virtual const ArgTV* arg_list (int& len) const = 0;
};

// ------------------------------------
// ValParam

class ValParam : public EventArg {
public:
    virtual ValParam* clone () const = 0;
};

// ------------------------------------
// RefParam

class RefParam : public EventArg {};

// ------------------------------------
// EventArg_N<Base, N>

template<class Base, int N>
class EventArg_N : public Base {
protected:
    ArgTV   _arg[N];
    /*final*/ const ArgTV* arg_list (int& len) const;
};

template<class Base, int N>
const ArgTV* EventArg_N<Base, N>::arg_list (int& len) const
{
    len = N;
    return &_arg[0];
}

// ===========================================================================
// StateCmd

class StateCmd {
public:
    //
    // static operator new/delete...
    //
    virtual ~StateCmd ();
    bool syntax (const ArgTV* xA, int xN, EventArg* arg, bool report) const;
    bool execute (EvCtx& ctx, const ArgTV* xA, int xN, EventArg* arg);
    virtual ArgTV* arg_list (int& num) = 0;
private:
    virtual void _execute (EvCtx& ctx) const = 0;
};

// ------------------------------------
// StateCmd_N<N>

template<int N>
class StateCmd_N : public StateCmd {
protected:
    ArgTV   _arg[N];
    /*final*/ ArgTV* arg_list (int& len);
};

template<int N>
ArgTV* StateCmd_N<N>::arg_list (int& len)
{
    len = N;
    return &_arg[0];
}

// ===========================================================================
// TimeSpec
// ===========================================================================

inline bool operator== (const TimeSpec& lhs, const TimeSpec& rhs)
    { return lhs.tv_sec == rhs.tv_sec && lhs.tv_nsec == rhs.tv_nsec; }

inline bool operator!= (const TimeSpec& lhs, const TimeSpec& rhs)
    { return lhs.tv_sec != rhs.tv_sec || lhs.tv_nsec != rhs.tv_nsec; }

inline bool operator<  (const TimeSpec& lhs, const TimeSpec& rhs)
    { return lhs.compare(rhs) <  0; }

inline bool operator<= (const TimeSpec& lhs, const TimeSpec& rhs)
    { return lhs.compare(rhs) <= 0; }

inline bool operator>  (const TimeSpec& lhs, const TimeSpec& rhs)
    { return lhs.compare(rhs) >  0; }

inline bool operator>= (const TimeSpec& lhs, const TimeSpec& rhs)
    { return lhs.compare(rhs) >= 0; }

inline TimeSpec operator+ (const TimeSpec& lhs, const TimeSpec& rhs)
    {
        TimeSpec result(lhs);
        result += rhs;
        return result;
    }
inline TimeSpec operator- (const TimeSpec& lhs, const TimeSpec& rhs)
    {
        TimeSpec result(lhs);
        result -= rhs;
        return result;
    }

// ===========================================================================
// ValParam1<A1> ... ValParam5<A1 .. A5>
// ===========================================================================

template<class A1>
class ValParam1 : public EventArg_N<ValParam, 1> {
public:
    ValParam1 (const A1& a1)
        : _a1(a1)
        {
            _arg[0].set(&typeid(A1), &_a1);
        }
    ValParam* clone () const
        { return new ValParam1(_a1); }
private:
    A1  _a1;
};

// ------------------------------------

template<class A1, class A2>
class ValParam2 : public EventArg_N<ValParam, 2> {
public:
    ValParam2 (const A1& a1, const A2& a2)
        : _a1(a1), _a2(a2)
        {
            _arg[0].set(&typeid(A1), &_a1);
            _arg[1].set(&typeid(A2), &_a2);
        }
    ValParam* clone () const
        { return new ValParam2(_a1, _a2); }
private:
    A1  _a1;
    A2  _a2;
};

// ------------------------------------

template<class A1, class A2, class A3>
class ValParam3 : public EventArg_N<ValParam, 3> {
public:
    ValParam3 (const A1& a1, const A2& a2, const A3& a3)
        : _a1(a1), _a2(a2), _a3(a3)
        {
            _arg[0].set(&typeid(A1), &_a1);
            _arg[1].set(&typeid(A2), &_a2);
            _arg[2].set(&typeid(A3), &_a3);
        }
    ValParam* clone () const
        { return new ValParam3(_a1, _a2, _a3); }
private:
    A1  _a1;
    A2  _a2;
    A3  _a3;
};

// ------------------------------------

template<class A1, class A2, class A3, class A4>
class ValParam4 : public EventArg_N<ValParam, 4> {
public:
    ValParam4 (const A1& a1, const A2& a2, const A3& a3, const A4& a4)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4)
        {
            _arg[0].set(&typeid(A1), &_a1);
            _arg[1].set(&typeid(A2), &_a2);
            _arg[2].set(&typeid(A3), &_a3);
            _arg[3].set(&typeid(A4), &_a4);
        }
    ValParam* clone () const
        { return new ValParam4(_a1, _a2, _a3, _a4); }
private:
    A1  _a1;
    A2  _a2;
    A3  _a3;
    A4  _a4;
};

// ------------------------------------

template<class A1, class A2, class A3, class A4, class A5>
class ValParam5 : public EventArg_N<ValParam, 5> {
public:
    ValParam5 (const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5)
        {
            _arg[0].set(&typeid(A1), &_a1);
            _arg[1].set(&typeid(A2), &_a2);
            _arg[2].set(&typeid(A3), &_a3);
            _arg[3].set(&typeid(A4), &_a4);
            _arg[4].set(&typeid(A5), &_a5);
        }
    ValParam* clone () const
        { return new ValParam5(_a1, _a2, _a3, _a4, _a5); }
private:
    A1  _a1;
    A2  _a2;
    A3  _a3;
    A4  _a4;
    A4  _a5;
};

// ===========================================================================
// RefParam1 ... RefParam5
// ===========================================================================

class RefParam1 : public EventArg_N<RefParam, 1> {
public:
    template<class A1>
    RefParam1 (A1& a1)
        {
            _arg[0].set(&typeid(A1), &a1);
        }
};

// ------------------------------------

class RefParam2 : public EventArg_N<RefParam, 2> {
public:
    template<class A1, class A2>
    RefParam2 (A1& a1, A2& a2)
        {
            _arg[0].set(&typeid(A1), &a1);
            _arg[1].set(&typeid(A2), &a2);
        }
};

// ------------------------------------

class RefParam3 : public EventArg_N<RefParam, 3> {
public:
    template<class A1, class A2, class A3>
    RefParam3 (A1& a1, A2& a2, A3& a3)
        {
            _arg[0].set(&typeid(A1), &a1);
            _arg[1].set(&typeid(A2), &a2);
            _arg[2].set(&typeid(A3), &a3);
        }
};

// ------------------------------------

class RefParam4 : public EventArg_N<RefParam, 4> {
public:
    template<class A1, class A2, class A3, class A4>
    RefParam4 (A1& a1, A2& a2, A3& a3, A4& a4)
        {
            _arg[0].set(&typeid(A1), &a1);
            _arg[1].set(&typeid(A2), &a2);
            _arg[2].set(&typeid(A3), &a3);
            _arg[3].set(&typeid(A4), &a4);
        }
};

// ------------------------------------

class RefParam5 : public EventArg_N<RefParam, 5> {
public:
    template<class A1, class A2, class A3, class A4, class A5>
    RefParam5 (A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
        {
            _arg[0].set(&typeid(A1), &a1);
            _arg[1].set(&typeid(A2), &a2);
            _arg[2].set(&typeid(A3), &a3);
            _arg[3].set(&typeid(A4), &a4);
            _arg[4].set(&typeid(A5), &a5);
        }
};

// ===========================================================================
// MFunCmd0 ... MFunCmd5
// ===========================================================================

class MFunCmd0 : public StateCmd {
public:
    class _Obj;
    typedef void (_Obj::*_MemFun)(EvCtx&);
    template<class Obj>
    MFunCmd0 (Obj& obj, void (Obj::*memfun)(EvCtx&))
        : _obj((_Obj*)&obj), _memfun((_MemFun)memfun)
        {}
    template<class Heap, class Obj>
    MFunCmd0 (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&))
        : _obj((_Obj*)&obj), _memfun((_MemFun)memfun)
        {}
private:
    /*final*/ ArgTV* arg_list (int& len);
    void _execute (EvCtx& ctx) const;
    _Obj* _obj;
    void (_Obj::*_memfun)(EvCtx&);
};

// ------------------------------------

class MFunCmd1 : public StateCmd_N<1> {
public:
    class _Obj;
    class _Arg;
    typedef void (_Obj::*_MemFun)(EvCtx&, _Arg&);
    template<class Obj, class A1>
    MFunCmd1 (Obj& obj, void (Obj::*memfun)(EvCtx&, A1&))
        : _obj((_Obj*)&obj), _memfun((_MemFun)memfun)
        { _initialize<A1>(); }
    template<class Heap, class Obj, class A1>
    MFunCmd1 (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, A1&))
        : _obj((_Obj*)&obj), _memfun((_MemFun)memfun)
        { _initialize<A1>(); }
private:
    template<class A1>
    void _initialize ()
        {
            _arg[0].set(&typeid(A1));
        }
    void _execute (EvCtx& ctx) const;
    _Obj* _obj;
    void (_Obj::*_memfun)(EvCtx&, _Arg&);
};

// ------------------------------------

class MFunCmd2 : public StateCmd_N<2> {
public:
    class _Obj;
    class _Arg;
    typedef void (_Obj::*_MemFun)(EvCtx&, _Arg&, _Arg&);
    template<class Obj, class A1, class A2>
    MFunCmd2 (Obj& obj, void (Obj::*memfun)(EvCtx&, A1&, A2&))
        : _obj((_Obj*)&obj), _memfun((_MemFun)memfun)
        { _initialize<A1, A2>(); }
    template<class Heap, class Obj, class A1, class A2>
    MFunCmd2 (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, A1&, A2&))
        : _obj((_Obj*)&obj), _memfun((_MemFun)memfun)
        { _initialize<A1, A2>(); }
private:
    template<class A1, class A2>
    void _initialize ()
        {
            _arg[0].set(&typeid(A1));
            _arg[1].set(&typeid(A2));
        }
    void _execute (EvCtx& ctx) const;
    _Obj* _obj;
    void (_Obj::*_memfun)(EvCtx&, _Arg&, _Arg&);
};

// ------------------------------------

class MFunCmd3 : public StateCmd_N<3> {
public:
    class _Obj;
    class _Arg;
    typedef void (_Obj::*_MemFun)(EvCtx&, _Arg&, _Arg&, _Arg&);
    template<class Obj, class A1, class A2, class A3>
    MFunCmd3 (Obj& obj, void (Obj::*memfun)(EvCtx&, A1&, A2&, A3&))
        : _obj((_Obj*)&obj), _memfun((_MemFun)memfun)
        { _initialize<A1, A2, A3>(); }
    template<class Heap, class Obj, class A1, class A2, class A3>
    MFunCmd3 (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, A1&, A2&, A3&))
        : _obj((_Obj*)&obj), _memfun((_MemFun)memfun)
        { _initialize<A1, A2, A3>(); }
private:
    template<class A1, class A2, class A3>
    void _initialize ()
        {
            _arg[0].set(&typeid(A1));
            _arg[1].set(&typeid(A2));
            _arg[2].set(&typeid(A3));
        }
    void _execute (EvCtx& ctx) const;
    _Obj* _obj;
    void (_Obj::*_memfun)(EvCtx&, _Arg&, _Arg&, _Arg&);
};

// ------------------------------------

class MFunCmd4 : public StateCmd_N<4> {
public:
    class _Obj;
    class _Arg;
    typedef void (_Obj::*_MemFun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&);
    template<class Obj, class A1, class A2, class A3, class A4>
    MFunCmd4 (Obj& obj, void (Obj::*memfun)(EvCtx&, A1&, A2&, A3&, A4&))
        : _obj((_Obj*)&obj), _memfun((_MemFun)memfun)
        { _initialize<A1, A2, A3, A4>(); }
    template<class Heap, class Obj, class A1, class A2, class A3, class A4>
    MFunCmd4 (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, A1&, A2&, A3&, A4&))
        : _obj((_Obj*)&obj), _memfun((_MemFun)memfun)
        { _initialize<A1, A2, A3, A4>(); }
private:
    template<class A1, class A2, class A3, class A4>
    void _initialize ()
        {
            _arg[0].set(&typeid(A1));
            _arg[1].set(&typeid(A2));
            _arg[2].set(&typeid(A3));
            _arg[3].set(&typeid(A4));
        }
    void _execute (EvCtx& ctx) const;
    _Obj* _obj;
    void (_Obj::*_memfun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&);
};

// ------------------------------------

class MFunCmd5 : public StateCmd_N<5> {
public:
    class _Obj;
    class _Arg;
    typedef void (_Obj::*_MemFun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&, _Arg&);
    template<class Obj, class A1, class A2, class A3, class A4, class A5>
    MFunCmd5 (Obj& obj, void (Obj::*memfun)(EvCtx&, A1&, A2&, A3&, A4&, A5&))
        : _obj((_Obj*)&obj), _memfun((_MemFun)memfun)
        { _initialize<A1, A2, A3, A4, A5>(); }
    template<class Heap, class Obj, class A1, class A2, class A3, class A4, class A5>
    MFunCmd5 (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, A1&, A2&, A3&, A4&, A5&))
        : _obj((_Obj*)&obj), _memfun((_MemFun)memfun)
        { _initialize<A1, A2, A3, A4, A5>(); }
private:
    template<class A1, class A2, class A3, class A4, class A5>
    void _initialize ()
        {
            _arg[0].set(&typeid(A1));
            _arg[1].set(&typeid(A2));
            _arg[2].set(&typeid(A3));
            _arg[3].set(&typeid(A4));
            _arg[4].set(&typeid(A5));
        }
    void _execute (EvCtx& ctx) const;
    _Obj* _obj;
    void (_Obj::*_memfun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&, _Arg&);
};

// ===========================================================================
// GFunCmd0 .. GFunCmd5
// ===========================================================================

class GFunCmd0 : public StateCmd {
public:
    typedef void (*_Fun)(EvCtx&);
    GFunCmd0 (void (*fun)(EvCtx&))
        : _fun((_Fun)fun) {}
    template<class Heap>
    GFunCmd0 (void (*fun)(TEvCtx<Heap>&))
        : _fun((_Fun)fun) {}
private:
    /*final*/ ArgTV* arg_list (int& len);
    void _execute (EvCtx& ctx) const;
    void (*_fun)(EvCtx&);
};

// ------------------------------------

class GFunCmd1 : public StateCmd_N<1> {
public:
    class _Arg;
    typedef void (*_Fun)(EvCtx&, _Arg&);
    template<class A1>
    GFunCmd1 (void (*fun)(EvCtx&, A1&))
        : _fun((_Fun)fun)
        { _initialize<A1>(); }
    template<class Heap, class A1>
    GFunCmd1 (void (*fun)(TEvCtx<Heap>&, A1&))
        : _fun((_Fun)fun)
        { _initialize<A1>(); }
private:
    template<class A1>
    void _initialize ()
        {
            _arg[0].set(&typeid(A1));
        }
    void _execute (EvCtx& ctx) const;
    void (*_fun)(EvCtx&, _Arg&);
};

// ------------------------------------

class GFunCmd2 : public StateCmd_N<2> {
public:
    class _Arg;
    typedef void (*_Fun)(EvCtx&, _Arg&, _Arg&);
    template<class A1, class A2>
    GFunCmd2 (void (*fun)(EvCtx&, A1&, A2&))
        : _fun((_Fun)fun)
        { _initialize<A1, A2>(); }
    template<class Heap, class A1, class A2>
    GFunCmd2 (void (*fun)(TEvCtx<Heap>&, A1&, A2&))
        : _fun((_Fun)fun)
        { _initialize<A1, A2>(); }
private:
    template<class A1, class A2>
    void _initialize ()
        {
            _arg[0].set(&typeid(A1));
            _arg[1].set(&typeid(A2));
        }
    void _execute (EvCtx& ctx) const;
    void (*_fun)(EvCtx&, _Arg&, _Arg&);
};

// ------------------------------------

class GFunCmd3 : public StateCmd_N<3> {
public:
    class _Arg;
    typedef void (*_Fun)(EvCtx&, _Arg&, _Arg&, _Arg&);
    template<class A1, class A2, class A3>
    GFunCmd3 (void (*fun)(EvCtx&, A1&, A2&, A3&))
        : _fun((_Fun)fun)
        { _initialize<A1, A2, A3>(); }
    template<class Heap, class A1, class A2, class A3>
    GFunCmd3 (void (*fun)(TEvCtx<Heap>&, A1&, A2&, A3&))
        : _fun((_Fun)fun)
        { _initialize<A1, A2, A3>(); }
private:
    template<class A1, class A2, class A3>
    void _initialize ()
        {
            _arg[0].set(&typeid(A1));
            _arg[1].set(&typeid(A2));
            _arg[2].set(&typeid(A3));
        }
    void _execute (EvCtx& ctx) const;
    void (*_fun)(EvCtx&, _Arg&, _Arg&, _Arg&);
};

// ------------------------------------

class GFunCmd4 : public StateCmd_N<4> {
public:
    class _Arg;
    typedef void (*_Fun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&);
    template<class A1, class A2, class A3, class A4>
    GFunCmd4 (void (*fun)(EvCtx&, A1&, A2&, A3&, A4&))
        : _fun((_Fun)fun)
        { _initialize<A1, A2, A3, A4>(); }
    template<class Heap, class A1, class A2, class A3, class A4>
    GFunCmd4 (void (*fun)(TEvCtx<Heap>&, A1&, A2&, A3&, A4&))
        : _fun((_Fun)fun)
        { _initialize<A1, A2, A3, A4>(); }
private:
    template<class A1, class A2, class A3, class A4>
    void _initialize ()
        {
            _arg[0].set(&typeid(A1));
            _arg[1].set(&typeid(A2));
            _arg[2].set(&typeid(A3));
            _arg[3].set(&typeid(A4));
        }
    void _execute (EvCtx& ctx) const;
    void (*_fun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&);
};

// ------------------------------------

class GFunCmd5 : public StateCmd_N<5> {
public:
    class _Arg;
    typedef void (*_Fun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&, _Arg&);
    template<class A1, class A2, class A3, class A4, class A5>
    GFunCmd5 (void (*fun)(EvCtx&, A1&, A2&, A3&, A4&, A5&))
        : _fun((_Fun)fun)
        { _initialize<A1, A2, A3, A4, A5>(); }
    template<class Heap, class A1, class A2, class A3, class A4, class A5>
    GFunCmd5 (void (*fun)(TEvCtx<Heap>&, A1&, A2&, A3&, A4&, A5&))
        : _fun((_Fun)fun)
        { _initialize<A1, A2, A3, A4, A5>(); }
private:
    template<class A1, class A2, class A3, class A4, class A5>
    void _initialize ()
        {
            _arg[0].set(&typeid(A1));
            _arg[1].set(&typeid(A2));
            _arg[2].set(&typeid(A3));
            _arg[3].set(&typeid(A4));
            _arg[4].set(&typeid(A5));
        }
    void _execute (EvCtx& ctx) const;
    void (*_fun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&, _Arg&);
};

// ===========================================================================
// vparam (p1[, ...])
// ===========================================================================

template<class P1>
ValParam* vparam (const P1& p1)
    { return new ValParam1<P1>(p1); }

template<class P1, class P2>
ValParam* vparam (const P1& p1, const P2& p2)
    { return new ValParam2<P1, P2>(p1, p2); }

template<class P1, class P2, class P3>
ValParam* vparam (const P1& p1, const P2& p2, const P3& p3)
    { return new ValParam3<P1, P2, P3>(p1, p2, p3); }

template<class P1, class P2, class P3, class P4>
ValParam* vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4)
    { return new ValParam4<P1, P2, P3, P4>(p1, p2, p3, p4); }

template<class P1, class P2, class P3, class P4, class P5>
ValParam* vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
    { return new ValParam5<P1, P2, P3, P4, P5>(p1, p2, p3, p4, p5); }

// ===========================================================================
// rparam (p1[, ...])
// ===========================================================================

template<class P1>
RefParam* rparam (P1& p1)
    { return new RefParam1(p1); }

template<class P1, class P2>
RefParam* rparam (P1& p1, P2& p2)
    { return new RefParam2(p1, p2); }

template<class P1, class P2, class P3>
RefParam* rparam (P1& p1, P2& p2, P3& p3)
    { return new RefParam3(p1, p2, p3); }

template<class P1, class P2, class P3, class P4>
RefParam* rparam (P1& p1, P2& p2, P3& p3, P4& p4)
    { return new RefParam4(p1, p2, p3, p4); }

template<class P1, class P2, class P3, class P4, class P5>
RefParam* rparam (P1& p1, P2& p2, P3& p3, P4& p4, P5& p5)
    { return new RefParam5(p1, p2, p3, p4, p5); }

// ===========================================================================
// handler (obj, memfun)
// ===========================================================================

template<class Obj>
MFunCmd0* handler (Obj& obj, void (Obj::*memfun)(EvCtx&))
    { return new MFunCmd0(obj, memfun); }

template<class Obj, class P1>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(EvCtx&, P1&))
    { return new MFunCmd1(obj, memfun); }

template<class Obj, class P1, class P2>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(EvCtx&, P1&, P2&))
    { return new MFunCmd2(obj, memfun); }

template<class Obj, class P1, class P2, class P3>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(EvCtx&, P1&, P2&, P3&))
    { return new MFunCmd3(obj, memfun); }

template<class Obj, class P1, class P2, class P3, class P4>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(EvCtx&, P1&, P2&, P3&, P4&))
    { return new MFunCmd4(obj, memfun); }

template<class Obj, class P1, class P2, class P3, class P4, class P5>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(EvCtx&, P1&, P2&, P3&, P4&, P5&))
    { return new MFunCmd5(obj, memfun); }

// ------------------------------------

template<class Heap, class Obj>
MFunCmd0* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&))
    { return new MFunCmd0(obj, memfun); }

template<class Heap, class Obj, class P1>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, P1&))
    { return new MFunCmd1(obj, memfun); }

template<class Heap, class Obj, class P1, class P2>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, P1&, P2&))
    { return new MFunCmd2(obj, memfun); }

template<class Heap, class Obj, class P1, class P2, class P3>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, P1&, P2&, P3&))
    { return new MFunCmd3(obj, memfun); }

template<class Heap, class Obj, class P1, class P2, class P3, class P4>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, P1&, P2&, P3&, P4&))
    { return new MFunCmd4(obj, memfun); }

template<class Heap, class Obj, class P1, class P2, class P3, class P4, class P5>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, P1&, P2&, P3&, P4&, P5&))
    { return new MFunCmd5(obj, memfun); }

// ---------------------------------------------------------------------------
// handler (fun)
// ---------------------------------------------------------------------------

template<class P1>
StateCmd* handler (void (*fun)(EvCtx&, P1&))
    { return new GFunCmd1(fun); }

template<class P1, class P2>
StateCmd* handler (void (*fun)(EvCtx&, P1&, P2&))
    { return new GFunCmd2(fun); }

template<class P1, class P2, class P3>
StateCmd* handler (void (*fun)(EvCtx&, P1&, P2&, P3&))
    { return new GFunCmd3(fun); }

template<class P1, class P2, class P3, class P4>
StateCmd* handler (void (*fun)(EvCtx&, P1&, P2&, P3&, P4&))
    { return new GFunCmd4(fun); }

template<class P1, class P2, class P3, class P4, class P5>
StateCmd* handler (void (*fun)(EvCtx&, P1&, P2&, P3&, P4&, P5&))
    { return new GFunCmd5(fun); }

// ------------------------------------

template<class Heap>
StateCmd* handler (void (*fun)(TEvCtx<Heap>&))
    { return new GFunCmd0(fun); }

template<class Heap, class P1>
StateCmd* handler (void (*fun)(TEvCtx<Heap>&, P1&))
    { return new GFunCmd1(fun); }

template<class Heap, class P1, class P2>
StateCmd* handler (void (*fun)(TEvCtx<Heap>&, P1&, P2&))
    { return new GFunCmd2(fun); }

template<class Heap, class P1, class P2, class P3>
StateCmd* handler (void (*fun)(TEvCtx<Heap>&, P1&, P2&, P3&))
    { return new GFunCmd3(fun); }

template<class Heap, class P1, class P2, class P3, class P4>
StateCmd* handler (void (*fun)(TEvCtx<Heap>&, P1&, P2&, P3&, P4&))
    { return new GFunCmd4(fun); }

template<class Heap, class P1, class P2, class P3, class P4, class P5>
StateCmd* handler (void (*fun)(TEvCtx<Heap>&, P1&, P2&, P3&, P4&, P5&))
    { return new GFunCmd5(fun); }

// ===========================================================================

} ///// namespace coe

#endif

