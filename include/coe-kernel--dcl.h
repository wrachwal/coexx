// $Id$

/*************************************************************************
Copyright (c) 2008 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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
*************************************************************************/

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

class PostArg : public EventArg {}; //TODO: add cloning capability
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

