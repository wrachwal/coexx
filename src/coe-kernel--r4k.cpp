// coe-kernel--r4k.cpp

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
        _thread = new d4Thread(true);
        _thread->_os_thread = pthread_self();
        d4Thread::set_d4t_tls(_thread);
        _thread->allocate_tid();            // --@@--
    }

    d4Thread::allocate_kid(*this);          // --@@--

    // once `kid' is known, setup sid_generator
    _sid_generator = IdentGenerator<SiD>(SiD(_kid, 0));

    // initiate kernel local storage
    const _KlsD*    head = _KlsD::registry();
    if (NULL != head) {
        _user_kls.resize(head->info.index + 1);
    }

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

// ------------------------------------

r4Kernel::~r4Kernel ()
{
    // deallocate kernel local storage
    for (const _KlsD* info = _KlsD::registry(); NULL != info; info = info->next) {
        assert(info->info.index < _user_kls.size());
        void*   kls = _user_kls[info->info.index];
        if (NULL != kls) {
            (*info->info.destroy)(kls);
        }
    }

    delete _handle;
    _handle = NULL;
}

// ---------------------------------------------------------------------------

void* r4Kernel::get_user_kls (const _KlsD* info)
{
    assert(info->info.index < _user_kls.size());
    void*   kls = _user_kls[info->info.index];
    if (NULL == kls) {
        kls = _user_kls[info->info.index] = (*info->info.create)();
    }
    return kls;
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

    if (! r4s->_start_handler) {            // _start_handler not set
        delete s;
        return SiD::NONE();
    }

    const _TypeDN*  hT = r4s->_start_handler.par_type();
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

    ExecuteContext  run(r4s, EventContext::START, ".start");

    run.argument(arg);

    run.execute(*_handle, r4s->_start_handler);

    while (run.continuation) {
        Handler0    cont = run.continuation;
        run.continuation = Handler0();
        cont.execute(*_handle);
    }

    return r4s->_sid;
}

// ---------------------------------------------------------------------------

void r4Kernel::call_stop (r4Session& root, r4Session& node)
{
    //
    // call `_stop_handler'
    //
    if (node._stop_handler) {

        ExecuteContext  run(&node, EventContext::STOP, ".stop", &root);

        // EventContext.sender will be a session on which stop_session()
        // was called i.e. the root.ID

        run.execute(*_handle, node._stop_handler);

        while (run.continuation) {
            Handler0    cont = run.continuation;
            run.continuation = Handler0();
            cont.execute(*_handle);
        }
    }

    //
    // call unregistrar function(s) in LIFO order
    //
    while (! node._unregistrar.empty()) {
        void (*funptr)(SiD) = node._unregistrar.back();
        node._unregistrar.pop_back();   // pop before funptr call
        (*funptr)(node._sid);
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

    if (_thread->_dispatched_alarm == alarm) {
        _thread->_dispatched_alarm = NULL;      // mark as deleted
    }
    _thread->delete_alarm(alarm);       // delete

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
        ValParam*   old_arg = alarm->arg_change(new_arg);
        if (NULL != old_arg) {
            old_arg->destroy();
        }
    }

    _thread->adjust_alarm(alarm, abs_time);
    return true;
}

// ---------------------------------------------------------------------------

HandlerX r4Kernel::find_state_handler (SiD::IntType sid1, const CoeStr& ev)
{
    S1Ev_Cmd::iterator  i = _s1ev_cmd.find(make_pair(sid1, ev));
    return (i == _s1ev_cmd.end()) ? HandlerX() : (*i).second;
}

// ---------------------------------------------------------------------------

void r4Kernel::state__cmd (const CoeStr& ev, const HandlerX& cmd)
{
    //TODO: check if _current_session is allowable to accept Kernel::state()

    S1Ev    s1ev(_current_context->session->_sid.id(), ev);

    if (cmd) {
        _s1ev_cmd.insert(S1Ev_Cmd::value_type(s1ev, cmd));
    }
    else {
        //@@@XXX with planned CoeStr implementation there might be a Kernel
        //policy telling whether to remove an entry or reset only a value.
        _s1ev_cmd.erase(s1ev);
    }
}

// ---------------------------------------------------------------------------

bool r4Kernel::call__arg (SiD on, const CoeStr& ev, ValParam* pfx, EventArg* arg)
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

        ExecuteContext  run(session, EventContext::CALL, ev);

        run.locked_prefix(pfx);
        run.argument(arg);

        status = run.execute(*_handle, find_state_handler(on.id(), ev));

        while (run.continuation) {
            Handler0    cont = run.continuation;
            run.continuation = Handler0();
            cont.execute(*_handle);
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

        ExecuteContext  run(session, evmsg);

        run.locked_prefix  (evmsg->pfx());
        run.locked_argument(evmsg->arg());

        run.execute(*_handle, find_state_handler(session->_sid.id(), evmsg->name()));

        while (run.continuation) {
            Handler0    cont = run.continuation;
            run.continuation = Handler0();
            cont.execute(*_handle);
        }
    }

    delete evmsg;
}

// ---------------------------------------------------------------------------

void r4Kernel::dispatch_alarm (EvAlarm* alarm)
{
    assert(NULL == _thread->_dispatched_alarm);

    _thread->_dispatched_alarm = alarm;         // to track deletion

    r4Session*  session = alarm->target();

    if (! session->local.stopper.isset()) {     // alive session

        ExecuteContext  run(session, alarm);

        run.locked_argument(alarm->arg());

        run.execute(*_handle, find_state_handler(session->_sid.id(), alarm->name()));

        while (run.continuation) {
            Handler0    cont = run.continuation;
            run.continuation = Handler0();
            cont.execute(*_handle);
        }
    }

    if (NULL != _thread->_dispatched_alarm) {   // not deleted in handler(s)

        assert(alarm == _thread->_dispatched_alarm);

        _thread->_dispatched_alarm = NULL;      // clean the flag

        if (false/* alarm -> is-periodic */) {  // TODO
            /* re-schedule `alarm' */
        }
        else {
            _thread->delete_alarm(alarm);
        }
    }
}

// ---------------------------------------------------------------------------

void r4Kernel::dispatch_evio (EvIO* evio)
{
    r4Session*  session = evio->target();

    if (! session->local.stopper.isset()) {     // alive session

        ExecuteContext  run(session, evio);

        IO_Ctx  ioctx(evio->fd(), evio->mode());
        void*   pfx[] = { &ioctx };

        run.prefix(_TypeI1<IO_Ctx>().data(), pfx);
        run.locked_argument(evio->arg());

        run.execute(*_handle, find_state_handler(session->_sid.id(), evio->name()));

        while (run.continuation) {
            Handler0    cont = run.continuation;
            run.continuation = Handler0();
            cont.execute(*_handle);
        }
    }
}

