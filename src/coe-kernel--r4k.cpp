// coe-kernel--r4k.cpp

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

#include "coe-kernel--r4k.h"
#include "coe-thread--d4t.h"
#include "coe-kernel--s4k.h"
#include "coe-session.h"
#include "coe-session--r4s.h"

#include <memory>       // auto_ptr
#include <cassert>
#include <iostream>

using namespace std;
using namespace coe;

// ---------------------------------------------------------------------------

namespace {

    class CCScope {
    public:
        CCScope (r4Session* session, const string& state)
            :   _new_ctx(session, state)
            {
                assert(NULL != session);
                assert(NULL != session->_kernel);
                assert(NULL != session->_kernel->_current_context);

                _old_ctx =  session->_kernel->_current_context;
                _new_ctx.parent =                     _old_ctx;
                session->_kernel->_current_context = &_new_ctx;
            }

        ~CCScope ()
            {
                _new_ctx.session->_kernel->_current_context = _old_ctx;
            }

    private:
        SessionContext* _old_ctx;
        SessionContext  _new_ctx;
    };

}

// ---------------------------------------------------------------------------
// d4Thread::Local

r4Session* r4Kernel::Local::find_session (SiD sid) const
{
    Sid_Map::const_iterator i = sid_map.find(sid);
    return i == sid_map.end() ? NULL : (*i).second;
}

// ===========================================================================

r4Kernel::r4Kernel ()
{
    _handle = NULL;

    // trying attach kernel to current thread
    _thread = d4Thread::get_tls_data();

    if (NULL == _thread) {  // thread's event loop has not been run yet
        _thread = new d4Thread;
        _thread->_os_thread = pthread_self();
        d4Thread::set_tls_data(_thread);
        _thread->allocate_tid();            // --@@--
    }

    d4Thread::allocate_kid(*this);          // --@@--

    // once `kid' is known, setup sid_generator
    _sid_generator = IdentGenerator<SiD>(SiD(_kid, 0));

    //
    // create and start the kernel session
    //
    _current_context = &_kernel_session_context;

    _s4kernel = new s4Kernel;

    start_session(_s4kernel, NULL);         // --@@--

    assert(_s4kernel->ID().is_kernel());
    assert(NULL == _s4kernel->_r4session->_parent);

    _kernel_session_context.session = _s4kernel->_r4session;
}

// ---------------------------------------------------------------------------

void r4Kernel::_allocate_sid (r4Session* r4s)
{
    // --@@--
    RWLock::Guard   guard(local.rwlock, RWLock::WRITE);

    r4s->_sid = _sid_generator.generate_next(local.sid_map);
    local.sid_map[r4s->_sid] = r4s;
}

// ---------------------------------------------------------------------------

SiD r4Kernel::start_session (Session* s, EventArg* arg)
{
    if (NULL == s || NULL == s->_r4session) // resource has been detached
        return SiD::NONE();

    r4Session*  r4s = s->_r4session;
    if (NULL != r4s->_handle)               // resource is already attached
        return r4s->_sid;

    if (NULL == r4s->_start_handler) {      // _start_handler not set
        delete s;
        return SiD::NONE();
    }

    if (! r4s->_start_handler->syntax(NULL, 0, arg, true)) {
        delete s;
        return SiD::NONE();
    }

    r4s->_handle = s;   // attach resource now
    r4s->_kernel = this;
    r4s->_parent = _current_context->session;

    // add to parent's list of children
    if (NULL != r4s->_parent) {
        _r4Session::list_children(*r4s->_parent).put_tail(r4s);
    }

    _allocate_sid(r4s);                     // --@@--

    CCScope __scope(r4s, ".start");

    EvCtx   ctx(this);

    if (NULL != r4s->_parent) {
        ctx.sender       = _current_context->parent->session->_sid;
        ctx.sender_state = _current_context->parent->state;
    }
    else {
        ctx.sender = SiD(_kid, 1);  // kernel session (itself)
    }

    r4s->_start_handler->execute(ctx, NULL, 0, arg);

    delete r4s->_start_handler;
    r4s->_start_handler = NULL;

    return r4s->_sid;
}

// ---------------------------------------------------------------------------

void r4Kernel::call_stop (r4Session& root, r4Session& node)
{
    CCScope __scope(&node, ".stop");

    EvCtx   ctx(this);

    // sender is always a session on which stop_session() is called
    ctx.sender = root._sid;

    if (&root == _current_context->parent->session) {
        ctx.sender_state = _current_context->parent->state;
    }

    //
    // call _stop_handler() and unregistar function(s)
    //
    if (NULL != node._stop_handler) {
        node._stop_handler->execute(ctx, NULL, 0, NULL/*no-arg*/);
    }

    vector<Unregistrar>::iterator i = node._unregistrar.begin();
    for (; i != node._unregistrar.end(); ++i) {
        (*i)(node._sid);
    }
}

// ---------------------------------------------------------------------------

StateCmd* r4Kernel::find_state_handler (SiD::IntType sid1, const string& ev)
{
    S1Ev_Cmd::iterator  sh = _s1ev_cmd.find(make_pair(sid1, ev));
    return (sh == _s1ev_cmd.end()) ? NULL : (*sh).second;
}

// ---------------------------------------------------------------------------

void r4Kernel::state__cmd (const string& ev, StateCmd* cmd)
{
    //TODO: check if _current_session is allowable to accept Kernel::state()

    S1Ev    s1ev(_current_context->session->_sid.id(), ev);

    S1Ev_Cmd::iterator  kv = _s1ev_cmd.find(s1ev);

    if (kv != _s1ev_cmd.end()) {
        swap(cmd, (*kv).second);
        delete cmd;
        if (NULL == (*kv).second)
            _s1ev_cmd.erase(kv);
    }
    else
    if (NULL != cmd) {
        _s1ev_cmd.insert(S1Ev_Cmd::value_type(s1ev, cmd));
    }
}

// ---------------------------------------------------------------------------

bool r4Kernel::call__arg (SiD on, const string& ev, ValParam* pfx, EventArg* arg)
{
    auto_ptr<EventArg>  __arg(arg);

    // assertions confirm what have been validated by a caller
    assert(NULL != _thread);
    assert(on.kid() == _kid);

    r4Session*  session = NULL;

    // check if `on' is the current session
    if (on == _current_context->session->_sid) {
        session = _current_context->session;
        assert(NULL != session);
    }
    else {
        session = local.find_session(on);
    }

    if (NULL == session) {
#if 1
        // `on' not found or inappropriate
        //errno = ???
        {
            cerr << "---\nCALLing (" << ev << ") failed: target "
                                     << on << " not found or invalid\n"
                 << "  sender " << _current_context->session->_sid << " at state "
                                << _current_context->state << "."
                 << endl;
        }
#endif
        return false;
    }
    assert(this == session->_kernel);

    StateCmd* cmd = find_state_handler(on.id(), ev);
    if (NULL == cmd) {
#if 1
        //state handler not found
        //errno = ???
        {
            cerr << "---\nCALLing (" << ev << ") failed: handler in target "
                                     << on << " not found\n"
                 << "  sender " << _current_context->session->_sid << " at state "
                                << _current_context->state << "."
                 << endl;
        }
#endif
        return false;
    }

    CCScope __scope(session, ev);

    EvCtx   ctx(this);

    ctx.sender       = _current_context->parent->session->_sid;
    ctx.sender_state = _current_context->parent->state;

    if (NULL != pfx) {
        int xN;
        const ArgTV* xA = pfx->arg_list(xN);
        cmd->execute(ctx, xA, xN, arg);     // callback
    }
    else {
        cmd->execute(ctx, NULL, 0, arg);
    }

    return true;
}

// ---------------------------------------------------------------------------

void r4Kernel::dispatch_evmsg (EvMsg* evmsg)
{
    r4Session*  session = evmsg->target();

    if (! session->local.stopper.isset()) {     // alive session

        StateCmd*   cmd = find_state_handler(session->_sid.id(), evmsg->name());
        if (NULL != cmd) {

            CCScope __scope(session, evmsg->name());

            EvCtx   ctx(this);

            ctx.sender       = evmsg->sender();
            ctx.sender_state = evmsg->sender_state();

            if (NULL != evmsg->pfx()) {
                int xN;
                const ArgTV* xA = evmsg->pfx()->arg_list(xN);
                cmd->execute(ctx, xA, xN, evmsg->arg());    // postback
            }
            else {
                cmd->execute(ctx, NULL, 0, evmsg->arg());
            }
        }
        else {  // state handler not found
#if 1
            cerr << "---\nPOST event (" << evmsg->name() << ") to target "
                                        << session->_sid
                                            << " not delivered: handler not found\n"
                 << "  sender " << evmsg->sender() << " at state "
                                << evmsg->sender_state() << "."
                 << endl;
#endif
        }
    }

    delete evmsg;
}

// ---------------------------------------------------------------------------

void r4Kernel::dispatch_alarm (EvAlarm* alarm)
{
    r4Session*  session = alarm->target();

    if (! session->local.stopper.isset()) {     // alive session

        StateCmd*   cmd = find_state_handler(session->_sid.id(), alarm->name());
        if (NULL != cmd) {

            CCScope __scope(session, alarm->name());

            EvCtx   ctx(this);

            ctx.sender       = session->_sid;
            ctx.sender_state = alarm->sender_state();
            ctx.alarm_id     = alarm->aid();

            cmd->execute(ctx, NULL, 0, alarm->arg());
        }
        else {  // state handler not found
#if 1
            cerr << "---\nALARM event (" << alarm->name() << ") on target "
                                         << session->_sid
                                            << " not delivered: handler not found."
                 << endl;
#endif
        }
    }

    assert(NULL    != session->_kernel);
    assert(_thread == session->_kernel->_thread);

    //TODO: if alarm periodic - re-schedule, rather than delete
    //
    _thread->delete_alarm(alarm, false/*already erased from _dsa_map*/);
}

// ---------------------------------------------------------------------------

void r4Kernel::dispatch_evio (EvIO* evio)
{
    r4Session*  session = evio->target();

    if (! session->local.stopper.isset()) {     // alive session

        StateCmd*   cmd = find_state_handler(session->_sid.id(), evio->name());
        if (NULL != cmd) {

            CCScope __scope(session, evio->name());

            EvCtx   ctx(this);

            ctx.sender       = session->_sid;
            ctx.sender_state = evio->sender_state();

            DatIO   dio(evio->fd(), evio->mode());

            ArgTV   iop;
            iop.set(&typeid(dio), &dio);

            cmd->execute(ctx, &iop, 1, evio->arg());
        }
        else {  // state handler not found
#if 1
            cerr << "---\nI/O event (" << evio->name() << ") on target "
                                       << session->_sid
                                            << " not delivered: handler not found."
                 << endl;
#endif
        }
    }
}

