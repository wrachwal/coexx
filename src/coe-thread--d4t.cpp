// coe-thread--d4t.cpp

#include "coe-thread.h"
#include "coe-thread--d4t.h"
#include "coe-config--d4c.h"
#include "coe-kernel--r4k.h"
#include "coe-session--r4s.h"
#include <cerrno>

#include <iostream>
#include <unistd.h>     // sleep() on Linux

using namespace std;

// -----------------------------------------------------------------------

d4Thread::Glob  d4Thread::glob;

// =======================================================================
// d4Thread

d4Thread::d4Thread ()
{
}

d4Thread::Sched::Sched ()
  : state(BUSY),
    io_requests(0)
{
}

void d4Thread::run_event_loop ()
{
    EvCommon*   ev;
    while ((ev = deque_event()) != NULL) {
        ev->dispatch();     //TODO: should take d4Thread as param
    }
}

void d4Thread::enque_event (EvCommon* ev)
{
    //TODO: put to local or foreign thread sched queue
}

EvCommon* d4Thread::deque_event ()
{
    //TODO
    while(1) {
        cout << "thread #" << tid.id() << " is running." << endl;
        sleep(5);
    }
    return NULL;    // should never happen
}

// -----------------------------------------------------------------------

bool d4Thread::anon_post__arg (SiD to, const string& ev, PostArg* pp)
{
    //TODO: posting event from within an unrelated thread,
    //e.g. from others' apis thread providing data to a wheel
    return false;
}

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

