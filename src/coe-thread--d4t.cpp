// coe-thread--d4t.cpp

#include "coe-thread.h"
#include "coe-thread--d4t.h"
#include "coe-config--d4c.h"
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

bool d4Thread::anon_post__arg (SiD to, const string& ev, PostArg* pp)
{
    //TODO: posting event from within an unrelated thread,
    //e.g. from others' apis thread providing data to a wheel
    return false;
}

// =======================================================================

namespace {
    struct _Arg {
        _Arg (d4Thread* d) : data(d), done(false) {}
        d4Thread*   data;       // owned by new thread
        Mutex       mutex;
        CondVar     cond;
        TiD         tid;        // result
        bool        done;       // predicate
    };
}

int g_TiD = 0;  //FIXME

void* d4Thread::_thread_entry (void* arg)
{
    _Arg*   init = (_Arg*)arg;

    d4Thread*   d4t = init->data;

    // no thread will join it
    pthread_detach(pthread_self());

    // associate d4t data with the thread
    d4Thread::set_tls_data(d4t);

    //
    // allocate `tid' before run_event_loop()
    //
    {
        RWLock::Guard   guard(glob.tid_rwlock, RWLock::WRITE);

        //TODO: find unique tid in glob.tid_map
        d4t->tid = TiD(++ ::g_TiD);

        init->tid = d4t->tid;   // set result
        init->done = true;      // set predicate
        init->cond.signal();    // notify the caller
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

    while (! arg.done) {
        arg.cond.wait(guard);
    }

    //TODO: pass/set errno if tid invalid
    return arg.tid;
}

