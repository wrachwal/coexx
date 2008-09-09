// coe-thread--d4t.cpp

/*************************************************************************
Copyright (c) 2008 Waldemar Rachwal

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

#include "coe-thread.h"
#include "coe-thread--d4t.h"
#include "coe-config--d4c.h"
#include "coe-kernel--r4k.h"
#include "coe-session--r4s.h"
#include <cerrno>

#include <iostream>
#include <unistd.h>     // sleep() on Linux

using namespace std;

d4Thread::Glob  d4Thread::glob;

// -----------------------------------------------------------------------
// d4Thread::Glob

r4Kernel* d4Thread::Glob::find_kernel (KiD kid) const
{
    map<KiD, r4Kernel*>::const_iterator i = kid_map.find(kid);
    return i == kid_map.end() ? NULL : (*i).second;
}

// -----------------------------------------------------------------------
// d4Thread::Local

r4Kernel* d4Thread::Local::find_kernel (KiD kid) const
{
    map<KiD, r4Kernel*>::const_iterator i = kid_map.find(kid);
    return i == kid_map.end() ? NULL : (*i).second;
}

r4Session* d4Thread::Local::find_session (SiD sid) const
{
    map<SiD, r4Session*>::const_iterator i = sid_map.find(sid);
    return i == sid_map.end() ? NULL : (*i).second;
}

// =======================================================================
// d4Thread

d4Thread::d4Thread ()
  : _event_loop_running(false)
{
}

d4Thread::Sched::Sched ()
  : state(BUSY),
    io_requests(0)
{
}

void d4Thread::run_event_loop ()
{
    if (! _event_loop_running) {
        _event_loop_running = true;

        EvCommon*   ev;
        while ((ev = deque_event()) != NULL) {
            ev->dispatch();
        }

        _event_loop_running = false;
    }
}

void d4Thread::enque_event (EvCommon* ev)
{
    if (ev->prio_order() < 0) {
        assert(this == get_tls_data());

        sched.lqueue.put_tail(ev);
    }
    else {
        // --@@--
        Mutex::Guard    guard(sched.mutex);

        if (Sched::WAIT == sched.state && sched.pqueue.empty()) {

            sched.pqueue.put_tail(ev);

            // wake up waiting thread
            if (sched.io_requests) {
                // write a byte to notification pipe
                //TODO
            }
            else {
                sched.cond.signal();
            }
        }
        else {
            sched.pqueue.put_tail(ev);
        }
    }
}

EvCommon* d4Thread::deque_event ()
{
    if (! sched.lqueue.empty()) {
        return sched.lqueue.get_head();
    }

    // --@@--
    Mutex::Guard    guard(sched.mutex);

    if (! sched.pqueue.empty()) {
        return sched.pqueue.get_head();
    }

    sched.state = Sched::WAIT;

    if (sched.io_requests) {
        //TODO: unlock guard, then wait on select()
        //TODO: static void Mutex::Guard::unlock (Mutex::Guard& guard);
        assert(0);
        return NULL;    //FIXME
    }
    else {

        while (sched.pqueue.empty()) {  // test predicate
            //TODO: wait/timedwait
            sched.cond.wait(guard);
        }

        sched.state = Sched::BUSY;

        return sched.pqueue.get_head();
    }
}

// -----------------------------------------------------------------------

bool d4Thread::anon_post_event (SiD to, EvMsg *evmsg)
{
    d4Thread*   thread = get_tls_data();
    if (NULL != thread) {
        if (thread->local.kid_map.find(to.kid()) != thread->local.kid_map.end()) {
            return post_event(thread, to, evmsg);
        }
    }
    return post_event(NULL, to, evmsg);
}

bool d4Thread::post_event (d4Thread* local, SiD to, EvMsg* evmsg)
{
    if (NULL != local) {

        r4Session*  session = local->local.find_session(to);
        if (NULL != session) {

            evmsg->target(session);

            // --@@--
            local->enque_event(evmsg);
            return true;
        }
    }
    else {
        // --@@--
        RWLock::Guard   guard(glob.rwlock, RWLock::READ);

        r4Kernel*   kernel = glob.find_kernel(to.kid());
        if (NULL != kernel &&
            NULL != kernel->_thread)
        {
            // --@@--
            d4Thread*   thread = kernel->_thread;
            RWLock::Guard   guard(thread->local.rwlock, RWLock::READ);

            r4Session*  session = thread->local.find_session(to);
            if (NULL != session) {

                evmsg->target(session);
                evmsg->source(NULL);    // source and target in different threads

                if (evmsg->prio_order() < 0) {
                    evmsg->prio_order(0);   //TODO: get from config
                }

                // --@@--
                thread->enque_event(evmsg);
                return true;
            }
        }
    }

    // `to' not found
    //errno = ???   //TODO
    delete evmsg;
    return false;
}

// -----------------------------------------------------------------------

AiD d4Thread::create_alarm (SetupAlarmMode mode, TimeSpec spec, EvAlarm* evalm)
{
    r4Session*  session = evalm->target();
    //TODO
    return AiD::NONE();
}

// -----------------------------------------------------------------------

d4Thread* d4Thread::get_tls_data ()
{
    d4Config*   cfg = d4Config::instance();
    return static_cast<d4Thread*>(pthread_getspecific(cfg->key_d4t));
}

void d4Thread::set_tls_data (d4Thread* d4t)
{
    d4Config*   cfg = d4Config::instance();
    int status = pthread_setspecific(cfg->key_d4t, (void*)d4t);
    if (status != 0) {
        perror("pthread_setspecific");  //TODO: appropriate error-handling
        abort();
    }
}

// =======================================================================

void d4Thread::_allocate_tid ()
{
    assert(tid == TiD::NONE());
    // --@@--
    RWLock::Guard   guard(glob.rwlock, RWLock::WRITE);

    tid = glob.tid_generator.generate_next(glob.tid_map);

    glob.tid_map[tid] = this;
}

void d4Thread::_allocate_kid (r4Kernel& r4k)
{
    assert(r4k._kid == KiD::NONE());
    assert(NULL != r4k._thread);

    // --@@--
    RWLock::Guard   g_guard(glob.rwlock, RWLock::WRITE);
    RWLock::Guard   l_guard(r4k._thread->local.rwlock, RWLock::WRITE);

    r4k._kid = glob.kid_generator.generate_next(glob.kid_map);

                  glob.kid_map[r4k._kid] = &r4k;
    r4k._thread->local.kid_map[r4k._kid] = &r4k;
}

void d4Thread::_allocate_sid (r4Session& r4s)
{
    assert(NULL != r4s._kernel);
    assert(NULL != r4s._kernel->_thread);

    r4Kernel*   kernel = r4s._kernel;
    d4Thread*   thread = kernel->_thread;

    // --@@--
    RWLock::Guard   guard(thread->local.rwlock, RWLock::WRITE);

    r4s._sid = kernel->_sid_generator.generate_next(thread->local.sid_map);

    thread->local.sid_map[r4s._sid] = &r4s;
}

// -----------------------------------------------------------------------

namespace {
    struct _Arg {
        _Arg (d4Thread* d) : data(d) {}
        d4Thread*   data;       // owned by new thread
        Mutex       mutex;
        CondVar     cond;
        TiD         tid;        // predicate
    };
}

void* d4Thread::_thread_entry (void* arg)
{
    _Arg*   init = (_Arg*)arg;

    d4Thread*   d4t = init->data;

    pthread_detach(pthread_self());
    d4Thread::set_tls_data(d4t);
    d4t->_allocate_tid();           // --@@--

    // notify the creator
    init->tid = d4t->tid;   // set predicate
    init->cond.signal();    // notify the caller

    //
    // Now we can start running event loop (forever)
    // Before we enter deque_event(), the `sched.state' is in safe BUSY state.
    //
    d4t->run_event_loop();

    return NULL;
}

// -----------------------------------------------------------------------

TiD Thread::spawn_new ()
{
    // when d4Config singleton is being created, signal mask (among other things)
    // of the calling thread is modified, therefore to allow future threads
    // to inherit that changed signal mask, the singleton instantiation
    // should always happen in the main thread.
    d4Config*   cfg = d4Config::instance();
                cfg = cfg;  // suppress warning

    d4Thread*   d4t = new d4Thread;

    _Arg    arg(d4t);

    Mutex::Guard    guard(arg.mutex);

    int status = pthread_create(&d4t->os_thread, NULL, &d4Thread::_thread_entry, &arg);
    if (status != 0) {
        delete d4t;
        errno = status;
        return TiD::NONE();
    }

    while (! arg.tid.valid()) {
        arg.cond.wait(guard);
    }

    return arg.tid;
}

