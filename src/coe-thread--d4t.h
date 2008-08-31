// coe-thread--d4t.h

#ifndef __COE_THREAD__D4T_H
#define __COE_THREAD__D4T_H

#include "coe--ev.h"
#include "coe-os.h"
#include <map>

// =======================================================================

struct d4Thread {

    d4Thread ();

    void run_event_loop ();
    void      enque_event (EvCommon* ev);
    EvCommon* deque_event ();

    // --------------------------------

    static struct Glob {

        RWLock                      tid_rwlock;
        std::map<TiD, d4Thread*>    tid_map;        //TODO: hash_map

        RWLock                      kid_rwlock;
        std::map<KiD, r4Kernel*>    kid_map;        //TODO: hash_map

    } glob;

    // --------------------------------

    RWLock                          sid_rwlock;
    std::map<SiD, r4Session*>       sid_map;        //TODO: hash_map

    // --------------------------------

    pthread_t                       os_thread;
    TiD                             tid;

    struct Sched {

        Sched () : state(BUSY), fired_time(0.0), io_requests(0) {}

        Mutex   mutex;
        CondVar cond;

        enum State {
            BUSY,
            WAIT
        } state;

        EvCommonList::Queue lqueue;
        EvCommonList::Queue pqueue; //TODO: change to priority queue

        double  fired_time;
        int     io_requests;

    } sched;
};

// =======================================================================

#endif

