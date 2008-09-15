// $Id$

/*************************************************************************
Copyright (c) 2008 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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

#ifndef __COE_THREAD__D4T_H
#define __COE_THREAD__D4T_H

#include "coe-sys.h"
#include "coe--event.h"
#include "coe--util.h"

struct r4Kernel;
struct r4Session;

// =======================================================================
// d4Thread

struct d4Thread {

    d4Thread ();

    void run_event_loop ();
    void      enque_event (EvCommon* ev);
    EvCommon* deque_event ();

    static TimeSpec get_current_time ();

    void _queue_expired_alarms();

    static bool anon_post_event (                 SiD to, EvMsg* evmsg);
    static bool      post_event (d4Thread* local, SiD to, EvMsg* evmsg);

    enum SetupAlarmMode {
        _DELAY_SET
    };
    AiD create_alarm (SetupAlarmMode mode, const TimeSpec& spec, EvAlarm* evalm);

    bool create_io_watcher (EvIO* evio);

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

        // --------

        r4Kernel* find_kernel (KiD kid) const;

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

        // --------

        r4Kernel*  find_kernel  (KiD kid) const;
        r4Session* find_session (SiD sid) const;

    } local;

    // --------------------------------
    //
    struct Sched {

        Sched ();

        Mutex   mutex;
        CondVar cond;
        int     msgpipe_wfd;

        enum State {
            BUSY,
            WAIT
        } state;

        EvCommonStore::Queue    pqueue; //TODO: change to priority queue

        int         io_requests;

        TimeSpec    timestamp;

        // --------

        struct Trans {
            EvCommonStore::Queue    queue;
            DueSidAid_Map           dsa_map;
            FdModeSid_Map           fms_map;
        } trans;

    } sched;

    // --------------------------------

    TiD             tid;
    pthread_t       os_thread;
    bool            _event_loop_running;

    EvCommonStore::Queue    _lqueue;
    int                     _msgpipe_rfd;

    /*
     * alarms
     */
    DueSidAid_Map   _dsa_map;

    /*
     * I/O
     */
    FdModeSid_Map   _fms_map;
};

// =======================================================================

#endif

