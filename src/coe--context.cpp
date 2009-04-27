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
    pfx_tab(NULL),
    pfx_len(0)
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
    pfx_tab(NULL),
    pfx_len(0)
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
    pfx_tab(NULL),
    pfx_len(0)
{
    session->_kernel->_current_context = this;
}

// ---------------------------------------------------------------------------

ExecuteContext::~ExecuteContext ()
{
    if (NULL != session) {
        session->_kernel->_current_context = parent;
    }
}

// ---------------------------------------------------------------------------

bool ExecuteContext::execute (EvCtx& ctx, const ArgTV* xA, int xN, EventArg* arg)
{
    pfx_tab = xA;
    pfx_len = xN;

    if (NULL == handler) {
        cerr << "! ERROR: handler not found\n";
        _print_stack(cerr);
        cerr << flush;
        return false;
    }

    if (! handler->execute(ctx, xA, xN, arg)) {
        cerr << "! ERROR: type mismatch\n";
        _print_stack(cerr);
        cerr << flush;
        return false;
    }

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

