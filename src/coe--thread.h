// coe-kernel--r4k.h

#ifndef __COE__THREAD_H
#define __COE__THREAD_H

// =======================================================================

struct d4Thread {

    d4Thread ();

    // scheduler's data:
    // - local event queue
    // - *shared* event priority queue

    // storage for event types:
    // - alarms (reason: getting next time-due operation trivial)
    // - io watchers (reason: select/poll has all data in place)
    // NOTE: posted events are stored directly in scheduler queues
};

// =======================================================================

#endif

