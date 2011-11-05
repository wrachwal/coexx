// coe-callback-sev.cpp

/*****************************************************************************
Copyright (c) 2008-2011 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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
#include "coe-memory.h"

#include "coe-kernel--r4k.h"
#include "coe-session--r4s.h"
#include "coe-thread--d4t.h"
#include "coe--errno.h"

using namespace std;
using namespace coe;

// ---------------------------------------------------------------------------
// _Sev_Callback

namespace coe {
    ///
    struct _Sev_Callback {
        _Sev_Callback (SiD target, const CoeStr& evname, ValParam* prefix)
        :   refcnt(1)
        ,   target(target)
        ,   evname(evname)
        ,   prefix(prefix)
        {}
        ~_Sev_Callback ()
        {
            delete prefix;
            prefix = NULL;
        }
        _Atomic<size_t> refcnt;
        SiD             target;
        CoeStr          evname;
        ValParam*       prefix;
    };
    ///
    const size_t SIZEOF__sev_callback = sizeof(_Sev_Callback);
}

// ===========================================================================
// Callback

Callback::Callback (SiD target, const CoeStr& evname, ValParam* prefix)
:   _impl(NULL)
{
    _impl = static_cast<_Sev_Callback*>(allocate__sev_callback());
    new (_impl) _Sev_Callback(target, evname, prefix);
}

Callback::Callback (const Callback& rhs)
:   _impl(NULL)
{
    if (this != &rhs && NULL != rhs._impl) {
        ++(_impl = rhs._impl)->refcnt;
    }
}

Callback& Callback::operator= (const Callback& rhs)
{
    if (_impl != rhs._impl) {
        if (NULL != _impl && 0 == --_impl->refcnt) {
            _impl->~_Sev_Callback();
            deallocate__sev_callback(_impl);
        }
        if (NULL != (_impl = rhs._impl)) {
            ++_impl->refcnt;
        }
    }
    return *this;
}

void Callback::reset ()
{
    if (NULL != _impl) {
        if (0 == --_impl->refcnt) {
            _impl->~_Sev_Callback();
            deallocate__sev_callback(_impl);
        }
        _impl = NULL;
    }
}

// ------------------------------------


SiD Callback::session () const
{
    return _impl ? _impl->target : SiD();
}

// ---------------------------------------------------------------------------

bool Callback::call (Kernel& kernel, EventArg* arg)
{
    auto_ptr<EventArg>  __arg(arg);
    if (NULL == _impl) {
        //errno = ???
        return false;
    }
    r4Kernel*  r4k = kernel._r4kernel;
    if (   ! kernel_attached(r4k)
        || ! kernel_equal(r4k, _impl->target))
    {
        return false;
    }
    return r4k->call__arg(_impl->target, _impl->evname, _impl->prefix, arg);
}

// ------------------------------------

bool Callback::call_keep_arg (Kernel& kernel, EventArg& arg)
{
    if (NULL == _impl) {
        //errno = ???
        return false;
    }
    r4Kernel*  r4k = kernel._r4kernel;
    if (   ! kernel_attached(r4k)
        || ! kernel_equal(r4k, _impl->target))
    {
        return false;
    }
    return r4k->call__arg(_impl->target, _impl->evname, _impl->prefix, &arg);
}

