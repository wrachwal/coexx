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

#include "coe-kernel.h"

using namespace std;

// =======================================================================
// TimeSpec

int TimeSpec::compare (const TimeSpec& rhs) const
{
    if (tv_sec < rhs.tv_sec)
        return -1;
    if (rhs.tv_sec < tv_sec)
        return +1;

    if (tv_nsec < rhs.tv_nsec)
        return -1;
    if (rhs.tv_nsec < tv_nsec)
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

// =======================================================================
// MFunCmd0 ... MFunCmd5

ArgTV* MFunCmd0::arg_list (int& len)
{
    len = 0;
    return NULL;
}

void MFunCmd0::_execute (EvCtx& ctx) const
{
    (_obj->*_memfun)(ctx);
}

void MFunCmd1::_execute (EvCtx& ctx) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)_arg[0]._pv
    );
}

void MFunCmd2::_execute (EvCtx& ctx) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv
    );
}

void MFunCmd3::_execute (EvCtx& ctx) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv,
        *(_Arg*)_arg[2]._pv
    );
}

void MFunCmd4::_execute (EvCtx& ctx) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv,
        *(_Arg*)_arg[2]._pv,
        *(_Arg*)_arg[3]._pv
    );
}

void MFunCmd5::_execute (EvCtx& ctx) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv,
        *(_Arg*)_arg[2]._pv,
        *(_Arg*)_arg[3]._pv,
        *(_Arg*)_arg[4]._pv
    );
}

// =======================================================================
// GFunCmd0 ... GFunCmd5

ArgTV* GFunCmd0::arg_list (int& len)
{
    len = 0;
    return NULL;
}

void GFunCmd0::_execute (EvCtx& ctx) const
{
    (*_fun)(ctx);
}

void GFunCmd1::_execute (EvCtx& ctx) const
{
    (*_fun)(
        ctx,
        *(_Arg*)_arg[0]._pv
    );
}

void GFunCmd2::_execute (EvCtx& ctx) const
{
    (*_fun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv
    );
}

void GFunCmd3::_execute (EvCtx& ctx) const
{
    (*_fun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv,
        *(_Arg*)_arg[2]._pv
    );
}

void GFunCmd4::_execute (EvCtx& ctx) const
{
    (*_fun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv,
        *(_Arg*)_arg[2]._pv,
        *(_Arg*)_arg[3]._pv
    );
}

void GFunCmd5::_execute (EvCtx& ctx) const
{
    (*_fun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv,
        *(_Arg*)_arg[2]._pv,
        *(_Arg*)_arg[3]._pv,
        *(_Arg*)_arg[4]._pv
    );
}

// -----------------------------------------------------------------------

StateCmd* handler (void (*fun)(EvCtx&))
{
    return new GFunCmd0(fun);
}

