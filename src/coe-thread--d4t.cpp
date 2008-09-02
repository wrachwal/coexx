// coe-thread--d4t.cpp

#include "coe-thread.h"
#include "coe-thread--d4t.h"
#include "coe-config--d4c.h"
#include <cerrno>

#include <iostream>

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
    io_requests(0),
    timer_state(READY),
    timestamp(0.0),
    timer_due(0.0)
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
        cout << "thread #" << tid.nid() << " is running." << endl;
        sleep(5);
    }
    return NULL;    // should never happen
}

// =======================================================================

namespace {
    struct _Arg {
        _Arg (d4Thread* d) : data(d) {}
        d4Thread*   data;
        TiD         tid;        // predicate: tid.valid()
        Mutex       mutex;
        CondVar     cond;
    };
}

void* d4Thread::_thread_entry (void* arg)
{
    _Arg*   init = (_Arg*)arg;

    d4Thread*   d4t = init->data;
    d4Config*   cfg = d4Config::instance(); // already done in calling thread

    // set TLS
    int status = pthread_setspecific(cfg->key_d4t, (void*)d4t);
    if (status != 0) {
        //TODO: appropriate error-handling
        perror("pthread_setspecific");
        abort();
    }

    //
    // critical-section before run_event_loop()
    //
    {
        RWLock::Guard   guard(glob.tid_rwlock, RWLock::WLock);

        //TODO: find unique tid in glob.tid_map
        static int  t = 0;
        d4t->tid = TiD(++ t);

        init->tid = d4t->tid;   // set predicate
        init->cond.signal();    // ...and notify the caller
    }

    //
    // Now we can start running event loop (forever)
    // Before we enter deque_event(), the `sched.state' is in safe BUSY state.
    //
    d4t->run_event_loop();

    return NULL;
}

// -----------------------------------------------------------------------

TiD Thread::create_new ()
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

