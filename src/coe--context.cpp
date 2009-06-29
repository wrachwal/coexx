// coe--context.cpp

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

#include "coe--context.h"
#include "coe-kernel--r4k.h"
#include "coe-session--r4s.h"

#include <iostream>

using namespace std;
using namespace coe;

// ===========================================================================
// ExecuteContext

ExecuteContext::ExecuteContext (r4Kernel* kernel)
:   parent(kernel->_current_context),
    level(0),
    session(NULL),
    event(NULL),
    handler(NULL),
    pfx_type(NULL),
    pfx_pval(NULL),
    pfx_locked(NULL),
    arg(NULL),
    arg_locked(NULL)
{
    kernel->_current_context = this;
}

// ------------------------------------

ExecuteContext::ExecuteContext (r4Session* session,
                                const string& state,
                                StateCmd* handler)
:   parent(session->_kernel->_current_context),
    level(parent->level + 1),
    session(session),
    event(NULL),
    state(state),
    handler(handler),
    pfx_type(NULL),
    pfx_pval(NULL),
    pfx_locked(NULL),
    arg(NULL),
    arg_locked(NULL)
{
    session->_kernel->_current_context = this;
}

// ------------------------------------

ExecuteContext::ExecuteContext (r4Session* session,
                                EvUser* event,
                                StateCmd* handler)
:   parent(session->_kernel->_current_context),
    level(parent->level + 1),
    session(session),
    event(event),
    state(event->name()),
    handler(handler),
    pfx_type(NULL),
    pfx_pval(NULL),
    pfx_locked(NULL),
    arg(NULL),
    arg_locked(NULL)
{
    session->_kernel->_current_context = this;
}

// ---------------------------------------------------------------------------

ExecuteContext::~ExecuteContext ()
{
    if (NULL != session) {
        session->_kernel->_current_context = parent;
    }

    if (NULL != pfx_locked) {
        if (pfx_locked->_locked > 0) {
            -- pfx_locked->_locked;
        }
        else {
            assert(pfx_locked->_locked < 0);
            if (0 == ++ pfx_locked->_locked) {
                delete pfx_locked;
            }
        }
    }

    if (NULL != arg_locked) {
        if (arg_locked->_locked > 0) {
            -- arg_locked->_locked;
        }
        else {
            assert(arg_locked->_locked < 0);
            if (0 == ++ arg_locked->_locked) {
                delete arg_locked;
            }
        }
    }
}

// ---------------------------------------------------------------------------

void ExecuteContext::prefix (const _TypeDN* type, void* pval[])
{
    assert(NULL == pfx_pval);
    pfx_type = type;
    pfx_pval = pval;
}

void ExecuteContext::locked_prefix (ValParam* vp)
{
    assert(NULL == pfx_pval);
    if (NULL != vp) {
        pfx_locked = vp;
        pfx_type = vp->arg_type();
        pfx_pval = vp->arg_list();
        vp->_locked += vp->_locked < 0 ? -1 : +1;
    }
}

// ------------------------------------

void ExecuteContext::argument (EventArg* ea)
{
    assert(NULL == arg);
    arg = ea;
}

void ExecuteContext::locked_argument (ValParam* vp)
{
    assert(NULL == arg);
    if (NULL != vp) {
        arg = arg_locked = vp;
        vp->_locked += vp->_locked < 0 ? -1 : +1;
    }
}

// ---------------------------------------------------------------------------

bool ExecuteContext::execute (EvCtx& ctx)
{
    //
    // argptr[]
    //
    size_t  len = pfx_type ? pfx_type->len : 0;

    if (len) {
        copy(pfx_pval, pfx_pval + len, argptr);
    }

    const _TypeDN*  aT = arg ? arg->arg_type() : NULL;

    if (arg) {
        void**  aV = arg->arg_list();
        copy(aV, aV + aT->len, argptr + len);
        len += aT->len;
    }

    if (NULL == handler) {
        cerr << "! ERROR: handler not found\n";
        _print_stack(cerr);
        cerr << flush;
        return false;
    }

    const _TypeDN* hT = handler->par_type();

    if (! syntax_check(hT, pfx_type, aT)) {
        cerr << "! ERROR: type mismatch\n";
        _print_stack(cerr);
        cerr << flush;
        return false;
    }

    handler->execute(ctx, argptr);

    return true;
}

// ---------------------------------------------------------------------------

void ExecuteContext::_print_stack (ostream& os)
{
    for (ExecuteContext* ctx = this; ctx && ctx->parent; ctx = ctx->parent) {
        ctx->_print_frame(os);
    }
}

// ------------------------------------

void ExecuteContext::_print_frame (ostream& os)
{
    os << '#' << level << ' ';
    if (event)
        event->describe(os);
    else
        os << session->_sid << " at (" << state << ")";
    os << '\n';

    //TODO: print arguments
}

