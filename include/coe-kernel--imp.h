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
#error "Never include this header directly!"
#endif

#ifndef __COE_KERNEL__IMP_H
#define __COE_KERNEL__IMP_H

namespace coe { /////

// ===========================================================================
// Kernel

template<class> class _KlsI;

// ------------------------------------
// _KlsD

struct _KlsD {

    _KlsD*              next;
    LocalStorageInfo    info;

    static const _KlsD* registry () { return _register(0); }

private:
    template<class> friend class _KlsI;
    template<class T>
    _KlsD (T* (*_create)(), void (*_destroy)(T*))
        :   next(0)
        {
            info.typeinfo = &typeid(T);
            info.create   = reinterpret_cast<void*(*)()>(_create);
            info.destroy  = reinterpret_cast<void(*)(void*)>(_destroy);
            _register(this);
        }
    static const _KlsD* _register (_KlsD* data);
};

// ------------------------------------
// _KlsI<T>

template<class T>
class _KlsI {
public:
    const _KlsD* data () const;
private:
    const static _KlsD  _data;
};

template<class T>
const _KlsD _KlsI<T>::_data(&Factory<T>::create, &Factory<T>::destroy);

template<class T>
const _KlsD* _KlsI<T>::data () const
    {
        return &_data;
    }

// ------------------------------------
// T& <-- kls<T>()

template<class T>
T& Kernel::kls ()
    {
        return *static_cast<T*>(_get_user_kls(_KlsI<T>().data()));
    }

// ***************************************************************************

class EventArg;
    class ValParam;
    //  class ValParam1<A1> ... ValParam5<A1 .. A5>
    class RefParam;
    //  class RefParam1     ... RefParam5

// ===========================================================================
// EventArg

class EventArg {
public:
    //
    // static operator new/delete...
    //
    virtual ~EventArg ();
    virtual void**         arg_list () const = 0;
    virtual const _TypeDN* arg_type () const = 0;
};

// ---------------------------------------------------------------------------
// ValParam

struct ExecuteContext;

class ValParam : public EventArg {
public:
    // documented API
    virtual ValParam* clone () const = 0;
    template<class _Arg>
    _Arg* argptr (size_t index);
    // implementation
    ValParam () : _locked(0) {}
    ValParam (const ValParam&) : _locked(0) {}
    ~ValParam ();
    void destroy ();
private:
    friend struct ExecuteContext;
    int _locked;    // (-)delete-on-unlock, (0)free, (+)locked
};

template<class _Arg>
_Arg* ValParam::argptr (size_t index)
    {
        const _TypeDN*  tdn = arg_type();
        return index < tdn->len && &typeid(_Arg) == tdn->info[index]->info
             ? static_cast<_Arg*>(arg_list()[index])
             : 0;
    }

// ------------------------------------
// ValParam_N<N>

template<int N>
class ValParam_N : public ValParam {
public:
    /*final*/ void** arg_list () const;
protected:
    void*   _arg[N];
};

template<int N>
void** ValParam_N<N>::arg_list () const
    {
        return (void**)&_arg[0];
    }

// ---------------------------------------------------------------------------
// RefParam

class RefParam : public EventArg {};

// ------------------------------------
// RefParam_N<N>

template<int N>
class RefParam_N : public RefParam {
public:
    /*final*/ void**         arg_list () const;
    /*final*/ const _TypeDN* arg_type () const;
private:
    const _TypeDN*  _info;
protected:
    RefParam_N (const _TypeDN* info) : _info(info) {}
    void*   _arg[N];
};

template<int N>
void** RefParam_N<N>::arg_list () const
    {
        return (void**)&_arg[0];
    }

template<int N>
const _TypeDN* RefParam_N<N>::arg_type () const
    {
        return _info;
    }

// ===========================================================================
// ValParam1<A1> ... ValParam5<A1 .. A5>
// ===========================================================================

template<class A1>
class ValParam1 : public ValParam_N<1> {
public:
    ValParam1 (const A1& a1)
        : _a1(a1)
        {
            _arg[0] = &_a1;
        }
    const _TypeDN* arg_type () const
        { return _TypeI1<A1>().data(); }
    ValParam* clone () const
        { return new ValParam1(_a1); }
private:
    A1  _a1;
};

// ------------------------------------

template<class A1, class A2>
class ValParam2 : public ValParam_N<2> {
public:
    ValParam2 (const A1& a1, const A2& a2)
        : _a1(a1), _a2(a2)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
        }
    const _TypeDN* arg_type () const
        { return _TypeI2<A1, A2>().data(); }
    ValParam* clone () const
        { return new ValParam2(_a1, _a2); }
private:
    A1  _a1;
    A2  _a2;
};

// ------------------------------------

template<class A1, class A2, class A3>
class ValParam3 : public ValParam_N<3> {
public:
    ValParam3 (const A1& a1, const A2& a2, const A3& a3)
        : _a1(a1), _a2(a2), _a3(a3)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
        }
    const _TypeDN* arg_type () const
        { return _TypeI3<A1, A2, A3>().data(); }
    ValParam* clone () const
        { return new ValParam3(_a1, _a2, _a3); }
private:
    A1  _a1;
    A2  _a2;
    A3  _a3;
};

// ------------------------------------

template<class A1, class A2, class A3, class A4>
class ValParam4 : public ValParam_N<4> {
public:
    ValParam4 (const A1& a1, const A2& a2, const A3& a3, const A4& a4)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
        }
    const _TypeDN* arg_type () const
        { return _TypeI4<A1, A2, A3, A4>().data(); }
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
class ValParam5 : public ValParam_N<5> {
public:
    ValParam5 (const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
        }
    const _TypeDN* arg_type () const
        { return _TypeI5<A1, A2, A3, A4, A5>().data(); }
    ValParam* clone () const
        { return new ValParam5(_a1, _a2, _a3, _a4, _a5); }
private:
    A1  _a1;
    A2  _a2;
    A3  _a3;
    A4  _a4;
    A5  _a5;
};

// ===========================================================================
// RefParam1 ... RefParam5
// ===========================================================================

class RefParam1 : public RefParam_N<1> {
public:
    template<class A1>
    RefParam1 (A1& a1)
        :   RefParam_N<1>(_TypeI1<A1>().data())
        {
            _arg[0] = &a1;
        }
};

// ------------------------------------

class RefParam2 : public RefParam_N<2> {
public:
    template<class A1, class A2>
    RefParam2 (A1& a1, A2& a2)
        :   RefParam_N<2>(_TypeI2<A1, A2>().data())
        {
            _arg[0] = &a1;
            _arg[1] = &a2;
        }
};

// ------------------------------------

class RefParam3 : public RefParam_N<3> {
public:
    template<class A1, class A2, class A3>
    RefParam3 (A1& a1, A2& a2, A3& a3)
        :   RefParam_N<3>(_TypeI3<A1, A2, A3>().data())
        {
            _arg[0] = &a1;
            _arg[1] = &a2;
            _arg[2] = &a3;
        }
};

// ------------------------------------

class RefParam4 : public RefParam_N<4> {
public:
    template<class A1, class A2, class A3, class A4>
    RefParam4 (A1& a1, A2& a2, A3& a3, A4& a4)
        :   RefParam_N<4>(_TypeI4<A1, A2, A3, A4>().data())
        {
            _arg[0] = &a1;
            _arg[1] = &a2;
            _arg[2] = &a3;
            _arg[3] = &a4;
        }
};

// ------------------------------------

class RefParam5 : public RefParam_N<5> {
public:
    template<class A1, class A2, class A3, class A4, class A5>
    RefParam5 (A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
        :   RefParam_N<5>(_TypeI5<A1, A2, A3, A4, A5>().data())
        {
            _arg[0] = &a1;
            _arg[1] = &a2;
            _arg[2] = &a3;
            _arg[3] = &a4;
            _arg[4] = &a5;
        }
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

template<class Obj> inline
Handler0 handler (Obj& obj, void (Obj::*fun)(Kernel&))
    { return Handler0(obj, fun); }

template<class Obj, class P1> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3, class P4> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3, class P4, class P5> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3, class P4, class P5, class P6> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3, class P4, class P5, class P6,
                    class P7> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&,
                                                      P7&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3, class P4, class P5, class P6,
                    class P7, class P8> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&,
                                                      P7&, P8&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3, class P4, class P5, class P6,
                    class P7, class P8, class P9> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&,
                                                      P7&, P8&, P9&))
    { return HandlerX(obj, fun); }

// ---------------------------------------------------------------------------
// handler (fun)
// ---------------------------------------------------------------------------

inline
Handler0 handler (void (*fun)(Kernel&))
    { return Handler0(fun); }

template<class P1> inline
HandlerX handler (void (*fun)(Kernel&, P1&))
    { return HandlerX(fun); }

template<class P1, class P2> inline
HandlerX handler (void (*fun)(Kernel&, P1&, P2&))
    { return HandlerX(fun); }

template<class P1, class P2, class P3> inline
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&))
    { return HandlerX(fun); }

template<class P1, class P2, class P3, class P4> inline
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&))
    { return HandlerX(fun); }

template<class P1, class P2, class P3, class P4, class P5> inline
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&))
    { return HandlerX(fun); }

template<class P1, class P2, class P3, class P4, class P5, class P6> inline
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&))
    { return HandlerX(fun); }

template<class P1, class P2, class P3, class P4, class P5, class P6, class P7> inline
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&, P7&))
    { return HandlerX(fun); }

template<class P1, class P2, class P3, class P4, class P5, class P6, class P7,
         class P8> inline
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&))
    { return HandlerX(fun); }

template<class P1, class P2, class P3, class P4, class P5, class P6, class P7,
         class P8, class P9> inline
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&))
    { return HandlerX(fun); }

// ===========================================================================

} ///// namespace coe

#endif

