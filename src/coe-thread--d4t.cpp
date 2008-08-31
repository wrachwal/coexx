// coe-thread--d4t.cpp

#include "coe-thread.h"
#include "coe-thread--d4t.h"
#include <cerrno>

using namespace std;

// =======================================================================
// d4Thread

d4Thread::d4Thread ()
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
    //TODO: return next event or block on sched.cond
    return NULL;
}

// =======================================================================
// Thread

static void* _coe_thread (void* arg)
{
    return NULL;
}

TiD Thread::create_new ()
{
    auto_ptr<d4Thread>  d4t(new d4Thread);

    //TODO

    int status = pthread_create(&d4t->os_thread, NULL, &_coe_thread, d4t.get());
    if (status != 0) {
        errno = status;
        return TiD::NONE();
    }

    return TiD::NONE();
}

