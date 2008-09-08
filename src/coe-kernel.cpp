// coe-kernel.cpp

#include "coe-kernel.h"
#include "coe-kernel--r4k.h"
#include "coe-session.h"
#include "coe-session--r4s.h"

using namespace std;

// =======================================================================
// EvCtx

EvCtx::EvCtx (Kernel& k, Session& s)
    : kernel(k), session(s)
{
    heap = NULL;
}

// -----------------------------------------------------------------------
// DatIO

DatIO::DatIO (int f, IO_Mode m)
    : filedes(f), mode(m)
{
}

// =======================================================================
// Kernel

Kernel::Kernel ()
{
    _r4kernel = new r4Kernel;
    _r4kernel->_handle = this;
}

Kernel& Kernel::create_new ()
{
    Kernel* pKernel = new Kernel;
    return *pKernel;
}

SiD Kernel::start_session (Session* s)
{
    return _r4kernel->start_session(s);
}

void Kernel::run_event_loop ()
{
    if (NULL != _r4kernel && NULL != _r4kernel->_thread) {
        // blocks only if loop has not been run yet
        _r4kernel->_thread->run_event_loop();
    }
}

// -----------------------------------------------------------------------

static inline
bool kernel_attached (r4Kernel* r4k)
{
    if (NULL == r4k || NULL == r4k->_thread) {
        // errno = ???  //TODO
        return false;
    }
    return true;
}

static inline
bool target_valid (SiD target)
{
    if (! target.valid()) {
        // errno = ???  //TODO
        return false;
    }
    return true;
}

static inline
bool user_evname (const string& ev)
{
    if (ev.empty() || '.' == ev[0]) {
        // errno = ???  //TODO
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------

bool Kernel::anon_post (SiD to, const string& ev, PostArg* vp)
{
    if (! target_valid(to) || ! user_evname(ev)) {
        delete vp;
        return false;
    }
    return d4Thread::anon_post_event(to, new EvMsg(ev, vp));
}

bool Kernel::post (SiD to, const string& ev, PostArg* vp)
{
    if (! kernel_attached(_r4kernel) || ! target_valid(to) || ! user_evname(ev)) {
        delete vp;
        return false;
    }
    return _r4kernel->post__arg(to, ev, vp);
}

bool Kernel::yield (const string& ev, PostArg* vp)
{
    if (! kernel_attached(_r4kernel) || ! user_evname(ev)) {
        delete vp;
        return false;
    }
    SiD to = _r4kernel->_current_context.session->_sid;
    if (! target_valid(to)) {
        delete vp;
        return false;
    }
    return _r4kernel->post__arg(to, ev, vp);
}

bool Kernel::call (SiD on, const string& ev, CallArg* rp)
{
    if (! kernel_attached(_r4kernel) || ! target_valid(on) || ! user_evname(ev)) {
        delete rp;
        return false;
    }
    return _r4kernel->call__arg(on, ev, rp);
}

bool Kernel::select (int fd, IO_Mode mode, const string ev, PostArg* vp)
{
    //TODO: check fd/mode
    if (! kernel_attached(_r4kernel) || ! user_evname(ev)) {
        delete vp;
        return false;
    }
    return _r4kernel->select__arg(fd, mode, ev, vp);
}

// -----------------------------------------------------------------------

void Kernel::state (const string& ev)
{
    _r4kernel->state__cmd(ev, NULL);
}

void Kernel::state (const string& ev, StateCmd* handler)
{
    _r4kernel->state__cmd(ev, handler);
}

