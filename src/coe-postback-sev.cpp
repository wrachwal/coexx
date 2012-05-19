// coe-postback-sev.cpp

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
// _Sev_Postback

namespace coe {
    ///
    struct _Sev_Postback {
        _Sev_Postback (SiD target, const CoeStr& evname, ValParam* prefix)
        :   refcnt(1)
        ,   target(target)
        ,   evname(evname)
        ,   prefix(prefix)
        {}
        ~_Sev_Postback ()
        {
            if (NULL != prefix) {
                prefix->destroy();
                prefix = NULL;  // just in case
            }
        }
        _AtomIC<size_t> refcnt;
        SiD             target;
        CoeStr          evname;
        ValParam*       prefix;
    };
    ///
    const size_t SIZEOF__sev_postback = sizeof(_Sev_Postback);
}

// ===========================================================================
// Postback

Postback::Postback (SiD target, const CoeStr& evname, ValParam* prefix)
:   _impl(NULL)
{
    _impl = static_cast<_Sev_Postback*>(allocate__sev_postback());
    new (_impl) _Sev_Postback(target, evname, prefix);
}

Postback::Postback (const Postback& rhs)
:   _impl(NULL)
{
    if (this != &rhs && NULL != rhs._impl) {
        ++(_impl = rhs._impl)->refcnt;
    }
}

Postback& Postback::operator= (const Postback& rhs)
{
    if (_impl != rhs._impl) {
        if (NULL != _impl && 0 == --_impl->refcnt) {
            _impl->~_Sev_Postback();
            deallocate__sev_postback(_impl);
        }
        if (NULL != (_impl = rhs._impl)) {
            ++_impl->refcnt;
        }
    }
    return *this;
}

void Postback::reset ()
{
    if (NULL != _impl) {
        if (0 == --_impl->refcnt) {
            _impl->~_Sev_Postback();
            deallocate__sev_postback(_impl);
        }
        _impl = NULL;
    }
}

// ------------------------------------


SiD Postback::session () const
{
    return _impl ? _impl->target : SiD();
}

// ---------------------------------------------------------------------------

bool Postback::post (Kernel& kernel, ValParam* vp)
{
    if (NULL == _impl) {
        delete vp;
        //errno = ???
        return false;
    }

    r4Kernel*             r4k = kernel._r4kernel;
    if (! kernel_attached(r4k)) {
        delete vp;
        return false;
    }
    assert(NULL != r4k->_current_context);
    assert(NULL != r4k->_current_context->session);

    EvMsg*  evmsg = new EvMsg(_impl->evname, vp, *r4k->_current_context);

    if (NULL != _impl->prefix) {
        evmsg->pfx(_impl->prefix->clone());
    }

    evmsg = d4Thread::post_event(r4k, _impl->target, evmsg);

    if (NULL != evmsg) {
        delete evmsg;
        return false;
    }
    else {
        return true;
    }
}

// ------------------------------------

bool Postback::post (Kernel& kernel, auto_ptr<ValParam>& vp)
{
    if (NULL == _impl) {
        //errno = ???
        return false;
    }

    r4Kernel*             r4k = kernel._r4kernel;
    if (! kernel_attached(r4k)) {
        return false;
    }
    assert(NULL != r4k->_current_context);
    assert(NULL != r4k->_current_context->session);

    EvMsg*  evmsg = new EvMsg(_impl->evname, vp.release(), *r4k->_current_context);

    if (NULL != _impl->prefix) {
        evmsg->pfx(_impl->prefix->clone());
    }

    evmsg = d4Thread::post_event(r4k, _impl->target, evmsg);

    if (NULL != evmsg) {
        vp.reset(evmsg->arg_change(NULL));
        delete evmsg;
        return false;
    }
    else {
        return true;
    }
}

// ------------------------------------

bool Postback::anon_post (ValParam* vp)
{
    if (NULL == _impl) {
        delete vp;
        //errno = ???
        return false;
    }

    EvMsg*  evmsg = new EvMsg(_impl->evname, vp);

    if (NULL != _impl->prefix) {
        evmsg->pfx(_impl->prefix->clone());
    }

    evmsg = d4Thread::post_event(NULL/*source-kernel*/, _impl->target, evmsg);

    if (NULL != evmsg) {
        delete evmsg;
        return false;
    }
    else {
        return true;
    }
}

// ------------------------------------

bool Postback::anon_post (auto_ptr<ValParam>& vp)
{
    if (NULL == _impl) {
        //errno = ???
        return false;
    }

    EvMsg*  evmsg = new EvMsg(_impl->evname, vp.release());

    if (NULL != _impl->prefix) {
        evmsg->pfx(_impl->prefix->clone());
    }

    evmsg = d4Thread::post_event(NULL/*source-kernel*/, _impl->target, evmsg);

    if (NULL != evmsg) {
        vp.reset(evmsg->arg_change(NULL));
        delete evmsg;
        return false;
    }
    else {
        return true;
    }
}

