// coe-thread--d4t.h

#ifndef __COE_THREAD__D4T_H
#define __COE_THREAD__D4T_H

#include "coe-sys.h"
#include "coe--event.h"
#include "coe--util.h"
#include <map>

struct r4Kernel;
struct r4Session;

// =======================================================================

struct d4Thread {

    d4Thread ();

    void run_event_loop ();
    void      enque_event (EvCommon* ev);
    EvCommon* deque_event ();

    static bool anon_post__arg (SiD to, const std::string& ev, PostArg* pp);

    static d4Thread* get_tls_data ();
    static void      set_tls_data (d4Thread* d4t);

    static void* _thread_entry (void* arg);
           void  _allocate_tid ();
    static void  _allocate_kid (r4Kernel& r4k);
    static void  _allocate_sid (r4Session& r4s);

    // --------------------------------
    //         | rwlock |
    // thread  |  R | W |
    // ------------------
    //   all   |  R | W |
    //
    static struct Glob {

        RWLock                      rwlock;

        IdentGenerator<TiD>         tid_generator;
        std::map<TiD, d4Thread*>    tid_map;        //TODO: hash_map

        IdentGenerator<KiD>         kid_generator;
        std::map<KiD, r4Kernel*>    kid_map;        //TODO: hash_map

    } glob;

    // --------------------------------
    //         | rwlock |
    // thread  |  R | W |
    // ------------------
    //  local  |  0 | W |
    // foreign |  R | ! |
    //
    struct Local {
        RWLock                      rwlock;

        std::map<KiD, r4Kernel*>    kid_map;        //TODO: hash_map
        std::map<SiD, r4Session*>   sid_map;        //TODO: hash_map

    } local;

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

        int         io_requests;

        TimeSpec    timestamp;

    } sched;
};

// =======================================================================

#endif

