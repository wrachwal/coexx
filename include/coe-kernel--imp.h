// coe-kernel--imp.h

#if !defined(__COE_KERNEL_H) || defined(__COE_KERNEL__IMP_H)
#error "coe-kernel--imp.h is a private header!!!"
#endif

#ifndef __COE_KERNEL__IMP_H
#define __COE_KERNEL__IMP_H

// =======================================================================
// PostArg1<A1> ... PostArg5<A1 .. A5>
// =======================================================================

template<class A1>
class PostArg1 : public EventArg_N<PostArg, 1> {
public:
    PostArg1 (const A1& a1)
        : _a1(const_cast<A1&>(a1))  //XXX: temp. trick to make auto_ptr happy
        {
            _arg[0].set(&typeid(A1), &_a1);
        }
private:
    A1  _a1;
};

// ------------------------------------

template<class A1, class A2>
class PostArg2 : public EventArg_N<PostArg, 2> {
public:
    PostArg2 (const A1& a1, const A2& a2)
        : _a1(a1), _a2(a2)
        {
            _arg[0].set(&typeid(A1), &_a1);
            _arg[1].set(&typeid(A2), &_a2);
        }
private:
    A1  _a1;
    A2  _a2;
};

// ------------------------------------

template<class A1, class A2, class A3>
class PostArg3 : public EventArg_N<PostArg, 3> {
public:
    PostArg3 (const A1& a1, const A2& a2, const A3& a3)
        : _a1(a1), _a2(a2), _a3(a3)
        {
            _arg[0].set(&typeid(A1), &_a1);
            _arg[1].set(&typeid(A2), &_a2);
            _arg[2].set(&typeid(A3), &_a3);
        }
private:
    A1  _a1;
    A2  _a2;
    A3  _a3;
};

// ------------------------------------

template<class A1, class A2, class A3, class A4>
class PostArg4 : public EventArg_N<PostArg, 4> {
public:
    PostArg4 (const A1& a1, const A2& a2, const A3& a3, const A4& a4)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4)
        {
            _arg[0].set(&typeid(A1), &_a1);
            _arg[1].set(&typeid(A2), &_a2);
            _arg[2].set(&typeid(A3), &_a3);
            _arg[3].set(&typeid(A4), &_a4);
        }
private:
    A1  _a1;
    A2  _a2;
    A3  _a3;
    A4  _a4;
};

// ------------------------------------

template<class A1, class A2, class A3, class A4, class A5>
class PostArg5 : public EventArg_N<PostArg, 5> {
public:
    PostArg5 (const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
        : _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5)
        {
            _arg[0].set(&typeid(A1), &_a1);
            _arg[1].set(&typeid(A2), &_a2);
            _arg[2].set(&typeid(A3), &_a3);
            _arg[3].set(&typeid(A4), &_a4);
            _arg[4].set(&typeid(A5), &_a5);
        }
private:
    A1  _a1;
    A2  _a2;
    A3  _a3;
    A4  _a4;
    A4  _a5;
};

// =======================================================================
// CallArg1 ... CallArg5
// =======================================================================

class CallArg1 : public EventArg_N<CallArg, 1> {
public:
    template<class A1>
    CallArg1 (A1& a1)
        {
            _arg[0].set(&typeid(A1), &a1);
        }
};

// ------------------------------------

class CallArg2 : public EventArg_N<CallArg, 2> {
public:
    template<class A1, class A2>
    CallArg2 (A1& a1, A2& a2)
        {
            _arg[0].set(&typeid(A1), &a1);
            _arg[1].set(&typeid(A2), &a2);
        }
};

// ------------------------------------

class CallArg3 : public EventArg_N<CallArg, 3> {
public:
    template<class A1, class A2, class A3>
    CallArg3 (A1& a1, A2& a2, A3& a3)
        {
            _arg[0].set(&typeid(A1), &a1);
            _arg[1].set(&typeid(A2), &a2);
            _arg[2].set(&typeid(A3), &a3);
        }
};

// ------------------------------------

class CallArg4 : public EventArg_N<CallArg, 4> {
public:
    template<class A1, class A2, class A3, class A4>
    CallArg4 (A1& a1, A2& a2, A3& a3, A4& a4)
        {
            _arg[0].set(&typeid(A1), &a1);
            _arg[1].set(&typeid(A2), &a2);
            _arg[2].set(&typeid(A3), &a3);
            _arg[3].set(&typeid(A4), &a4);
        }
};

// ------------------------------------

class CallArg5 : public EventArg_N<CallArg, 5> {
public:
    template<class A1, class A2, class A3, class A4, class A5>
    CallArg5 (A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
        {
            _arg[0].set(&typeid(A1), &a1);
            _arg[1].set(&typeid(A2), &a2);
            _arg[2].set(&typeid(A3), &a3);
            _arg[3].set(&typeid(A4), &a4);
            _arg[4].set(&typeid(A5), &a5);
        }
};

// =======================================================================
// MFunCmd0 ... MFunCmd5
// =======================================================================

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

// =======================================================================
// GFunCmd0 .. GFunCmd5
// =======================================================================

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

// =======================================================================
// vparam (p1[, ...])
// =======================================================================

template<class P1>
PostArg* vparam (const P1& p1)
    { return new PostArg1<P1>(p1); }

template<class P1, class P2>
PostArg* vparam (const P1& p1, const P2& p2)
    { return new PostArg2<P1, P2>(p1, p2); }

template<class P1, class P2, class P3>
PostArg* vparam (const P1& p1, const P2& p2, const P3& p3)
    { return new PostArg3<P1, P2, P3>(p1, p2, p3); }

template<class P1, class P2, class P3, class P4>
PostArg* vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4)
    { return new PostArg4<P1, P2, P3, P4>(p1, p2, p3, p4); }

template<class P1, class P2, class P3, class P4, class P5>
PostArg* vparam (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
    { return new PostArg5<P1, P2, P3, P4, P5>(p1, p2, p3, p4, p5); }

// =======================================================================
// rparam (p1[, ...])
// =======================================================================

template<class P1>
CallArg* rparam (P1& p1)
    { return new CallArg1(p1); }

template<class P1, class P2>
CallArg* rparam (P1& p1, P2& p2)
    { return new CallArg2(p1, p2); }

template<class P1, class P2, class P3>
CallArg* rparam (P1& p1, P2& p2, P3& p3)
    { return new CallArg3(p1, p2, p3); }

template<class P1, class P2, class P3, class P4>
CallArg* rparam (P1& p1, P2& p2, P3& p3, P4& p4)
    { return new CallArg4(p1, p2, p3, p4); }

template<class P1, class P2, class P3, class P4, class P5>
CallArg* rparam (P1& p1, P2& p2, P3& p3, P4& p4, P5& p5)
    { return new CallArg5(p1, p2, p3, p4, p5); }

// =======================================================================
// handler (obj, memfun)
// =======================================================================

template<class Obj>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(EvCtx&))
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
StateCmd* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&))
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

// -----------------------------------------------------------------------
// handler (fun)
// -----------------------------------------------------------------------

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

// =======================================================================

#endif

