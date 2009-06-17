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
// d4Thread::Local

r4Session* r4Kernel::Local::find_session (SiD sid) const
{
    Sid_Map::const_iterator i = sid_map.find(sid);
    return i == sid_map.end() ? NULL : (*i).second;
}

// ===========================================================================

r4Kernel::r4Kernel ()
:   _thread(NULL),
    _handle(NULL),
    _current_context(NULL),
    _kernel_session_context(this)
{
    // trying attach kernel to current thread
    _thread = d4Thread::get_d4t_tls();

    if (NULL == _thread) {  // thread's event loop has not been run yet
        _thread = new d4Thread;
        _thread->_os_thread = pthread_self();
        d4Thread::set_d4t_tls(_thread);
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

    const _TypeDN*  hT = r4s->_start_handler->par_type();
    const _TypeDN*  aT = arg ? arg->arg_type() : NULL;

    if (! syntax_check(hT, NULL, aT)) {
        //TODO: detailed error message
#if 1
        cerr << "! ERROR: invalid `start_handler'" << endl;
#endif
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

    ExecuteContext  run(r4s, ".start", r4s->_start_handler);

    EvCtx   ctx(this);

    if (NULL != r4s->_parent) {
        ctx.sender       = _current_context->parent->session->_sid;
        ctx.sender_state = _current_context->parent->state;
    }
    else {
        ctx.sender = SiD(_kid, 1);  // kernel session (itself)
    }

    run.execute(ctx, NULL, 0, arg);

    delete r4s->_start_handler;
    r4s->_start_handler = NULL;

    return r4s->_sid;
}

// ---------------------------------------------------------------------------

void r4Kernel::call_stop (r4Session& root, r4Session& node)
{
    //
    // call `_stop_handler'
    //
    if (NULL != node._stop_handler) {

        ExecuteContext  run(&node, ".stop", node._stop_handler);

        EvCtx   ctx(this);

        // sender is always a session on which stop_session() was called
        ctx.sender = root._sid;

        if (&root == _current_context->parent->session) {
            ctx.sender_state = _current_context->parent->state;
        }

        run.execute(ctx, NULL, 0, NULL/*no-arg*/);
    }

    //
    // call unregistar function(s)
    //
    vector<Unregistrar>::iterator i = node._unregistrar.begin();
    for (; i != node._unregistrar.end(); ++i) {
        (*i)(node._sid);
    }
}

// ---------------------------------------------------------------------------

bool r4Kernel::delete_alarm (AiD aid)
{
    Sid1Aid_Key     s1a(_current_context->session->_sid, aid);

    Sid1Aid_Map::iterator   i = _s1a_map.find(s1a);

    if (i == _s1a_map.end()) {
        return false;
    }

    EvAlarm*    alarm = (*i).second;
    assert(alarm->s1a_iter() == i);     // btw cheap validity check

    _thread->delete_alarm(alarm);
    return true;
}

// ------------------------------------

bool r4Kernel::adjust_alarm (AiD aid, const TimeSpec& abs_time, bool update, ValParam* new_arg)
{
    Sid1Aid_Key     s1a(_current_context->session->_sid, aid);

    Sid1Aid_Map::iterator   i = _s1a_map.find(s1a);

    if (i == _s1a_map.end()) {
        delete new_arg;
        return false;
    }

    EvAlarm*    alarm = (*i).second;
    assert(alarm->s1a_iter() == i);     // btw cheap validity check

    // remember where it was adjust'ed
    alarm->sender_state(_current_context->state);

    if (update) {
        ValParam*   old_arg = alarm->arg(new_arg);
        if (old_arg != new_arg) {
            delete old_arg;
        }
    }

    _thread->adjust_alarm(alarm, abs_time);
    return true;
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

    bool    status = false;

    if (NULL != session) {

        assert(this == session->_kernel);

        ExecuteContext  run(session, ev, find_state_handler(on.id(), ev));

        EvCtx   ctx(this);

        ctx.sender       = _current_context->parent->session->_sid;
        ctx.sender_state = _current_context->parent->state;

        if (NULL != pfx) {
            const _TypeDN*  xT = pfx->arg_type();
            void**          xV = pfx->arg_list();
            status = run.execute(ctx, xT, xV, arg);     // callback
        }
        else {
            status = run.execute(ctx, NULL, 0, arg);
        }
    }
    else {
#if 1
        //TODO: reformat error message
        //  (`on' not found or inappropriate; errno = ???)
        //  print _current_context stack
        {
            cerr << "---\nCALLing (" << ev << ") failed: target "
                                     << on << " not found or invalid\n"
                 << "  sender " << _current_context->session->_sid << " at state "
                                << _current_context->state << "."
                 << endl;
        }
#endif
    }

    delete arg;
    return status;
}

// ---------------------------------------------------------------------------

void r4Kernel::dispatch_evmsg (EvMsg* evmsg)
{
    r4Session*  session = evmsg->target();

    if (! session->local.stopper.isset()) {     // alive session

        ExecuteContext  run(session,
                            evmsg,
                            find_state_handler(session->_sid.id(), evmsg->name()));

        EvCtx   ctx(this);

        ctx.sender       = evmsg->sender();
        ctx.sender_state = evmsg->sender_state();

        if (NULL != evmsg->pfx()) {
            const _TypeDN*  xT = evmsg->pfx()->arg_type();
            void**          xV = evmsg->pfx()->arg_list();
            run.execute(ctx, xT, xV, evmsg->arg());     // postback
        }
        else {
            run.execute(ctx, NULL, 0, evmsg->arg());
        }
    }

    delete evmsg;
}

// ---------------------------------------------------------------------------

void r4Kernel::dispatch_alarm (EvAlarm* alarm)
{
    r4Session*  session = alarm->target();

    if (! session->local.stopper.isset()) {     // alive session

        ExecuteContext  run(session,
                            alarm,
                            find_state_handler(session->_sid.id(), alarm->name()));

        EvCtx   ctx(this);

        ctx.sender       = session->_sid;
        ctx.sender_state = alarm->sender_state();
        ctx.alarm_id     = alarm->aid();

        run.execute(ctx, NULL, 0, alarm->arg());
    }

    assert(NULL    != session->_kernel);
    assert(_thread == session->_kernel->_thread);

    //TODO: if alarm periodic - re-schedule, rather than delete
    //
    _thread->delete_alarm(alarm);
}

// ---------------------------------------------------------------------------

void r4Kernel::dispatch_evio (EvIO* evio)
{
    r4Session*  session = evio->target();

    if (! session->local.stopper.isset()) {     // alive session

        ExecuteContext  run(session,
                            evio,
                            find_state_handler(session->_sid.id(), evio->name()));

        EvCtx   ctx(this);

        ctx.sender       = session->_sid;
        ctx.sender_state = evio->sender_state();

        DatIO   dio(evio->fd(), evio->mode());
        void*   pfx[] = { &dio };

        run.execute(ctx, _TypeI1<DatIO>().data(), pfx, evio->arg());
    }
}

