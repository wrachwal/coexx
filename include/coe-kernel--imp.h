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
    //  class ValParamA<ARGS, 1 .. 9>
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
    virtual void**          arg_list () const = 0;
    virtual const ArgListI* arg_type () const = 0;
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
        const ArgListI* tdn = arg_type();
        return index < tdn->len && TypeInfo(typeid(_Arg)) == tdn->arg[index]->type
             ? static_cast<_Arg*>(arg_list()[index])
             : 0;
    }

// ---------------------------------------------------------------------------
// RefParam

class RefParam : public EventArg {};

// ------------------------------------
// RefParam_N<N>

template<int N>
class RefParam_N : public RefParam {
public:
    /*final*/ void**            arg_list () const;
    /*final*/ const ArgListI*   arg_type () const;
private:
    const ArgListI* _info;
protected:
    RefParam_N (const ArgListI* info) : _info(info) {}
    void*           _arg[N];
};

template<int N>
void** RefParam_N<N>::arg_list () const
    {
        return (void**)&_arg[0];
    }

template<int N>
const ArgListI* RefParam_N<N>::arg_type () const
    {
        return _info;
    }

// ===========================================================================
// ValParamA<ARGS, 1 .. 9>
// ===========================================================================

template<class ARGS>
class ValParamA<ARGS, 1> : public ValParam {
public:
    enum { N = 1 };
    typedef typename Nth<0, ARGS>::type A1;
    ValParamA (COE_TA(1, const A, &a)) : COE_PA(1, _a, a)
        {
            _arg[0] = &_a1;
        }
    const ArgListI* arg_type () const { return & Rtti<ARGS, ArgListI>::meta()->info; }
    void** arg_list () const          { return (void**)&_arg[0]; }
    ValParam* clone () const          { return new ValParamA(COE_T(1, _a)); }
private:
    void*   _arg[N];
    A1      _a1;
};

// ------------------------------------

template<class ARGS>
class ValParamA<ARGS, 2> : public ValParam {
public:
    enum { N = 2 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    ValParamA (COE_TA(2, const A, &a)) : COE_PA(2, _a, a)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
        }
    const ArgListI* arg_type () const { return & Rtti<ARGS, ArgListI>::meta()->info; }
    void** arg_list () const          { return (void**)&_arg[0]; }
    ValParam* clone () const          { return new ValParamA(COE_T(2, _a)); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
};

// ------------------------------------

template<class ARGS>
class ValParamA<ARGS, 3> : public ValParam {
public:
    enum { N = 3 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    ValParamA (COE_TA(3, const A, &a)) : COE_PA(3, _a, a)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
        }
    const ArgListI* arg_type () const { return & Rtti<ARGS, ArgListI>::meta()->info; }
    void** arg_list () const          { return (void**)&_arg[0]; }
    ValParam* clone () const          { return new ValParamA(COE_T(3, _a)); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
    A3      _a3;
};

// ------------------------------------

template<class ARGS>
class ValParamA<ARGS, 4> : public ValParam {
public:
    enum { N = 4 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    ValParamA (COE_TA(4, const A, &a)) : COE_PA(4, _a, a)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
        }
    const ArgListI* arg_type () const { return & Rtti<ARGS, ArgListI>::meta()->info; }
    void** arg_list () const          { return (void**)&_arg[0]; }
    ValParam* clone () const          { return new ValParamA(COE_T(4, _a)); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
    A3      _a3;
    A4      _a4;
};

// ------------------------------------

template<class ARGS>
class ValParamA<ARGS, 5> : public ValParam {
public:
    enum { N = 5 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    ValParamA (COE_TA(5, const A, &a)) : COE_PA(5, _a, a)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
        }
    const ArgListI* arg_type () const { return & Rtti<ARGS, ArgListI>::meta()->info; }
    void** arg_list () const          { return (void**)&_arg[0]; }
    ValParam* clone () const          { return new ValParamA(COE_T(5, _a)); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
    A3      _a3;
    A4      _a4;
    A5      _a5;
};

// ------------------------------------

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
    ValParamA (COE_TA(6, const A, &a)) : COE_PA(6, _a, a)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
            _arg[5] = &_a6;
        }
    const ArgListI* arg_type () const { return & Rtti<ARGS, ArgListI>::meta()->info; }
    void** arg_list () const          { return (void**)&_arg[0]; }
    ValParam* clone () const          { return new ValParamA(COE_T(6, _a)); }
private:
    void*   _arg[N];
    A1      _a1;
    A2      _a2;
    A3      _a3;
    A4      _a4;
    A5      _a5;
    A6      _a6;
};

// ------------------------------------

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
    ValParamA (COE_TA(7, const A, &a)) : COE_PA(7, _a, a)
        {
            _arg[0] = &_a1;
            _arg[1] = &_a2;
            _arg[2] = &_a3;
            _arg[3] = &_a4;
            _arg[4] = &_a5;
            _arg[5] = &_a6;
            _arg[6] = &_a7;
        }
    const ArgListI* arg_type () const { return & Rtti<ARGS, ArgListI>::meta()->info; }
    void** arg_list () const          { return (void**)&_arg[0]; }
    ValParam* clone () const          { return new ValParamA(COE_T(7, _a)); }
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

// ------------------------------------

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
    ValParamA (COE_TA(8, const A, &a)) : COE_PA(8, _a, a)
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
    const ArgListI* arg_type () const { return & Rtti<ARGS, ArgListI>::meta()->info; }
    void** arg_list () const          { return (void**)&_arg[0]; }
    ValParam* clone () const          { return new ValParamA(COE_T(8, _a)); }
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

// ------------------------------------

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
    ValParamA (COE_TA(9, const A, &a)) : COE_PA(9, _a, a)
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
    const ArgListI* arg_type () const { return & Rtti<ARGS, ArgListI>::meta()->info; }
    void** arg_list () const          { return (void**)&_arg[0]; }
    ValParam* clone () const          { return new ValParamA(COE_T(9, _a)); }
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

// ===========================================================================
// RefParam1 ... RefParam9
// ===========================================================================

class RefParam1 : public RefParam_N<1> {
public:
    template<COE_T(1, class A)>
    RefParam1 (COE_TA(1, A, &a))
        :   RefParam_N<1>(& Rtti<typename List1<COE_T(1, A)>::type, ArgListI>::meta()->info)
        {
            _arg[0] = &a1;
        }
};

// ------------------------------------

class RefParam2 : public RefParam_N<2> {
public:
    template<COE_T(2, class A)>
    RefParam2 (COE_TA(2, A, &a))
        :   RefParam_N<2>(& Rtti<typename List2<COE_T(2, A)>::type, ArgListI>::meta()->info)
        {
            _arg[0] = &a1;
            _arg[1] = &a2;
        }
};

// ------------------------------------

class RefParam3 : public RefParam_N<3> {
public:
    template<COE_T(3, class A)>
    RefParam3 (COE_TA(3, A, &a))
        :   RefParam_N<3>(& Rtti<typename List3<COE_T(3, A)>::type, ArgListI>::meta()->info)
        {
            _arg[0] = &a1;
            _arg[1] = &a2;
            _arg[2] = &a3;
        }
};

// ------------------------------------

class RefParam4 : public RefParam_N<4> {
public:
    template<COE_T(4, class A)>
    RefParam4 (COE_TA(4, A, &a))
        :   RefParam_N<4>(& Rtti<typename List4<COE_T(4, A)>::type, ArgListI>::meta()->info)
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
    template<COE_T(5, class A)>
    RefParam5 (COE_TA(5, A, &a))
        :   RefParam_N<5>(& Rtti<typename List5<COE_T(5, A)>::type, ArgListI>::meta()->info)
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
    template<COE_T(6, class A)>
    RefParam6 (COE_TA(6, A, &a))
        :   RefParam_N<6>(& Rtti<typename List6<COE_T(6, A)>::type, ArgListI>::meta()->info)
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
    template<COE_T(7, class A)>
    RefParam7 (COE_TA(7, A, &a))
        :   RefParam_N<7>(& Rtti<typename List7<COE_T(7, A)>::type, ArgListI>::meta()->info)
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
    template<COE_T(8, class A)>
    RefParam8 (COE_TA(8, A, &a))
        :   RefParam_N<8>(& Rtti<typename List8<COE_T(8, A)>::type, ArgListI>::meta()->info)
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
    template<COE_T(9, class A)>
    RefParam9 (COE_TA(9, A, &a))
        :   RefParam_N<9>(& Rtti<typename List9<COE_T(9, A)>::type, ArgListI>::meta()->info)
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
// RefParam_<ARGS, N>
// ===========================================================================

template<class ARGS>
class RefParam_<ARGS, 1> {
public:
    operator RefParam* () { return _ptr; }
    RefParam*    clear () { RefParam* tmp = _ptr; _ptr = 0; return tmp; }
    enum { N = 1 };
    typedef typename Nth<0, ARGS>::type A1;
    RefParam_ () : _ptr(0) {}
    RefParam_ (COE_TA(1, A, &a)) : _ptr(new RefParam1(COE_T(1, a))) {}
private:
    RefParam*   _ptr;
};

template<class ARGS>
class RefParam_<ARGS, 2> {
public:
    operator RefParam* () { return _ptr; }
    RefParam*    clear () { RefParam* tmp = _ptr; _ptr = 0; return tmp; }
    enum { N = 2 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    RefParam_ () : _ptr(0) {}
    RefParam_ (COE_TA(2, A, &a)) : _ptr(new RefParam2(COE_T(2, a))) {}
private:
    RefParam*   _ptr;
};

template<class ARGS>
class RefParam_<ARGS, 3> {
public:
    operator RefParam* () { return _ptr; }
    RefParam*    clear () { RefParam* tmp = _ptr; _ptr = 0; return tmp; }
    enum { N = 3 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    RefParam_ () : _ptr(0) {}
    RefParam_ (COE_TA(3, A, &a)) : _ptr(new RefParam3(COE_T(3, a))) {}
private:
    RefParam*   _ptr;
};

template<class ARGS>
class RefParam_<ARGS, 4> {
public:
    operator RefParam* () { return _ptr; }
    RefParam*    clear () { RefParam* tmp = _ptr; _ptr = 0; return tmp; }
    enum { N = 4 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    RefParam_ () : _ptr(0) {}
    RefParam_ (COE_TA(4, A, &a)) : _ptr(new RefParam4(COE_T(4, a))) {}
private:
    RefParam*   _ptr;
};

template<class ARGS>
class RefParam_<ARGS, 5> {
public:
    operator RefParam* () { return _ptr; }
    RefParam*    clear () { RefParam* tmp = _ptr; _ptr = 0; return tmp; }
    enum { N = 5 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    RefParam_ () : _ptr(0) {}
    RefParam_ (COE_TA(5, A, &a)) : _ptr(new RefParam5(COE_T(5, a))) {}
private:
    RefParam*   _ptr;
};

template<class ARGS>
class RefParam_<ARGS, 6> {
public:
    operator RefParam* () { return _ptr; }
    RefParam*    clear () { RefParam* tmp = _ptr; _ptr = 0; return tmp; }
    enum { N = 6 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    RefParam_ () : _ptr(0) {}
    RefParam_ (COE_TA(6, A, &a)) : _ptr(new RefParam6(COE_T(6, a))) {}
private:
    RefParam*   _ptr;
};

template<class ARGS>
class RefParam_<ARGS, 7> {
public:
    operator RefParam* () { return _ptr; }
    RefParam*    clear () { RefParam* tmp = _ptr; _ptr = 0; return tmp; }
    enum { N = 7 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    typedef typename Nth<6, ARGS>::type A7;
    RefParam_ () : _ptr(0) {}
    RefParam_ (COE_TA(7, A, &a)) : _ptr(new RefParam7(COE_T(7, a))) {}
private:
    RefParam*   _ptr;
};

template<class ARGS>
class RefParam_<ARGS, 8> {
public:
    operator RefParam* () { return _ptr; }
    RefParam*    clear () { RefParam* tmp = _ptr; _ptr = 0; return tmp; }
    enum { N = 8 };
    typedef typename Nth<0, ARGS>::type A1;
    typedef typename Nth<1, ARGS>::type A2;
    typedef typename Nth<2, ARGS>::type A3;
    typedef typename Nth<3, ARGS>::type A4;
    typedef typename Nth<4, ARGS>::type A5;
    typedef typename Nth<5, ARGS>::type A6;
    typedef typename Nth<6, ARGS>::type A7;
    typedef typename Nth<7, ARGS>::type A8;
    RefParam_ () : _ptr(0) {}
    RefParam_ (COE_TA(8, A, &a)) : _ptr(new RefParam8(COE_T(8, a))) {}
private:
    RefParam*   _ptr;
};

template<class ARGS>
class RefParam_<ARGS, 9> {
public:
    operator RefParam* () { return _ptr; }
    RefParam*    clear () { RefParam* tmp = _ptr; _ptr = 0; return tmp; }
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
    RefParam_ () : _ptr(0) {}
    RefParam_ (COE_TA(9, A, &a)) : _ptr(new RefParam9(COE_T(9, a))) {}
private:
    RefParam*   _ptr;
};

// ===========================================================================
// vparam (p1[, ...p9])
// ===========================================================================

template<COE_T(1, class P)>
ValParam_<typename List1<COE_T(1, P)>::type>
vparam (COE_TA(1, const P, &p))
    { return new ValParamA<typename List1<COE_T(1, P)>::type>(COE_T(1, p)); }

template<COE_T(2, class P)>
ValParam_<typename List2<COE_T(2, P)>::type>
vparam (COE_TA(2, const P, &p))
    { return new ValParamA<typename List2<COE_T(2, P)>::type>(COE_T(2, p)); }

template<COE_T(3, class P)>
ValParam_<typename List3<COE_T(3, P)>::type>
vparam (COE_TA(3, const P, &p))
    { return new ValParamA<typename List3<COE_T(3, P)>::type>(COE_T(3, p)); }

template<COE_T(4, class P)>
ValParam_<typename List4<COE_T(4, P)>::type>
vparam (COE_TA(4, const P, &p))
    { return new ValParamA<typename List4<COE_T(4, P)>::type>(COE_T(4, p)); }

template<COE_T(5, class P)>
ValParam_<typename List5<COE_T(5, P)>::type>
vparam (COE_TA(5, const P, &p))
    { return new ValParamA<typename List5<COE_T(5, P)>::type>(COE_T(5, p)); }

template<COE_T(6, class P)>
ValParam_<typename List6<COE_T(6, P)>::type>
vparam (COE_TA(6, const P, &p))
    { return new ValParamA<typename List6<COE_T(6, P)>::type>(COE_T(6, p)); }

template<COE_T(7, class P)>
ValParam_<typename List7<COE_T(7, P)>::type>
vparam (COE_TA(7, const P, &p))
    { return new ValParamA<typename List7<COE_T(7, P)>::type>(COE_T(7, p)); }

template<COE_T(8, class P)>
ValParam_<typename List8<COE_T(8, P)>::type>
vparam (COE_TA(8, const P, &p))
    { return new ValParamA<typename List8<COE_T(8, P)>::type>(COE_T(8, p)); }

template<COE_T(9, class P)>
ValParam_<typename List9<COE_T(9, P)>::type>
vparam (COE_TA(9, const P, &p))
    { return new ValParamA<typename List9<COE_T(9, P)>::type>(COE_T(9, p)); }

// ===========================================================================
// rparam (p1[, ...p9])
// ===========================================================================

template<COE_T(1, class P)>
RefParam_<typename List1<COE_T(1, P)>::type>
rparam (COE_TA(1, P, &p))
    { return RefParam_<typename List1<COE_T(1, P)>::type>(COE_T(1, p)); }

template<COE_T(2, class P)>
RefParam_<typename List2<COE_T(2, P)>::type>
rparam (COE_TA(2, P, &p))
    { return RefParam_<typename List2<COE_T(2, P)>::type>(COE_T(2, p)); }

template<COE_T(3, class P)>
RefParam_<typename List3<COE_T(3, P)>::type>
rparam (COE_TA(3, P, &p))
    { return RefParam_<typename List3<COE_T(3, P)>::type>(COE_T(3, p)); }

template<COE_T(4, class P)>
RefParam_<typename List4<COE_T(4, P)>::type>
rparam (COE_TA(4, P, &p))
    { return RefParam_<typename List4<COE_T(4, P)>::type>(COE_T(4, p)); }

template<COE_T(5, class P)>
RefParam_<typename List5<COE_T(5, P)>::type>
rparam (COE_TA(5, P, &p))
    { return RefParam_<typename List5<COE_T(5, P)>::type>(COE_T(5, p)); }

template<COE_T(6, class P)>
RefParam_<typename List6<COE_T(6, P)>::type>
rparam (COE_TA(6, P, &p))
    { return RefParam_<typename List6<COE_T(6, P)>::type>(COE_T(6, p)); }

template<COE_T(7, class P)>
RefParam_<typename List7<COE_T(7, P)>::type>
rparam (COE_TA(7, P, &p))
    { return RefParam_<typename List7<COE_T(7, P)>::type>(COE_T(7, p)); }

template<COE_T(8, class P)>
RefParam_<typename List8<COE_T(8, P)>::type>
rparam (COE_TA(8, P, &p))
    { return RefParam_<typename List8<COE_T(8, P)>::type>(COE_T(8, p)); }

template<COE_T(9, class P)>
RefParam_<typename List9<COE_T(9, P)>::type>
rparam (COE_TA(9, P, &p))
    { return RefParam_<typename List9<COE_T(9, P)>::type>(COE_T(9, p)); }

// ===========================================================================
// handler (obj, memfun)
// ===========================================================================

template<class Obj> inline
Handler0 handler (Obj& obj, void (Obj::*fun)(Kernel&))
    { return Handler0(obj, fun); }

template<class Obj, COE_T(1, class P)> inline
Handler_<typename List1<COE_T(1, P)>::type>
handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(1, P, &p)))
    { return Handler_<typename List1<COE_T(1, P)>::type>(obj, fun); }

template<class Obj, COE_T(2, class P)> inline
Handler_<typename List2<COE_T(2, P)>::type>
handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(2, P, &p)))
    { return Handler_<typename List2<COE_T(2, P)>::type>(obj, fun); }

template<class Obj, COE_T(3, class P)> inline
Handler_<typename List3<COE_T(3, P)>::type>
handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(3, P, &p)))
    { return Handler_<typename List3<COE_T(3, P)>::type>(obj, fun); }

template<class Obj, COE_T(4, class P)> inline
Handler_<typename List4<COE_T(4, P)>::type>
handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(4, P, &p)))
    { return Handler_<typename List4<COE_T(4, P)>::type>(obj, fun); }

template<class Obj, COE_T(5, class P)> inline
Handler_<typename List5<COE_T(5, P)>::type>
handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(5, P, &p)))
    { return Handler_<typename List5<COE_T(5, P)>::type>(obj, fun); }

template<class Obj, COE_T(6, class P)> inline
Handler_<typename List6<COE_T(6, P)>::type>
handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(6, P, &p)))
    { return Handler_<typename List6<COE_T(6, P)>::type>(obj, fun); }

template<class Obj, COE_T(7, class P)> inline
Handler_<typename List7<COE_T(7, P)>::type>
handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(7, P, &p)))
    { return Handler_<typename List7<COE_T(7, P)>::type>(obj, fun); }

template<class Obj, COE_T(8, class P)> inline
Handler_<typename List8<COE_T(8, P)>::type>
handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(8, P, &p)))
    { return Handler_<typename List8<COE_T(8, P)>::type>(obj, fun); }

template<class Obj, COE_T(9, class P)> inline
Handler_<typename List9<COE_T(9, P)>::type>
handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(9, P, &p)))
    { return Handler_<typename List9<COE_T(9, P)>::type>(obj, fun); }

// ---------------------------------------------------------------------------
// handler (fun)
// ---------------------------------------------------------------------------

inline
Handler0 handler (void (*fun)(Kernel&))
    { return Handler0(fun); }

template<COE_T(1, class P)> inline
Handler_<typename List1<COE_T(1, P)>::type>
handler (void (*fun)(Kernel&, COE_TA(1, P, &p)))
    { return Handler_<typename List1<COE_T(1, P)>::type>(fun); }

template<COE_T(2, class P)> inline
Handler_<typename List2<COE_T(2, P)>::type>
handler (void (*fun)(Kernel&, COE_TA(2, P, &p)))
    { return Handler_<typename List2<COE_T(2, P)>::type>(fun); }

template<COE_T(3, class P)> inline
Handler_<typename List3<COE_T(3, P)>::type>
handler (void (*fun)(Kernel&, COE_TA(3, P, &p)))
    { return Handler_<typename List3<COE_T(3, P)>::type>(fun); }

template<COE_T(4, class P)> inline
Handler_<typename List4<COE_T(4, P)>::type>
handler (void (*fun)(Kernel&, COE_TA(4, P, &p)))
    { return Handler_<typename List4<COE_T(4, P)>::type>(fun); }

template<COE_T(5, class P)> inline
Handler_<typename List5<COE_T(5, P)>::type>
handler (void (*fun)(Kernel&, COE_TA(5, P, &p)))
    { return Handler_<typename List5<COE_T(5, P)>::type>(fun); }

template<COE_T(6, class P)> inline
Handler_<typename List6<COE_T(6, P)>::type>
handler (void (*fun)(Kernel&, COE_TA(6, P, &p)))
    { return Handler_<typename List6<COE_T(6, P)>::type>(fun); }

template<COE_T(7, class P)> inline
Handler_<typename List7<COE_T(7, P)>::type>
handler (void (*fun)(Kernel&, COE_TA(7, P, &p)))
    { return Handler_<typename List7<COE_T(7, P)>::type>(fun); }

template<COE_T(8, class P)> inline
Handler_<typename List8<COE_T(8, P)>::type>
handler (void (*fun)(Kernel&, COE_TA(8, P, &p)))
    { return Handler_<typename List8<COE_T(8, P)>::type>(fun); }

template<COE_T(9, class P)> inline
Handler_<typename List9<COE_T(9, P)>::type>
handler (void (*fun)(Kernel&, COE_TA(9, P, &p)))
    { return Handler_<typename List9<COE_T(9, P)>::type>(fun); }

// ===========================================================================

} ///// namespace coe

#endif

