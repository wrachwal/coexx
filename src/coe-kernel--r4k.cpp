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

#include "coe-kernel--r4k.h"
#include "coe-thread--d4t.h"
#include "coe-kernel--s4k.h"
#include "coe-session.h"
#include "coe-session--r4s.h"

#include <cassert>

using namespace std;

// -----------------------------------------------------------------------

namespace {

    class CCScope {
    public:
        CCScope (r4Kernel* target,
                 r4Session* session,
                 const string& state) : _kernel(target),
                                        _old_ctx(target->_current_context),
                                        _new_ctx(session, state)
            {
                assert(target == session->_kernel);
                _new_ctx.parent = target->_current_context;
                target->_current_context = &_new_ctx;
            }

        ~CCScope ()
            {
                _kernel->_current_context = _old_ctx;
            }

    private:
        r4Kernel*       _kernel;
        SessionContext* _old_ctx;
        SessionContext  _new_ctx;
    };

}

// -----------------------------------------------------------------------
// d4Thread::Local

r4Session* r4Kernel::Local::find_session (SiD sid) const
{
    Sid_Map::const_iterator i = sid_map.find(sid);
    return i == sid_map.end() ? NULL : (*i).second;
}

// =======================================================================

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

    start_session(_s4kernel);               // --@@--

    assert(_s4kernel->ID().is_kernel());
    assert(NULL == _s4kernel->_r4session->_parent);

    _kernel_session_context.session = _s4kernel->_r4session;
}

// -----------------------------------------------------------------------

SiD r4Kernel::start_session (Session* s)
{
    if (NULL == s || NULL == s->_r4session) // resource has been detached
        return SiD::NONE();

    r4Session*  r4s = s->_r4session;
    if (NULL != r4s->_handle)               // resource is already attached
        return r4s->_sid;

    r4s->_handle = s;   // attach resource now
    r4s->_kernel = this;
    r4s->_parent = _current_context->session;

    _allocate_sid(r4s);                     // --@@--

    CCScope __scope(this, r4s, ".start");

    EvCtx   ctx(this);

    if (NULL != r4s->_parent) {
        ctx.sender       = _current_context->parent->session->_sid;
        ctx.sender_state = _current_context->parent->state;
    }
    else {
        ctx.sender = SiD(_kid, 1);  // kernel session (itself)
    }

    s->_start(ctx);

    return r4s->_sid;
}

// -----------------------------------------------------------------------

void r4Kernel::_allocate_sid (r4Session* r4s)
{
    // --@@--
    RWLock::Guard   guard(local.rwlock, RWLock::WRITE);

    r4s->_sid = _sid_generator.generate_next(local.sid_map);
    local.sid_map[r4s->_sid] = r4s;
}

// -----------------------------------------------------------------------

StateCmd* r4Kernel::find_state_handler (SiD::IntType sid1, const string& ev)
{
    S1Ev_Cmd::iterator  sh = _s1ev_cmd.find(make_pair(sid1, ev));
    return (sh == _s1ev_cmd.end()) ? NULL : (*sh).second;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

bool r4Kernel::call__arg (SiD on, const string& ev, CallArg* arg)
{
    auto_ptr<CallArg>   __arg(arg);

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
        //TODO: call session's default error handling, like _default in POE?
        //errno = ???
        return false;
    }
    assert(this == session->_kernel);

    StateCmd* cmd = find_state_handler(on.id(), ev);
    if (NULL == cmd) {
        //TODO: call session's default error handling, like _default in POE?
        //errno = ???
        return false;
    }

    CCScope __scope(this, session, ev);

    EvCtx   ctx(this);

    ctx.sender       = _current_context->parent->session->_sid;
    ctx.sender_state = _current_context->parent->state;

    cmd->execute(ctx, NULL, 0, arg);

    return true;
}

// -----------------------------------------------------------------------

void r4Kernel::dispatch_evmsg (EvMsg* evmsg)
{
    StateCmd* cmd = find_state_handler(evmsg->target()->_sid.id(), evmsg->name());
    if (NULL == cmd) {
        //TODO: call session's default error handling, like _default in POE?
        delete evmsg;
        return;
    }

    CCScope __scope(this, evmsg->target(), evmsg->name());

    EvCtx   ctx(this);

    ctx.sender       = evmsg->sender();
    ctx.sender_state = evmsg->sender_state();

    cmd->execute(ctx, NULL, 0, evmsg->arg());

    //TODO: decrement sender's ref-count

    delete evmsg;
}

// -----------------------------------------------------------------------

void r4Kernel::dispatch_alarm (EvAlarm* alarm)
{
    r4Session*  session = alarm->target();

    StateCmd* cmd = find_state_handler(session->_sid.id(), alarm->name());
    if (NULL == cmd) {
        //TODO: call session's default error handling, like _default in POE?
        delete alarm;
        return;
    }

    CCScope __scope(this, session, alarm->name());

    EvCtx   ctx(this);

    ctx.sender       = session->_sid;
    ctx.sender_state = alarm->sender_state();
    ctx.alarm_id     = alarm->aid();

    cmd->execute(ctx, NULL, 0, alarm->arg());

    //TODO: decrement sender's ref-count

    session->_list_alarm.remove(alarm);

    //TODO: re-schedule periodic alarm
    delete alarm;
}

// -----------------------------------------------------------------------

void r4Kernel::dispatch_evio (EvIO* evio)
{
    r4Session*  session = evio->target();

    StateCmd* cmd = find_state_handler(session->_sid.id(), evio->name());
    if (NULL == cmd) {
        //TODO: call session's default error handling, like _default in POE?
        return;
    }

    CCScope __scope(this, session, evio->name());

    EvCtx   ctx(this);

    ctx.sender       = session->_sid;
    ctx.sender_state = evio->sender_state();

    DatIO   dio(evio->fd(), evio->mode());

    ArgTV   iop;
    iop.set(&typeid(dio), &dio);

    cmd->execute(ctx, &iop, 1, evio->arg());

    //TODO: decrement sender's ref-count
}

