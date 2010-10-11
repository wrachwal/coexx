// coe-kernel--cb.cpp

/*****************************************************************************
Copyright (c) 2008-2010 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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
#include "coe-kernel--r4k.h"
#include "coe-session--r4s.h"
#include "coe-thread--d4t.h"
#include "coe--errno.h"

#include <memory>       // auto_ptr

using namespace std;
using namespace coe;

// ===========================================================================
// Callback

Callback::Callback (SiD target, const CoeStr& evname, ValParam* prefix)
  : _target(target),
    _evname(evname),
    _prefix(prefix)
{
}

Callback::~Callback ()
{
    if (NULL != _prefix) {
        _prefix->destroy();
        _prefix = NULL; // just in case
    }
}

// ---------------------------------------------------------------------------

bool Callback::call (Kernel& kernel, EventArg* arg)
{
    auto_ptr<EventArg>  __arg(arg);
    r4Kernel*  r4k = kernel._r4kernel;
    if (   ! kernel_attached(r4k)
        || ! kernel_equal(r4k, _target))
    {
        return false;
    }
    return r4k->call__arg(_target, _evname, _prefix, arg);
}

// ------------------------------------

bool Callback::call_keep_arg (Kernel& kernel, EventArg& arg)
{
    r4Kernel*  r4k = kernel._r4kernel;
    if (   ! kernel_attached(r4k)
        || ! kernel_equal(r4k, _target))
    {
        return false;
    }
    return r4k->call__arg(_target, _evname, _prefix, &arg);
}

// ---------------------------------------------------------------------------

bool Callback::post (Kernel& kernel, ValParam* vp)
{
    r4Kernel*             r4k = kernel._r4kernel;

    if (! kernel_attached(r4k)) {
        delete vp;
        return false;
    }
    assert(NULL != r4k->_current_context);
    assert(NULL != r4k->_current_context->session);

    EvMsg*  evmsg = new EvMsg(_evname, vp, *r4k->_current_context);

    if (NULL != _prefix) {
        evmsg->pfx(_prefix->clone());
    }

    evmsg = d4Thread::post_event(r4k, _target, evmsg);

    if (NULL != evmsg) {
        delete evmsg;
        return false;
    }
    else {
        return true;
    }
}

// ------------------------------------

bool Callback::post (Kernel& kernel, auto_ptr<ValParam>& vp)
{
    r4Kernel*             r4k = kernel._r4kernel;

    if (! kernel_attached(r4k)) {
        return false;
    }
    assert(NULL != r4k->_current_context);
    assert(NULL != r4k->_current_context->session);

    EvMsg*  evmsg = new EvMsg(_evname, vp.release(), *r4k->_current_context);

    if (NULL != _prefix) {
        evmsg->pfx(_prefix->clone());
    }

    evmsg = d4Thread::post_event(r4k, _target, evmsg);

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

bool Callback::anon_post (ValParam* vp)
{
    EvMsg*  evmsg = new EvMsg(_evname, vp);

    if (NULL != _prefix) {
        evmsg->pfx(_prefix->clone());
    }

    evmsg = d4Thread::post_event(NULL/*source-kernel*/, _target, evmsg);

    if (NULL != evmsg) {
        delete evmsg;
        return false;
    }
    else {
        return true;
    }
}

// ------------------------------------

bool Callback::anon_post (auto_ptr<ValParam>& vp)
{
    EvMsg*  evmsg = new EvMsg(_evname, vp.release());

    if (NULL != _prefix) {
        evmsg->pfx(_prefix->clone());
    }

    evmsg = d4Thread::post_event(NULL/*source-kernel*/, _target, evmsg);

    if (NULL != evmsg) {
        vp.reset(evmsg->arg_change(NULL));
        delete evmsg;
        return false;
    }
    else {
        return true;
    }
}

