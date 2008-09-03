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

    static bool anon_post__arg (SiD to, const std::string& ev, PostArg* pp);

    static void* _thread_entry (void* arg);
    static d4Thread* get_tls_data ();
    static void      set_tls_data (d4Thread* d4t);

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

        Sched ();

        Mutex   mutex;
        CondVar cond;

        enum State {
            BUSY,
            WAIT
        } state;

        EvCommonList::Queue lqueue;
        EvCommonList::Queue pqueue; //TODO: change to priority queue

        int io_requests;

        enum Timer_State {
            READY,
            ALARMED,
            EXPIRED
        } timer_state;
        //
        double  timestamp;
        double  timer_due;

    } sched;
};

// =======================================================================

#endif

