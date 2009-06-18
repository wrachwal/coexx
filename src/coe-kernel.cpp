// coe-kernel.cpp

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

#include "coe-kernel.h"
#include "coe-thread--d4t.h"    // coe-kernel--r4k.h
#include "coe-session.h"
#include "coe-session--r4s.h"
#include "coe--errno.h"

using namespace std;
using namespace coe;

// ===========================================================================
// EvCtx

EvCtx::EvCtx (r4Kernel* k)
  : kernel (*k->_handle),
    session(*k->_current_context->session->_handle),
    state  ( k->_current_context->state)
{
    heap = session.get_heap();
}

// ---------------------------------------------------------------------------
// DatIO

DatIO::DatIO (int f, IO_Mode m)
  : filedes(f),
    mode(m)
{
}

// ===========================================================================
// _KlsD

const _KlsD* _KlsD::_register (_KlsD* data)
{
    static _KlsD*   head = NULL;

    if (NULL != data) {
        assert(NULL == data->next);
        data->info.index = head ? head->info.index + 1 : 0;
        data->next = head;
        head = data;
    }

    return head;
}

// ===========================================================================
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

// ------------------------------------

KiD Kernel::ID () const
{
    return _r4kernel->_kid;
}

Thread& Kernel::thread () const
{
    return *_r4kernel->_thread->_handle;
}

void* Kernel::_get_user_kls (const _KlsD* data)
{
    return _r4kernel->get_user_kls(data);
}

void* next_kls_info (void* iter, LocalStorageInfo& info)
{
    const _KlsD*    data = iter ? static_cast<_KlsD*>(iter)->next
                                : _KlsD::registry();
    if (NULL != data) {
        info = data->info;
    }

    return (void*)data;
}

TimeSpec Kernel::timestamp () const
{
    return _r4kernel->_thread->_timestamp;
}

Session& Kernel::session ()
{
    assert(NULL != _r4kernel->_current_context);
    assert(NULL != _r4kernel->_current_context->session);
    assert(NULL != _r4kernel->_current_context->session->_handle);

    return *_r4kernel->_current_context->session->_handle;
}

SiD Kernel::current_session ()
{
    d4Thread*   thread = d4Thread::get_d4t_tls();
    if (NULL != thread) {
        r4Kernel*   kernel = thread->_current_kernel;
        if (NULL != kernel) {
            assert(NULL != kernel->_current_context);
            assert(NULL != kernel->_current_context->session);
            return kernel->_current_context->session->_sid;
        }
    }
    return SiD::NONE();
}

// ---------------------------------------------------------------------------

bool Kernel::move_to_thread (TiD tid)
{
    if (   ! kernel_attached(_r4kernel)
        || ! target_valid(tid))
    {
        return false;
    }

    if (tid == _r4kernel->_thread->_tid) {
        _r4kernel->_target_thread = TiD::NONE();    // reset (could be set earlier)
        //errno = ???
        return false;
    }

    // --@@--
    RWLock::Guard   guard(d4Thread::glob.rwlock, RWLock::READ);

    if (NULL == d4Thread::glob.find_thread(tid)) {
        //errno = ???
        return false;
    }

    if (_r4kernel->_target_thread.isset()) {
        _r4kernel->_target_thread = tid;
    }
    else {
        _r4kernel->_target_thread = tid;
        _r4kernel->_thread->_lqueue.put_head(new EvSys_Export_Kernel(_r4kernel));
    }

    return true;
}

// ---------------------------------------------------------------------------

Callback* Kernel::callback (const std::string& ev, ValParam* pfx)
{
    if (   ! kernel_attached(_r4kernel)
        || ! current_session_active(_r4kernel)
        || ! user_evname(ev))
    {
        delete pfx;
        return NULL;
    }
    return new Callback(_r4kernel->_current_context->session->_sid, ev, pfx);
}

// ---------------------------------------------------------------------------

bool Kernel::anon_post (SiD to, const string& ev, ValParam* vp)
{
    if (   ! target_valid(to)
        || ! user_evname(ev))
    {
        delete vp;
        return false;
    }
    return d4Thread::post_event(NULL/*source-kernel*/, to, new EvMsg(ev, vp));
}

// ------------------------------------

bool Kernel::post (SiD to, const string& ev, ValParam* vp)
{
    if (   ! kernel_attached(_r4kernel)
        || ! target_valid(to)
        || ! user_evname(ev))
    {
        delete vp;
        return false;
    }
    assert(NULL != _r4kernel->_current_context);
    assert(NULL != _r4kernel->_current_context->session);

    return d4Thread::post_event(
                            _r4kernel,
                            to,                                     // SiD
                            new EvMsg(ev, vp, *_r4kernel->_current_context)
                        );
}

// ------------------------------------

bool Kernel::yield (const string& ev, ValParam* vp)
{
    if (   ! kernel_attached(_r4kernel)
        || ! user_evname(ev))
    {
        delete vp;
        return false;
    }
    assert(NULL != _r4kernel->_current_context);
    assert(NULL != _r4kernel->_current_context->session);

    return d4Thread::post_event(
                            _r4kernel,
                            _r4kernel->_current_context->session,   // r4Session*
                            new EvMsg(ev, vp, *_r4kernel->_current_context)
                        );
}

// ---------------------------------------------------------------------------

bool Kernel::call (SiD on, const string& ev)
{
    if (   ! kernel_attached(_r4kernel)
        || ! target_valid(on)
        || ! kernel_equal(_r4kernel, on)
        || ! user_evname(ev))
    {
        return false;
    }
    return _r4kernel->call__arg(on, ev, NULL, NULL);
}

// ------------------------------------

bool Kernel::call (SiD on, const string& ev, RefParam* rp)
{
    if (   ! kernel_attached(_r4kernel)
        || ! target_valid(on)
        || ! kernel_equal(_r4kernel, on)
        || ! user_evname(ev))
    {
        delete rp;
        return false;
    }
    return _r4kernel->call__arg(on, ev, NULL, rp);
}

// ------------------------------------

bool Kernel::call (SiD on, const string& ev, ValParam* vp)
{
    if (   ! kernel_attached(_r4kernel)
        || ! target_valid(on)
        || ! kernel_equal(_r4kernel, on)
        || ! user_evname(ev))
    {
        delete vp;
        return false;
    }
    return _r4kernel->call__arg(on, ev, NULL, vp);
}

// ---------------------------------------------------------------------------

AiD Kernel::alarm_remove (AiD aid)
{
    if (   ! kernel_attached(_r4kernel)
        || ! current_session_active(_r4kernel))
    {
        return AiD::NONE();
    }
    return _r4kernel->delete_alarm(aid) ? aid : AiD::NONE();
}

// ------------------------------------

AiD Kernel::delay_set (const string ev, TimeSpec duration, ValParam* vp)
{
    if (   ! kernel_attached(_r4kernel)
        || ! current_session_active(_r4kernel)
        || ! delay_gt0(duration)
        || ! user_evname(ev))
    {
        delete vp;
        return AiD::NONE();
    }
    EvAlarm*    evalm = new EvAlarm(_r4kernel->_thread->_timestamp + duration,
                                    ev,
                                    vp,
                                    *_r4kernel->_current_context);
    return _r4kernel->_thread->create_alarm(evalm);
}

// ------------------------------------

bool Kernel::delay_adjust (AiD aid, TimeSpec secs_from_now)
{
    if (   ! kernel_attached(_r4kernel)
        || ! current_session_active(_r4kernel)
        || ! delay_gt0(secs_from_now))
    {
        return false;
    }
    return _r4kernel->adjust_alarm(
                            aid,
                            _r4kernel->_thread->_timestamp + secs_from_now,
                            /*update*/false,
                            NULL
                        );
}

// ------------------------------------

bool Kernel::delay_adjust (AiD aid, TimeSpec secs_from_now, ValParam* vp)
{
    if (   ! kernel_attached(_r4kernel)
        || ! current_session_active(_r4kernel)
        || ! delay_gt0(secs_from_now))
    {
        delete vp;
        return false;
    }
    return _r4kernel->adjust_alarm(
                            aid,
                            _r4kernel->_thread->_timestamp + secs_from_now,
                            /*update*/true,
                            vp
                        );
}

// ---------------------------------------------------------------------------

bool Kernel::select (int fd, IO_Mode mode)
{
    if (   ! kernel_attached(_r4kernel)
        || ! fd_valid(fd)
        || ! mode_valid(mode))
    {
        return false;
    }
    return _r4kernel->_thread->delete_io_watcher(
                                    fd,
                                    mode,
                                    _r4kernel->_current_context->session
                                );
}

// ------------------------------------

bool Kernel::select (int fd, IO_Mode mode, const string& ev, ValParam* vp)
{
    if (   ! kernel_attached(_r4kernel)
        || ! current_session_active(_r4kernel)
        || ! user_evname(ev)
        || ! fd_valid(fd)
        || ! mode_valid(mode))
    {
        delete vp;
        return false;
    }

    EvIO*   evio = new EvIO(fd, mode, ev, vp, *_r4kernel->_current_context);

    return _r4kernel->_thread->create_io_watcher(evio);
}

// ------------------------------------

bool Kernel::select_pause (int fd, IO_Mode mode)
{
    if (   ! kernel_attached(_r4kernel)
        || ! fd_valid(fd)
        || ! mode_valid(mode))
    {
        return false;
    }
    return _r4kernel->_thread->pause_io_watcher(
                                    fd,
                                    mode,
                                    _r4kernel->_current_context->session
                                );
}

// ------------------------------------

bool Kernel::select_resume (int fd, IO_Mode mode)
{
    if (   ! kernel_attached(_r4kernel)
        || ! fd_valid(fd)
        || ! mode_valid(mode))
    {
        return false;
    }
    return _r4kernel->_thread->resume_io_watcher(
                                    fd,
                                    mode,
                                    _r4kernel->_current_context->session
                                );
}

// ---------------------------------------------------------------------------

void Kernel::state (const string& ev)
{
    _r4kernel->state__cmd(ev, NULL);
}

// ------------------------------------

void Kernel::state (const string& ev, StateCmd* handler)
{
    _r4kernel->state__cmd(ev, handler);
}

