// coe-kernel--imp.cpp

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

#include "coe-kernel.h"
#include "coe--util.h"

#include <iostream> // debug
#include <cassert>

using namespace std;
using namespace coe;

// ===========================================================================
// TimeSpec

TimeSpec::TimeSpec (double sec)
{
    tv_sec  = time_t(sec);
    tv_nsec = long((sec - double(tv_sec)) * 1e9);
    if (tv_nsec < 0) {
        tv_sec --;
        tv_nsec += 1000000000L;
    }
}

int TimeSpec::compare (const TimeSpec& rhs) const
{
    if (    tv_sec < rhs.tv_sec)
        return -1;
    if (rhs.tv_sec <     tv_sec)
        return +1;

    if (    tv_nsec < rhs.tv_nsec)
        return -1;
    if (rhs.tv_nsec <     tv_nsec)
        return +1;

    return 0;
}

TimeSpec& TimeSpec::operator+= (const TimeSpec& add)
{
    tv_sec  += add.tv_sec;
    tv_nsec += add.tv_nsec;

    if (tv_nsec >= 1000000000L) {
        tv_sec ++;
        tv_nsec -= 1000000000L;
    }
    return *this;
}

TimeSpec& TimeSpec::operator-= (const TimeSpec& sub)
{
    tv_sec  -= sub.tv_sec;
    tv_nsec -= sub.tv_nsec;

    if (tv_nsec < 0) {
        tv_sec --;
        tv_nsec += 1000000000L;
    }
    return *this;
}

ostream& coe::operator<< (ostream& os, const TimeSpec& ts)
{
    return os << ts.tv_sec + 1e-9 * ts.tv_nsec;
}

// ===========================================================================
// _TypeD

const _TypeD* _TypeD::_register (_TypeD* type)
{
    static _TypeD*  head = NULL;

    if (NULL != type) {
        assert(NULL == type->next);
        type->pos  = head ? head->pos + 1 : 0;
        type->next = head;
        head = type;
    }

    return head;
}

// ===========================================================================
// _TypeDN

const _TypeDN* _TypeDN::_register (_TypeDN* type)
{
    static _TypeDN* head = NULL;

    if (NULL != type) {
        assert(NULL == type->next);
        type->pos  = head ? head->pos + 1 : 0;
        type->next = head;
        head = type;
    }

    return head;
}

// ------------------------------------

bool coe::syntax_check (const _TypeDN* hT, const _TypeDN* xT, const _TypeDN* aT)
{
    if (NULL == xT) {
        return (hT == aT);
    }
    if (NULL == aT) {
        return (hT == xT);
    }
    if (NULL == hT || hT->len != xT->len + aT->len) {
        return false;
    }

    size_t i = 0;
    while (i < xT->len) {
        if (hT->info[i] != xT->info[i]) {
            return false;
        }
        ++i;
    }
    for (size_t j = 0; j < aT->len; ++j, ++i) {
        if (hT->info[i] != aT->info[j]) {
            return false;
        }
    }
    return true;
}

// ===========================================================================
// EventArg

EventArg::~EventArg ()
{
}

// ===========================================================================
// StateCmd

StateCmd::~StateCmd ()
{
}

// ===========================================================================
// MFunCmd0 ... MFunCmd5

void MFunCmd0::execute (EvCtx& ctx, void* arg[]) const
{
    (_obj->*_memfun)(ctx);
}

void MFunCmd1::execute (EvCtx& ctx, void* arg[]) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)arg[0]
    );
}

void MFunCmd2::execute (EvCtx& ctx, void* arg[]) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)arg[0],
        *(_Arg*)arg[1]
    );
}

void MFunCmd3::execute (EvCtx& ctx, void* arg[]) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)arg[0],
        *(_Arg*)arg[1],
        *(_Arg*)arg[2]
    );
}

void MFunCmd4::execute (EvCtx& ctx, void* arg[]) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)arg[0],
        *(_Arg*)arg[1],
        *(_Arg*)arg[2],
        *(_Arg*)arg[3]
    );
}

void MFunCmd5::execute (EvCtx& ctx, void* arg[]) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)arg[0],
        *(_Arg*)arg[1],
        *(_Arg*)arg[2],
        *(_Arg*)arg[3],
        *(_Arg*)arg[4]
    );
}

// ===========================================================================
// GFunCmd0 ... GFunCmd5

void GFunCmd0::execute (EvCtx& ctx, void* arg[]) const
{
    (*_fun)(ctx);
}

void GFunCmd1::execute (EvCtx& ctx, void* arg[]) const
{
    (*_fun)(
        ctx,
        *(_Arg*)arg[0]
    );
}

void GFunCmd2::execute (EvCtx& ctx, void* arg[]) const
{
    (*_fun)(
        ctx,
        *(_Arg*)arg[0],
        *(_Arg*)arg[1]
    );
}

void GFunCmd3::execute (EvCtx& ctx, void* arg[]) const
{
    (*_fun)(
        ctx,
        *(_Arg*)arg[0],
        *(_Arg*)arg[1],
        *(_Arg*)arg[2]
    );
}

void GFunCmd4::execute (EvCtx& ctx, void* arg[]) const
{
    (*_fun)(
        ctx,
        *(_Arg*)arg[0],
        *(_Arg*)arg[1],
        *(_Arg*)arg[2],
        *(_Arg*)arg[3]
    );
}

void GFunCmd5::execute (EvCtx& ctx, void* arg[]) const
{
    (*_fun)(
        ctx,
        *(_Arg*)arg[0],
        *(_Arg*)arg[1],
        *(_Arg*)arg[2],
        *(_Arg*)arg[3],
        *(_Arg*)arg[4]
    );
}

// ---------------------------------------------------------------------------

StateCmd* handler (void (*fun)(EvCtx&))
{
    return new GFunCmd0(fun);
}

