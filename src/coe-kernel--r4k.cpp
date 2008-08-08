// coe-kernel--r4k.cpp

#include "coe-session.h"
#include "coe-kernel--r4k.h"
#include "coe-kernel--s4k.h"
#include "coe-session--r4s.h"

#include <assert.h>

using namespace std;

static r4Session* s_LAST_SESSION;   //FIXME: HACK: this is temporary hack to keep initial test running

// =======================================================================

r4Kernel::r4Kernel ()
{
    _handle = NULL;
    _parent = NULL;             //FIXME: how to get/provide it?

    _current_session = NULL;

    _last_sid = 0;
    _sid_wrap = false;

    _s4kernel = new s4Kernel;
    bool res = start_session(_s4kernel);

    assert(res);
    assert(_s4kernel->ID() == SiD::KERNEL);

    _current_session = _s4kernel->_r4session;
}

// -----------------------------------------------------------------------

SiD r4Kernel::get_next_unique_sid ()
{
    if (0 == ++_last_sid || _sid_wrap) {
        _sid_wrap = true;
        //TODO: visit sessions' map to get next unique sid
    }
    return SiD(0, _last_sid);
}

// -----------------------------------------------------------------------

bool r4Kernel::start_session (Session* s)
{
    if (NULL == s || NULL == s->_r4session) // resource has been detached
        return false;

    r4Session*  r4s = s->_r4session;
    if (NULL != r4s->_handle)   // resource is already attached
        return false;

    r4s->_handle = s;   // attach resource now
    r4s->_kernel = this;
    r4s->_parent = _current_session;
    r4s->_sid    = get_next_unique_sid();

    r4Session*  keep = _current_session;
    _current_session = r4s;

    ::s_LAST_SESSION = _current_session;    //FIXME: HACK

    EvCtx   ctx(*_handle, *s);

    set_heap_ptr(ctx);
    //TODO: ev?, ...
    //ctx.sender = SiD(0,1);  // keep->_sid;
    s->_start(ctx);

    _current_session = keep;
    return true;
}

// -----------------------------------------------------------------------

void r4Kernel::set_heap_ptr (EvCtx& ctx)
{
    ctx.heap = ctx.session.get_heap();
}

// -----------------------------------------------------------------------

StateCmd* r4Kernel::find_state_handler (const string& ev)
{
    map<string, StateCmd*>::iterator sh = _state_cmd.find(ev);
    return (sh == _state_cmd.end()) ? NULL : (*sh).second;
}

// -----------------------------------------------------------------------

bool r4Kernel::post__arg (SiD to, const string& ev, PostArg* arg)
{
    // 1) create EvMessage and attach EventArg to it
    // 2) try to enqueue such an EvMessage
    //  - if `to' is local -- easy
    //  - if `to' is foreign -- must do it in reciepient's critical section
    //    (in this case foreign receipient must be signalled somehow)
    // 3) if enqueue failed, destroy EvMessage (with EventArg attached)
    //

    // *******************************************************************

    //
    // FIXME: now it's loopback -- dispatched immediately to itself.
    //

    auto_ptr<EventArg>  __arg(arg);

    StateCmd* cmd = find_state_handler(ev);
    if (NULL == cmd) {
        //TODO: errno = ???
        return false;
    }

    //
    //FIXME:
    //  1) what to verify? _kernel, _current_session, etc...
    //  2) to, heap -- need to set correct values
    //

    r4Session*  keep = _current_session;
    _current_session = ::s_LAST_SESSION;

    EvCtx   ctx(*_handle, *_current_session->_handle);

    set_heap_ptr(ctx);
    ctx.state = ev;

    cmd->execute(ctx, NULL, 0, arg);

    _current_session = keep;
    return true;
}

// -----------------------------------------------------------------------

bool r4Kernel::yield__arg (const std::string& ev, PostArg* arg)
{
    auto_ptr<EventArg>  __arg(arg);

    StateCmd* cmd = find_state_handler(ev);
    if (NULL == cmd) {
        //TODO: errno = ???
        return false;
    }

    //
    //FIXME:
    //  1) what to verify? _kernel, _current_session, etc...
    //  2) to, heap -- need to set correct values
    //

    r4Session*  keep = _current_session;
    _current_session = ::s_LAST_SESSION;

    EvCtx   ctx(*_handle, *_current_session->_handle);

    set_heap_ptr(ctx);
    ctx.state = ev;

    cmd->execute(ctx, NULL, 0, arg);

    _current_session = keep;
    return true;
}

// -----------------------------------------------------------------------

bool r4Kernel::call__arg (SiD on, const string& ev, CallArg* arg)
{
    //
    // FIXME: now it's loopback -- dispatched immediately to itself.
    //

    auto_ptr<EventArg>  __arg(arg);

    StateCmd* cmd = find_state_handler(ev);
    if (NULL == cmd) {
        //TODO: errno = ???
        return false;
    }

    //
    //FIXME:
    //  1) what to verify? _kernel, _current_session, etc...
    //  2) to, heap -- need to set correct values
    //

    r4Session*  keep = _current_session;
    _current_session = ::s_LAST_SESSION;

    EvCtx   ctx(*_handle, *_current_session->_handle);

    set_heap_ptr(ctx);
    ctx.state = ev;

    cmd->execute(ctx, NULL, 0, arg);

    _current_session = keep;
    return true;
}

// -----------------------------------------------------------------------

bool r4Kernel::select__arg (int fd, Kernel::IO_Mode mode, const string& ev, PostArg* arg)
{
    //
    // similar to post__arg above, however with differencies:
    //
    // a) EvMessage is IOWatcher
    // b) IOWatcher is always local
    // c) Signalled is a thread controlling this Kernel
    //    (note more Kernels can be under such a thread)
    //

    // *******************************************************************

    //
    // FIXME: now it's loopback -- dispatched immediately to itself.
    //

    auto_ptr<EventArg>  __arg(arg);

    StateCmd* cmd = find_state_handler(ev);
    if (NULL == cmd) {
        //TODO: errno = ???
        return false;
    }

    //
    //FIXME:
    //  1) what to verify? _kernel, _current_session, etc...
    //  2) SiD, heap -- need to set correct values
    //

    r4Session*  keep = _current_session;
    _current_session = ::s_LAST_SESSION;

    EvCtx   ctx(*_handle, *_current_session->_handle);

    set_heap_ptr(ctx);
    ctx.state  = ev;
    ctx.sender = SiD(0,1);

    DatIO   dio(fd, mode);

    ArgTV   iop;
    iop.set(&typeid(dio), &dio);

    cmd->execute(ctx, &iop, 1, arg);

    _current_session = keep;
    return true;
}

// -----------------------------------------------------------------------

void r4Kernel::state__cmd (const string& ev, StateCmd* cmd)
{
    State2Cmd::iterator kv = _state_cmd.find(ev);

    if (kv != _state_cmd.end()) {
        swap(cmd, (*kv).second);
        delete cmd;
        if (NULL == (*kv).second)
            _state_cmd.erase(kv);
    }
    else
    if (NULL != cmd) {
        _state_cmd.insert(State2Cmd::value_type(ev, cmd));
    }
}

// -----------------------------------------------------------------------

void r4Kernel::run_event_loop ()
{
    //TODO
}

