// coe-kernel.cpp

#include "coe-kernel.h"
#include "coe-session.h"
#include "coe-kernel--r4k.h"

using namespace std;

// =======================================================================

EvCtx::EvCtx (Kernel& k, Session& s)
    : kernel(k), session(s)
{
    heap = NULL;
}

// ------------------------------------

DatIO::DatIO (int f, IO_Mode m)
    : filedes(f), mode(m)
{
}

// =======================================================================

Kernel::Kernel ()
{
    _r4kernel = new r4Kernel;
    _r4kernel->_handle = this;
}

Kernel& Kernel::create_new ()
{
    //TODO: use pthread(s) and TLS? Hmm... TLS would preclude thread pools!
    static Kernel* pKernel = new Kernel;
    return *pKernel;
}

SiD Kernel::start_session (Session* s)
{
    return _r4kernel->start_session(s);
}

void Kernel::run_event_loop ()
{
    _r4kernel->run_event_loop();
}

// -----------------------------------------------------------------------

bool Kernel::post (SiD to, const string& ev, PostArg* vp)
{
    return _r4kernel->post__arg(to, ev, vp);
}

bool Kernel::anon_post (SiD to, const string& ev, PostArg* vp)
{
    return d4Thread::anon_post__arg(to, ev, vp);
}

bool Kernel::yield (const string& ev, PostArg* vp)
{
    return _r4kernel->yield__arg(ev, vp);
}

bool Kernel::call (SiD on, const string& ev, CallArg* rp)
{
    return _r4kernel->call__arg(on, ev, rp);
}

bool Kernel::select (int fd, IO_Mode mode, const string ev, PostArg* vp)
{
    return _r4kernel->select__arg(fd, mode, ev, vp);
}

void Kernel::state (const string& ev)
{
    _r4kernel->state__cmd(ev, NULL);
}

void Kernel::state (const string& ev, StateCmd* handler)
{
    _r4kernel->state__cmd(ev, handler);
}

