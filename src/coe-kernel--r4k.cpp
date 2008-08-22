// coe-kernel--r4k.cpp

#include "coe-session.h"
#include "coe-kernel--r4k.h"
#include "coe-kernel--s4k.h"
#include "coe-session--r4s.h"

#include <assert.h>

#include <deque>    // g_Queue

using namespace std;

//TODO: put to global store, protected by a library-level mutex
typedef map<SiD, r4Session*> SessionMap;
static SessionMap g_SessionMap;

//TODO: event queue, thread (local or foreign) level
static deque<EvCommon*> g_Queue;

static /*FIXME:__thread*/ d4Thread* g_tls_thread = NULL;

// -----------------------------------------------------------------------

class CCScope {
public:
    CCScope (SessionContext& orig) : _orig(orig) { _swap(); }
    ~CCScope () { _swap(); }
private:
    void _swap ()
        {
            swap(_orig.session, _keep.session);
            _orig.state.swap(_keep.state);
        }
    SessionContext& _orig;
    SessionContext  _keep;
};

// =======================================================================

r4Kernel::r4Kernel ()
{
    _thread = g_tls_thread;
    if (NULL == _thread) {
        _thread = new d4Thread;
        g_tls_thread = _thread;
    }

    _handle = NULL;

    _last_sid = 0;
    _sid_wrap = false;

    _s4kernel = new s4Kernel;
    start_session(_s4kernel);

    assert(_s4kernel->ID() == SiD::KERNEL);

    _current_context.session = _s4kernel->_r4session;
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

SiD r4Kernel::start_session (Session* s)
{
    if (NULL == s || NULL == s->_r4session) // resource has been detached
        return SiD();

    r4Session*  r4s = s->_r4session;
    if (NULL != r4s->_handle)               // resource is already attached
        return r4s->_sid;

    r4s->_handle = s;   // attach resource now
    r4s->_kernel = this;
    r4s->_parent = _current_context.session;

    SiD sid = get_next_unique_sid();
    r4s->_sid = sid;

    //TODO: must be atomic op, under library-level mutex
    g_SessionMap.insert(SessionMap::value_type(sid, r4s));

    CCScope __scope(_current_context);

    _current_context.session = r4s;

    EvCtx   ctx(*_handle, *s);

    set_heap_ptr(ctx);
    //TODO: ev?, ...
    //ctx.sender = SiD(0,1);  // keep->_sid;
    s->_start(ctx);

    return r4s->_sid;
}

// -----------------------------------------------------------------------

void r4Kernel::set_heap_ptr (EvCtx& ctx)
{
    ctx.heap = ctx.session.get_heap();
}

// -----------------------------------------------------------------------

StateCmd* r4Kernel::find_state_handler (NiD sid1, const string& ev)
{
    S1Ev_Cmd::iterator  sh = _s1ev_cmd.find(make_pair(sid1, ev));
    return (sh == _s1ev_cmd.end()) ? NULL : (*sh).second;
}

// -----------------------------------------------------------------------

bool r4Kernel::post__arg (SiD to, const string& ev, PostArg* arg)
{
    auto_ptr<EvMsg> evmsg(new EvMsg(ev, arg, _current_context));

    //TODO:
    // case 1) `to' is local
    // case 2) `to' is foreign

    SessionMap::iterator    sp = g_SessionMap.find(to);
    if (sp == g_SessionMap.end()) {
        //errno = ???
        return false;
    }

    evmsg->_target = (*sp).second;

    g_Queue.push_back(evmsg.release());

    return true;
}

// -----------------------------------------------------------------------

bool r4Kernel::yield__arg (const std::string& ev, PostArg* arg)
{
    return post__arg(_current_context.session->_sid, ev, arg);
}

// -----------------------------------------------------------------------

bool r4Kernel::call__arg (SiD on, const string& ev, CallArg* arg)
{
    //
    // FIXME: now it's loopback -- dispatched immediately to itself.
    //

    auto_ptr<CallArg>   __arg(arg);

#if 0
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
#endif
    return true;
}

// -----------------------------------------------------------------------

bool r4Kernel::select__arg (int fd, IO_Mode mode, const string& ev, PostArg* arg)
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

    auto_ptr<PostArg>   __arg(arg);

#if 0
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
#endif
    return true;
}

// -----------------------------------------------------------------------

void r4Kernel::state__cmd (const string& ev, StateCmd* cmd)
{
    //TODO: check if _current_session is allowable to accept Kernel::state()

    S1Ev    s1ev(_current_context.session->_sid.s(), ev);

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

void r4Kernel::dispatch_evmsg (EvMsg* evmsg)
{
    StateCmd* cmd = find_state_handler(evmsg->_target->_sid.s(), evmsg->_name);
    if (NULL == cmd) {
        //TODO: call session's default error handling, like _default in POE?
        return;
    }

    CCScope __scope(_current_context);

    _current_context.session = evmsg->_target;
    _current_context.state   = evmsg->_name;

    EvCtx   ctx(*_handle, *_current_context.session->_handle);

    set_heap_ptr(ctx);
    ctx.state        = _current_context.state;
    ctx.sender       = evmsg->_sender;
    ctx.sender_state = evmsg->_sender_state;

    cmd->execute(ctx, NULL, 0, evmsg->_arg);

    //TODO: decrement sender's ref-count
}

// -----------------------------------------------------------------------

void r4Kernel::run_event_loop ()
{
    //TODO
    while (! g_Queue.empty()) {
        EvCommon*   ev = g_Queue.front();
        g_Queue.pop_front();
        ev->dispatch();
        delete ev;
    }
}

