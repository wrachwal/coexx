// coe-kernel--imp.h

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
    //  class ValParam1<A1> ... ValParam5<A1 .. A9>
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

    //TODO REMOVE as no longer needed
#if 0
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
#endif

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
// ValParam1<A1> ... ValParam5<A1 .. A9>
// ===========================================================================

#if 0
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
#else
template<class ARGS>
class ValParamA<ARGS, 1> : public ValParam {
public:
    enum { N = 1 };
    typedef typename Nth<0, ARGS>::type A1;
    ValParamA (const A1& a1)
        : _a1(a1)
        {
            _arg[0] = &_a1;
        }
    const _TypeDN* arg_type () const    { return _TypeI1<A1>().data(); }
    void** arg_list () const            { return (void**)&_arg[0]; }
    ValParam* clone () const            { return new ValParamA(_a1); }
private:
    void*   _arg[N];
    A1      _a1;
};
#endif

// ------------------------------------

#if 0
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
#else
template<class ARGS>
class ValParamA<ARGS, 2> : public ValParam {
public:
    enum { N = 2 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    ValParamA (const A1& a1, const A2& a2)
        : _a1(a1), _a2(a2)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
        }
    const _TypeDN* arg_type () const    { return _TypeI2<A1, A2>().data(); }
    void** arg_list () const            { return (void**)&_arg[0]; }
    ValParam* clone () const            { return new ValParamA(_a1, _a2); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
};
#endif

// ------------------------------------

#if 0
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
#else
#if 0
//XXX: I started with the following version but got the compiler errors!!!
//########################################################################
//include/coe-kernel--imp.h:XXX: error: template parameters not used in partial specialization:
//include/coe-kernel--imp.h:XXX: error:         ‘A1’
//include/coe-kernel--imp.h:XXX: error:         ‘A2’
//include/coe-kernel--imp.h:XXX: error:         ‘A3’
template<class A1, class A2, class A3>
class ValParamA<typename List3<A1, A2, A3>::type, 3> : public ValParam {    //XXX <--- error in this line?!!!
public:
    enum { N = 3 };
    typedef typename List3<A1, A2, A3>::type ARGS;
    ValParamA (const A1& a1, const A2& a2, const A3& a3)
        : _a1(a1), _a2(a2), _a3(a3)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
        }
    const _TypeDN* arg_type () const    { return _TypeI3<A1, A2, A3>().data(); }
    void** arg_list () const            { return (void**)&_arg[0]; }
    ValParam* clone () const            { return new ValParamA(_a1, _a2, _a3); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
    A3      _a3;
};
#else
template<class ARGS>
class ValParamA<ARGS, 3> : public ValParam {
public:
    enum { N = 3 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    ValParamA (const A1& a1, const A2& a2, const A3& a3)
        : _a1(a1), _a2(a2), _a3(a3)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
        }
    const _TypeDN* arg_type () const    { return _TypeI3<A1, A2, A3>().data(); }
    void** arg_list () const            { return (void**)&_arg[0]; }
    ValParam* clone () const            { return new ValParamA(_a1, _a2, _a3); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
    A3      _a3;
};
#endif
#endif

// ------------------------------------

#if 0
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
#else
template<class ARGS>
class ValParamA<ARGS, 4> : public ValParam {
public:
    enum { N = 4 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    ValParamA (const A1& a1, const A2& a2, const A3& a3, const A4& a4)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
        }
    const _TypeDN* arg_type () const    { return _TypeI4<A1, A2, A3, A4>().data(); }
    void** arg_list () const            { return (void**)&_arg[0]; }
    ValParam* clone () const            { return new ValParamA(_a1, _a2, _a3, _a4); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
    A3      _a3;
    A4      _a4;
};
#endif

// ------------------------------------

#if 0
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
#else
template<class ARGS>
class ValParamA<ARGS, 5> : public ValParam {
public:
    enum { N = 5 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    ValParamA (const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
        }
    const _TypeDN* arg_type () const    { return _TypeI5<A1, A2, A3, A4, A5>().data(); }
    void** arg_list () const            { return (void**)&_arg[0]; }
    ValParam* clone () const            { return new ValParamA(_a1, _a2, _a3, _a4, _a5); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
    A3      _a3;
    A4      _a4;
    A5      _a5;
};
#endif

// ------------------------------------

#if 0
template<class A1, class A2, class A3, class A4, class A5,
         class A6>
class ValParam6 : public ValParam_N<6> {
public:
    ValParam6 (const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5,
               const A6& a6)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5), _a6(a6)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
            _arg[5] = &_a6;
        }
    const _TypeDN* arg_type () const
        { return _TypeI6<A1, A2, A3, A4, A5, A6>().data(); }
    ValParam* clone () const
        { return new ValParam6(_a1, _a2, _a3, _a4, _a5, _a6); }
private:
    A1  _a1;
    A2  _a2;
    A3  _a3;
    A4  _a4;
    A5  _a5;
    A6  _a6;
};
#else
template<class ARGS>
class ValParamA<ARGS, 6> : public ValParam {
public:
    enum { N = 6 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    ValParamA (const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5,
               const A6& a6)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5), _a6(a6)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
            _arg[5] = &_a6;
        }
    const _TypeDN* arg_type () const    { return _TypeI6<A1, A2, A3, A4, A5, A6>().data(); }
    void** arg_list () const            { return (void**)&_arg[0]; }
    ValParam* clone () const            { return new ValParamA(_a1, _a2, _a3, _a4, _a5, _a6); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
    A3      _a3;
    A4      _a4;
    A5      _a5;
    A6      _a6;
};
#endif

// ------------------------------------

#if 0
template<class A1, class A2, class A3, class A4, class A5,
         class A6, class A7>
class ValParam7 : public ValParam_N<7> {
public:
    ValParam7 (const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5,
               const A6& a6, const A7& a7)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5), _a6(a6), _a7(a7)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
            _arg[5] = &_a6;
            _arg[6] = &_a7;
        }
    const _TypeDN* arg_type () const
        { return _TypeI7<A1, A2, A3, A4, A5, A6, A7>().data(); }
    ValParam* clone () const
        { return new ValParam7(_a1, _a2, _a3, _a4, _a5, _a6, _a7); }
private:
    A1  _a1;
    A2  _a2;
    A3  _a3;
    A4  _a4;
    A5  _a5;
    A6  _a6;
    A7  _a7;
};
#else
template<class ARGS>
class ValParamA<ARGS, 7> : public ValParam {
public:
    enum { N = 7 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    typedef typename Nth<6, ARGS>::type A7;
    ValParamA (const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5,
               const A6& a6, const A7& a7)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5), _a6(a6), _a7(a7)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
            _arg[5] = &_a6;
            _arg[6] = &_a7;
        }
    const _TypeDN* arg_type () const    { return _TypeI7<A1, A2, A3, A4, A5, A6, A7>().data(); }
    void** arg_list () const            { return (void**)&_arg[0]; }
    ValParam* clone () const            { return new ValParamA(_a1, _a2, _a3, _a4, _a5, _a6, _a7); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
    A3      _a3;
    A4      _a4;
    A5      _a5;
    A6      _a6;
    A7      _a7;
};
#endif

// ------------------------------------

#if 0
template<class A1, class A2, class A3, class A4, class A5,
         class A6, class A7, class A8>
class ValParam8 : public ValParam_N<8> {
public:
    ValParam8 (const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5,
               const A6& a6, const A7& a7, const A8& a8)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5), _a6(a6), _a7(a7), _a8(a8)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
            _arg[5] = &_a6;
            _arg[6] = &_a7;
            _arg[7] = &_a8;
        }
    const _TypeDN* arg_type () const
        { return _TypeI8<A1, A2, A3, A4, A5, A6, A7, A8>().data(); }
    ValParam* clone () const
        { return new ValParam8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8); }
private:
    A1  _a1;
    A2  _a2;
    A3  _a3;
    A4  _a4;
    A5  _a5;
    A6  _a6;
    A7  _a7;
    A8  _a8;
};
#else
template<class ARGS>
class ValParamA<ARGS, 8> : public ValParam {
public:
    enum { N = 8 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    typedef typename Nth<6, ARGS>::type A7;
    typedef typename Nth<7, ARGS>::type A8;
    ValParamA (const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5,
               const A6& a6, const A7& a7, const A8& a8)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5), _a6(a6), _a7(a7), _a8(a8)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
            _arg[5] = &_a6;
            _arg[6] = &_a7;
            _arg[7] = &_a8;
        }
    const _TypeDN* arg_type () const    { return _TypeI8<A1, A2, A3, A4, A5, A6, A7, A8>().data(); }
    void** arg_list () const            { return (void**)&_arg[0]; }
    ValParam* clone () const            { return new ValParamA(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
    A3      _a3;
    A4      _a4;
    A5      _a5;
    A6      _a6;
    A7      _a7;
    A8      _a8;
};
#endif

// ------------------------------------

#if 0
template<class A1, class A2, class A3, class A4, class A5,
         class A6, class A7, class A8, class A9>
class ValParam9 : public ValParam_N<9> {
public:
    ValParam9 (const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5,
               const A6& a6, const A7& a7, const A8& a8, const A9& a9)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5), _a6(a6), _a7(a7), _a8(a8), _a9(a9)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
            _arg[5] = &_a6;
            _arg[6] = &_a7;
            _arg[7] = &_a8;
            _arg[8] = &_a9;
        }
    const _TypeDN* arg_type () const
        { return _TypeI9<A1, A2, A3, A4, A5, A6, A7, A8, A9>().data(); }
    ValParam* clone () const
        { return new ValParam9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9); }
private:
    A1  _a1;
    A2  _a2;
    A3  _a3;
    A4  _a4;
    A5  _a5;
    A6  _a6;
    A7  _a7;
    A8  _a8;
    A9  _a9;
};
#else
template<class ARGS>
class ValParamA<ARGS, 9> : public ValParam {
public:
    enum { N = 9 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    typedef typename Nth<6, ARGS>::type A7;
    typedef typename Nth<7, ARGS>::type A8;
    typedef typename Nth<8, ARGS>::type A9;
    ValParamA (const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5,
               const A6& a6, const A7& a7, const A8& a8, const A9& a9)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5), _a6(a6), _a7(a7), _a8(a8), _a9(a9)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
            _arg[5] = &_a6;
            _arg[6] = &_a7;
            _arg[7] = &_a8;
            _arg[8] = &_a9;
        }
    const _TypeDN* arg_type () const    { return _TypeI9<A1, A2, A3, A4, A5, A6, A7, A8, A9>().data(); }
    void** arg_list () const            { return (void**)&_arg[0]; }
    ValParam* clone () const            { return new ValParamA(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
    A3      _a3;
    A4      _a4;
    A5      _a5;
    A6      _a6;
    A7      _a7;
    A8      _a8;
    A9      _a9;
};
#endif

// ===========================================================================
// RefParam1 ... RefParam9
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

// ------------------------------------

class RefParam6 : public RefParam_N<6> {
public:
    template<class A1, class A2, class A3, class A4, class A5,
             class A6>
    RefParam6 (A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6)
        :   RefParam_N<6>(_TypeI6<A1, A2, A3, A4, A5, A6>().data())
        {
            _arg[0] = &a1;
            _arg[1] = &a2;
            _arg[2] = &a3;
            _arg[3] = &a4;
            _arg[4] = &a5;
            _arg[5] = &a6;
        }
};

// ------------------------------------

class RefParam7 : public RefParam_N<7> {
public:
    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7>
    RefParam7 (A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7)
        :   RefParam_N<7>(_TypeI7<A1, A2, A3, A4, A5, A6, A7>().data())
        {
            _arg[0] = &a1;
            _arg[1] = &a2;
            _arg[2] = &a3;
            _arg[3] = &a4;
            _arg[4] = &a5;
            _arg[5] = &a6;
            _arg[6] = &a7;
        }
};

// ------------------------------------

class RefParam8 : public RefParam_N<8> {
public:
    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7, class A8>
    RefParam8 (A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8)
        :   RefParam_N<8>(_TypeI8<A1, A2, A3, A4, A5, A6, A7, A8>().data())
        {
            _arg[0] = &a1;
            _arg[1] = &a2;
            _arg[2] = &a3;
            _arg[3] = &a4;
            _arg[4] = &a5;
            _arg[5] = &a6;
            _arg[6] = &a7;
            _arg[7] = &a8;
        }
};

// ------------------------------------

class RefParam9 : public RefParam_N<9> {
public:
    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7, class A8, class A9>
    RefParam9 (A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8, A9& a9)
        :   RefParam_N<9>(_TypeI9<A1, A2, A3, A4, A5, A6, A7, A8, A9>().data())
        {
            _arg[0] = &a1;
            _arg[1] = &a2;
            _arg[2] = &a3;
            _arg[3] = &a4;
            _arg[4] = &a5;
            _arg[5] = &a6;
            _arg[6] = &a7;
            _arg[7] = &a8;
            _arg[8] = &a9;
        }
};

// ===========================================================================
// vparam (p1[, ...p9])
// ===========================================================================

#if 0
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

template<class P1, class P2, class P3, class P4, class P5,
         class P6>
ValParam* vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5,
                  const P6& p6)
    { return new ValParam6<P1, P2, P3, P4, P5, P6>
                          (p1, p2, p3, p4, p5, p6); }

template<class P1, class P2, class P3, class P4, class P5,
         class P6, class P7>
ValParam* vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5,
                  const P6& p6, const P7& p7)
    { return new ValParam7<P1, P2, P3, P4, P5, P6, P7>
                          (p1, p2, p3, p4, p5, p6, p7); }

template<class P1, class P2, class P3, class P4, class P5,
         class P6, class P7, class P8>
ValParam* vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5,
                  const P6& p6, const P7& p7, const P8& p8)
    { return new ValParam8<P1, P2, P3, P4, P5, P6, P7, P8>
                          (p1, p2, p3, p4, p5, p6, p7, p8); }

template<class P1, class P2, class P3, class P4, class P5,
         class P6, class P7, class P8, class P9>
ValParam* vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5,
                  const P6& p6, const P7& p7, const P8& p8, const P9& p9)
    { return new ValParam9<P1, P2, P3, P4, P5, P6, P7, P8, P9>
                          (p1, p2, p3, p4, p5, p6, p7, p8, p9); }
#else
#if 0
    //XXX 1 & 2 -- args vparam versions was the first attempt
template<class P1>
ValParam_<typename List1<P1>::type>
vparam (const P1& p1)
    { return new ValParam1<P1>(p1); }

template<class P1, class P2>
ValParam_<typename List2<P1, P2>::type>
vparam (const P1& p1, const P2& p2)
    { return new ValParam2<P1, P2>(p1, p2); }
#else
template<class P1>
ValParam_<typename List1<P1>::type>
vparam (const P1& p1)
    { return new ValParamA<typename List1<P1>::type>(p1); }

template<class P1, class P2>
ValParam_<typename List2<P1, P2>::type>
vparam (const P1& p1, const P2& p2)
    { return new ValParamA<typename List2<P1, P2>::type>(p1, p2); }
#endif

template<class P1, class P2, class P3>
ValParam_<typename List3<P1, P2, P3>::type>
vparam (const P1& p1, const P2& p2, const P3& p3)
    { return new ValParamA<typename List3<P1, P2, P3>::type>(p1, p2, p3); }

template<class P1, class P2, class P3, class P4>
ValParam_<typename List4<P1, P2, P3, P4>::type>
vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4)
    { return new ValParamA<typename List4<P1, P2, P3, P4>::type>(p1, p2, p3, p4); }

template<class P1, class P2, class P3, class P4, class P5>
ValParam_<typename List5<P1, P2, P3, P4, P5>::type>
vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
    { return new ValParamA<typename List5<P1, P2, P3, P4, P5>::type>(p1, p2, p3, p4, p5); }

template<class P1, class P2, class P3, class P4, class P5, class P6>
ValParam_<typename List6<P1, P2, P3, P4, P5, P6>::type>
vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
    { return new ValParamA<typename List6<P1, P2, P3, P4, P5, P6>::type>(p1, p2, p3, p4, p5, p6); }

template<class P1, class P2, class P3, class P4, class P5, class P6, class P7>
ValParam_<typename List7<P1, P2, P3, P4, P5, P6, P7>::type>
vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
    { return new ValParamA<typename List7<P1, P2, P3, P4, P5, P6, P7>::type>(p1, p2, p3, p4, p5, p6, p7); }

template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
ValParam_<typename List8<P1, P2, P3, P4, P5, P6, P7, P8>::type>
vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7, const P8& p8)
    { return new ValParamA<typename List8<P1, P2, P3, P4, P5, P6, P7, P8>::type>(p1, p2, p3, p4, p5, p6, p7, p8); }

template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
ValParam_<typename List9<P1, P2, P3, P4, P5, P6, P7, P8, P9>::type>
vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7, const P8& p8, const P9& p9)
    { return new ValParamA<typename List9<P1, P2, P3, P4, P5, P6, P7, P8, P9>::type>(p1, p2, p3, p4, p5, p6, p7, p8, p9); }
#endif

// ===========================================================================
// rparam (p1[, ...p9])
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

template<class P1, class P2, class P3, class P4, class P5,
         class P6>
RefParam* rparam (P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6)
    { return new RefParam6(p1, p2, p3, p4, p5, p6); }

template<class P1, class P2, class P3, class P4, class P5,
         class P6, class P7>
RefParam* rparam (P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7)
    { return new RefParam7(p1, p2, p3, p4, p5, p6, p7); }

template<class P1, class P2, class P3, class P4, class P5,
         class P6, class P7, class P8>
RefParam* rparam (P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7, P8& p8)
    { return new RefParam8(p1, p2, p3, p4, p5, p6, p7, p8); }

template<class P1, class P2, class P3, class P4, class P5,
         class P6, class P7, class P8, class P9>
RefParam* rparam (P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7, P8& p8, P9& p9)
    { return new RefParam9(p1, p2, p3, p4, p5, p6, p7, p8, p9); }

// ===========================================================================
// handler (obj, memfun)
// ===========================================================================

template<class Obj> inline
Handler0 handler (Obj& obj, void (Obj::*fun)(Kernel&))
    { return Handler0(obj, fun); }

#if 0
template<class Obj, class P1> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&))
    { return HandlerX(obj, fun); }
#else
template<class Obj, class P1> inline
Handler_<typename List1<P1>::type>
handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&))
    { return Handler_<typename List1<P1>::type>(obj, fun); }

template<class Obj, class P1, class P2> inline
Handler_<typename List2<P1, P2>::type>
handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&))
    { return Handler_<typename List2<P1, P2>::type>(obj, fun); }
#endif

template<class Obj, class P1, class P2, class P3> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3, class P4> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3, class P4, class P5> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3, class P4, class P5,
                    class P6> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&,
                                                      P6&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3, class P4, class P5,
                    class P6, class P7> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&,
                                                      P6&, P7&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3, class P4, class P5,
                    class P6, class P7, class P8> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&,
                                                      P6&, P7&, P8&))
    { return HandlerX(obj, fun); }

template<class Obj, class P1, class P2, class P3, class P4, class P5,
                    class P6, class P7, class P8, class P9> inline
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&,
                                                      P6&, P7&, P8&, P9&))
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

template<class P1, class P2, class P3, class P4, class P5,
         class P6> inline
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&))
    { return HandlerX(fun); }

template<class P1, class P2, class P3, class P4, class P5,
         class P6, class P7> inline
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&, P7&))
    { return HandlerX(fun); }

template<class P1, class P2, class P3, class P4, class P5,
         class P6, class P7, class P8> inline
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&))
    { return HandlerX(fun); }

template<class P1, class P2, class P3, class P4, class P5,
         class P6, class P7, class P8, class P9> inline
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&))
    { return HandlerX(fun); }

// ===========================================================================

} ///// namespace coe

#endif

