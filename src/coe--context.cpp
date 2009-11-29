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
:   magic(this),
    parent(kernel->_current_context),
    level(0),
    type(EventContext::INIT),
    session(NULL),
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
                                EventContext::Type type,
                                const string& state,
                                r4Session* stopper)
:   magic(this),
    parent(session->_kernel->_current_context),
    level(parent->level + 1),
    type(type),
    session(session),
    state(state),
    pfx_type(NULL),
    pfx_pval(NULL),
    pfx_locked(NULL),
    arg(NULL),
    arg_locked(NULL)
{
    session->_kernel->_current_context = this;

    switch (type) {

        case EventContext::START:
            if (NULL != session->_parent) {
                sender       = parent->session->_sid;
                sender_state = parent->state;
            }
            else {  // kernel session
                sender = session->_sid;     // itself
                assert(sender.id() == 1);
            }
            break;

        case EventContext::STOP:
            sender = stopper->_sid;
            if (stopper == parent->session) {
                sender_state = parent->state;
            }
            break;

        case EventContext::CALL:
            sender       = parent->session->_sid;
            sender_state = parent->state;
            break;

        default:
            assert(0);
            break;
    }
}

// ------------------------------------

ExecuteContext::ExecuteContext (r4Session* session,
                                EvUser* event)
:   magic(this),
    parent(NULL),
    level(0),
    type(event->event_type()),
    session(session),
    state(event->name()),
    sender_state(event->sender_state()),
    pfx_type(NULL),
    pfx_pval(NULL),
    pfx_locked(NULL),
    arg(NULL),
    arg_locked(NULL)
{
    assert(&session->_kernel->_kernel_session_context
        == session->_kernel->       _current_context);

    session->_kernel->_current_context = this;

    switch (type) {

        case EventContext::POST:
            sender = static_cast<EvMsg*>(event)->sender();
            break;

        case EventContext::ALARM:
            sender   = session->_sid;
            alarm_id = static_cast<EvAlarm*>(event)->aid();
            break;

        case EventContext::SELECT:
            sender = session->_sid;
            break;

        default:
            assert(0);
            break;
    }
}

// ---------------------------------------------------------------------------

ExecuteContext::~ExecuteContext ()
{
    magic = NULL;

    if (NULL != session) {

        if (NULL != parent) {
            session->_kernel->_current_context = parent;
        }
        else {
            assert(0 == level);
            session->_kernel->_current_context =
                &session->_kernel->_kernel_session_context;
        }
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

bool ExecuteContext::execute (Kernel& kernel, const HandlerX& handler)
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

    if (! handler) {
        cerr << "! ERROR: handler not found\n";
        _print_stack(cerr);
        cerr << flush;
        return false;
    }

    const _TypeDN* hT = handler.par_type();

    if (! syntax_check(hT, pfx_type, aT)) {
        cerr << "! ERROR: type mismatch\n";
        _print_stack(cerr);
        cerr << flush;
        return false;
    }

    handler.execute(kernel, argptr);

    return true;
}

// ---------------------------------------------------------------------------

void ExecuteContext::_print_stack (ostream& os)
{
    for (ExecuteContext* ctx = this; ctx != NULL; ctx = ctx->parent) {
        ctx->_print_frame(os);
    }
}

// ------------------------------------

#define TAR_EV  (1<<0)
#define SRC_EV  (1<<1)
#define SRC_SE  (1<<2)

void ExecuteContext::_print_frame (ostream& os)
{
    const char* et = "";
    int         ff = 0;

    switch (type) {

        case EventContext::INIT:
            et = "INIT";
            ff |= (state.empty() ? 0 : TAR_EV);
            break;

        case EventContext::START:
            et = "START";
            break;

        case EventContext::STOP:
            et = "STOP";
            ff |= (sender == parent->session->_sid ? 0 : SRC_SE);
            break;

        case EventContext::CALL:
            et = "CALL";
            ff |= TAR_EV;
            break;

        case EventContext::POST:
            ff |= TAR_EV | SRC_EV | (session->_sid == sender ? 0 : SRC_SE);
            et = (ff & SRC_SE) ? "POST" : "YIELD";
            break;

        case EventContext::ALARM:
            et = "ALARM";
            ff |= TAR_EV | SRC_EV;
            break;

        case EventContext::SELECT:
            et = "SELECT";
            ff |= TAR_EV | SRC_EV;
            break;

        case EventContext::SIGNAL:
            et = "SIGNAL";
            ff |= TAR_EV | SRC_EV;
            break;
    }

    os << '#' << level << ' ' << et << ' ' << session->_sid;

    if (ff & TAR_EV)
        os << " at (" << state << ')';

    if (ff & (SRC_SE | SRC_EV)) {

        os << " <-";

        if (ff & SRC_SE) {
            os << ' ' << sender;
            if (ff & SRC_EV)
                os << " at";
        }

        if (ff & SRC_SE)
            os << " (" << sender_state << ')';
    }

    os << '\n';

    //TODO: print arguments
}

