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
#error "never include this header directly!"
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
class StateCmd;
    //  class MFunCmd0 ... MFunCmd5
    //  class GFunCmd0 ... GFunCmd5

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

template<class>                             class _TypeI1;
template<class, class>                      class _TypeI2;
template<class, class, class>               class _TypeI3;
template<class, class, class, class>        class _TypeI4;
template<class, class, class, class, class> class _TypeI5;

struct _TypeDN {
    const _TypeD**  info;   // NULL-terminated array
    size_t          len;
    const _TypeDN*  next;
    size_t          pos;

    static const _TypeDN* registry () { return _register(0); }

private:
    template<class>                             friend class _TypeI1;
    template<class, class>                      friend class _TypeI2;
    template<class, class, class>               friend class _TypeI3;
    template<class, class, class, class>        friend class _TypeI4;
    template<class, class, class, class, class> friend class _TypeI5;

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
    virtual ValParam* clone () const = 0;
    // implementation
    ValParam () : _locked(0) {}
    ValParam (const ValParam&) : _locked(0) {}
    ~ValParam ();
    void destroy ();
private:
    friend struct ExecuteContext;
    int _locked;    // (-)delete-on-unlock, (0)free, (+)locked
};

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
// StateCmd

class StateCmd {
public:
    //
    // static operator new/delete...
    //
    virtual ~StateCmd ();

    const _TypeDN* par_type () const { return _info; }

    virtual void execute (EvCtx& ctx, void* arg[]) const = 0;

protected:
    StateCmd (const _TypeDN* info) : _info(info) {}

private:
    const _TypeDN*  _info;
};

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
// MFunCmd0 ... MFunCmd5
// ===========================================================================

class MFunCmd0 : public StateCmd {
public:
    class _Obj;
    typedef void (_Obj::*_MemFun)(EvCtx&);
    template<class Obj>
    MFunCmd0 (Obj& obj, void (Obj::*memfun)(EvCtx&))
        :   StateCmd(0),
            _obj((_Obj*)&obj),
            _memfun((_MemFun)memfun)
        {}
    void execute (EvCtx& ctx, void* arg[]) const;
private:
    _Obj* _obj;
    void (_Obj::*_memfun)(EvCtx&);
};

// ------------------------------------

class MFunCmd1 : public StateCmd {
public:
    class _Obj;
    class _Arg;
    typedef void (_Obj::*_MemFun)(EvCtx&, _Arg&);
    template<class Obj, class A1>
    MFunCmd1 (Obj& obj, void (Obj::*memfun)(EvCtx&, A1&))
        :   StateCmd(_TypeI1<A1>().data()),
            _obj((_Obj*)&obj),
            _memfun((_MemFun)memfun)
        {}
    void execute (EvCtx& ctx, void* arg[]) const;
private:
    _Obj* _obj;
    void (_Obj::*_memfun)(EvCtx&, _Arg&);
};

// ------------------------------------

class MFunCmd2 : public StateCmd {
public:
    class _Obj;
    class _Arg;
    typedef void (_Obj::*_MemFun)(EvCtx&, _Arg&, _Arg&);
    template<class Obj, class A1, class A2>
    MFunCmd2 (Obj& obj, void (Obj::*memfun)(EvCtx&, A1&, A2&))
        :   StateCmd(_TypeI2<A1, A2>().data()),
            _obj((_Obj*)&obj),
            _memfun((_MemFun)memfun)
        {}
    void execute (EvCtx& ctx, void* arg[]) const;
private:
    _Obj* _obj;
    void (_Obj::*_memfun)(EvCtx&, _Arg&, _Arg&);
};

// ------------------------------------

class MFunCmd3 : public StateCmd {
public:
    class _Obj;
    class _Arg;
    typedef void (_Obj::*_MemFun)(EvCtx&, _Arg&, _Arg&, _Arg&);
    template<class Obj, class A1, class A2, class A3>
    MFunCmd3 (Obj& obj, void (Obj::*memfun)(EvCtx&, A1&, A2&, A3&))
        :   StateCmd(_TypeI3<A1, A2, A3>().data()),
            _obj((_Obj*)&obj),
            _memfun((_MemFun)memfun)
        {}
    void execute (EvCtx& ctx, void* arg[]) const;
private:
    _Obj* _obj;
    void (_Obj::*_memfun)(EvCtx&, _Arg&, _Arg&, _Arg&);
};

// ------------------------------------

class MFunCmd4 : public StateCmd {
public:
    class _Obj;
    class _Arg;
    typedef void (_Obj::*_MemFun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&);
    template<class Obj, class A1, class A2, class A3, class A4>
    MFunCmd4 (Obj& obj, void (Obj::*memfun)(EvCtx&, A1&, A2&, A3&, A4&))
        :   StateCmd(_TypeI4<A1, A2, A3, A4>().data()),
            _obj((_Obj*)&obj),
            _memfun((_MemFun)memfun)
        {}
    void execute (EvCtx& ctx, void* arg[]) const;
private:
    _Obj* _obj;
    void (_Obj::*_memfun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&);
};

// ------------------------------------

class MFunCmd5 : public StateCmd {
public:
    class _Obj;
    class _Arg;
    typedef void (_Obj::*_MemFun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&, _Arg&);
    template<class Obj, class A1, class A2, class A3, class A4, class A5>
    MFunCmd5 (Obj& obj, void (Obj::*memfun)(EvCtx&, A1&, A2&, A3&, A4&, A5&))
        :   StateCmd(_TypeI5<A1, A2, A3, A4, A5>().data()),
            _obj((_Obj*)&obj),
            _memfun((_MemFun)memfun)
        {}
    void execute (EvCtx& ctx, void* arg[]) const;
private:
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
        :   StateCmd(0),
            _fun((_Fun)fun)
        {}
    void execute (EvCtx& ctx, void* arg[]) const;
private:
    void (*_fun)(EvCtx&);
};

// ------------------------------------

class GFunCmd1 : public StateCmd {
public:
    class _Arg;
    typedef void (*_Fun)(EvCtx&, _Arg&);
    template<class A1>
    GFunCmd1 (void (*fun)(EvCtx&, A1&))
        :   StateCmd(_TypeI1<A1>().data()),
            _fun((_Fun)fun)
        {}
    void execute (EvCtx& ctx, void* arg[]) const;
private:
    void (*_fun)(EvCtx&, _Arg&);
};

// ------------------------------------

class GFunCmd2 : public StateCmd {
public:
    class _Arg;
    typedef void (*_Fun)(EvCtx&, _Arg&, _Arg&);
    template<class A1, class A2>
    GFunCmd2 (void (*fun)(EvCtx&, A1&, A2&))
        :   StateCmd(_TypeI2<A1, A2>().data()),
            _fun((_Fun)fun)
        {}
    void execute (EvCtx& ctx, void* arg[]) const;
private:
    void (*_fun)(EvCtx&, _Arg&, _Arg&);
};

// ------------------------------------

class GFunCmd3 : public StateCmd {
public:
    class _Arg;
    typedef void (*_Fun)(EvCtx&, _Arg&, _Arg&, _Arg&);
    template<class A1, class A2, class A3>
    GFunCmd3 (void (*fun)(EvCtx&, A1&, A2&, A3&))
        :   StateCmd(_TypeI3<A1, A2, A3>().data()),
            _fun((_Fun)fun)
        {}
    void execute (EvCtx& ctx, void* arg[]) const;
private:
    void (*_fun)(EvCtx&, _Arg&, _Arg&, _Arg&);
};

// ------------------------------------

class GFunCmd4 : public StateCmd {
public:
    class _Arg;
    typedef void (*_Fun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&);
    template<class A1, class A2, class A3, class A4>
    GFunCmd4 (void (*fun)(EvCtx&, A1&, A2&, A3&, A4&))
        :   StateCmd(_TypeI4<A1, A2, A3, A4>().data()),
            _fun((_Fun)fun)
        {}
    void execute (EvCtx& ctx, void* arg[]) const;
private:
    void (*_fun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&);
};

// ------------------------------------

class GFunCmd5 : public StateCmd {
public:
    class _Arg;
    typedef void (*_Fun)(EvCtx&, _Arg&, _Arg&, _Arg&, _Arg&, _Arg&);
    template<class A1, class A2, class A3, class A4, class A5>
    GFunCmd5 (void (*fun)(EvCtx&, A1&, A2&, A3&, A4&, A5&))
        :   StateCmd(_TypeI5<A1, A2, A3, A4, A5>().data()),
            _fun((_Fun)fun)
        {}
    void execute (EvCtx& ctx, void* arg[]) const;
private:
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

// ===========================================================================

} ///// namespace coe

#endif

