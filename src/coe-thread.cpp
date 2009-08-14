// coe-thread.cpp

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

#include "coe-thread.h"
#include "coe-thread--d4t.h"
#include "coe-config--d4c.h"

#include <cerrno>

using namespace std;
using namespace coe;

// ---------------------------------------------------------------------------

namespace {
    struct _Arg {
        _Arg (d4Thread* d, bool (*q)(Thread&)) : data(d), quit(q) {}
        d4Thread*   data;       // owned by new thread
        bool      (*quit)(Thread&);
        Mutex       mutex;
        CondVar     cond;
        TiD         tid;        // predicate
    };
}

// ---------------------------------------------------------------------------

static void* coe_thread_entry (void* arg)
{
    _Arg*   init = (_Arg*)arg;

    d4Thread* d4t            = init->data;
    bool    (*quit)(Thread&) = init->quit;

    pthread_detach(pthread_self());
    d4Thread::set_d4t_tls(d4t);
    d4t->allocate_tid();            // --@@--

    // notify the creator
    {
        // --@@--
        Mutex::Guard    guard(init->mutex);

        init->tid = d4t->_tid;  // set predicate
        init->cond.signal();    // notify the caller

        // once signalled, `init' gets invalid going out of scope in the caller.
    }

    //
    // Now we can start running event loop (forever)
    // Before we enter dequeue_event(), the `sched.state' is in safe BUSY state.
    //
    d4t->run_event_loop(quit);

    return NULL;
}

// ===========================================================================
// Thread

TiD Thread::spawn_new (bool (*quit)(Thread&))
{
    // when d4Config singleton is being created, signal mask (among other things)
    // of the calling thread is modified, therefore to allow future threads
    // to inherit that changed signal mask, the singleton instantiation
    // should always happen in the main thread.
    d4Config*   cfg = d4Config::instance();
                cfg = cfg;  // suppress warning

    d4Thread*   d4t = new d4Thread(false);

    _Arg    arg(d4t, quit);

    // --@@--
    Mutex::Guard    guard(arg.mutex);

    int status = pthread_create(&d4t->_os_thread, NULL, &::coe_thread_entry, &arg);
    if (status != 0) {
        delete d4t;
        errno = status;
        return TiD::NONE();
    }

    while (! arg.tid.isset()) {
        arg.cond.wait(guard);
    }

    return arg.tid;
}

// ---------------------------------------------------------------------------

Thread::Thread ()
:   _d4thread(NULL)
{
}

TiD Thread::ID () const
{
    return _d4thread->_tid;
}

void Thread::run_event_loop (bool (*quit)(Thread&))
{
    _d4thread->run_event_loop(quit);
}

void* Thread::_get_user_tls (const _TlsD* data)
{
    return _d4thread->get_user_tls(data);
}

void* next_tls_info (void* iter, LocalStorageInfo& info)
{
    const _TlsD*    data = iter ? static_cast<_TlsD*>(iter)->next
                                : _TlsD::registry();
    if (NULL != data) {
        info = data->info;
    }

    return (void*)data;
}

// ===========================================================================
// _TlsD

const _TlsD* _TlsD::_register (_TlsD* data)
{
    static _TlsD*   head = NULL;

    if (NULL != data) {
        assert(NULL == data->next);
        data->info.index = head ? head->info.index + 1 : 0;
        data->next = head;
        head = data;
    }

    return head;
}

