// coe-kernel--dcl.h

#if !defined(__COE_KERNEL_H) || defined(__COE_KERNEL__DCL_H)
#error "coe-kernel--dcl.h is a private header!!!"
#endif

#ifndef __COE_KERNEL__DCL_H
#define __COE_KERNEL__DCL_H

#include <string>
#include <typeinfo>

class EventArg;
    class PostArg;
    //  class PostArg1<A1> ... PostArg5<A1 .. A5>
    class CallArg;
    //  class CallArg1     ... CallArg5
class StateCmd;
    //  class MFunCmd1 ... MFunCmd5
    //  class GFunCmd1 ... GFunCmd5

class EvCtx;

// -----------------------------------------------------------------------
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

// =======================================================================
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
// PostArg
// CallArg

class PostArg : public EventArg {};
class CallArg : public EventArg {};

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

// =======================================================================
// StateCmd

class StateCmd {
public:
    //
    // static operator new/delete...
    //
    virtual ~StateCmd ();
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

// =======================================================================

#endif

