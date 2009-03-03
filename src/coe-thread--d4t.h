// coe-thread--d4t.h

/*****************************************************************************
Copyright (c) 2008, 2009 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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
*****************************************************************************/

#ifndef __COE_THREAD__D4T_H
#define __COE_THREAD__D4T_H

#include "coe-kernel--r4k.h"

namespace coe { /////

// ---------------------------------------------------------------------------

struct d4Thread;
struct r4Kernel;
struct r4Session;

typedef std::map<TiD, d4Thread*> Tid_Map;
typedef std::map<KiD, r4Kernel*> Kid_Map;

// ===========================================================================
// d4Thread

struct d4Thread {

    d4Thread ();
    ~d4Thread ();

    static d4Thread* get_tls_data ();
    static void      set_tls_data (d4Thread* d4t);

    static void allocate_kid (r4Kernel& r4k);
           void allocate_tid ();

    static TimeSpec get_current_time ();

    void run_event_loop ();

    void enqueue_msg_event (EvMsg* evmsg);
    void _wakeup_waiting_thread ();
    void _pqueue_pending_events ();
    void _pqueue_expired_alarms ();

    EvCommon* dequeue_event ();
    void _select_io (const TimeSpec* due);

    static bool post_event (r4Kernel* source,             SiD to, EvMsg* evmsg);
    static bool post_event (r4Kernel* target, r4Session* session, EvMsg* evmsg);

    enum SetupAlarmMode {
        _DELAY_SET
    };
    AiD create_alarm (SetupAlarmMode mode, const TimeSpec& spec, EvAlarm* evalm);

    bool create_io_watcher (EvIO* evio);
    bool delete_io_watcher (int fd, IO_Mode mode, r4Session* session);
    bool  pause_io_watcher (int fd, IO_Mode mode, r4Session* session);
    bool resume_io_watcher (int fd, IO_Mode mode, r4Session* session);

    static void _export_kernel_local_data (r4Kernel*            kernel);
           void _import_kernel_local_data (EvSys_Import_Kernel& import);

    static void* _thread_entry (void* arg);

    // --------------------------------
    //
    static struct Glob {

        RWLock              rwlock;

        IdentGenerator<TiD> tid_generator;
        Tid_Map             tid_map;        //TODO: hash_map

        IdentGenerator<KiD> kid_generator;
        Kid_Map             kid_map;        //TODO: hash_map

        // --------

        d4Thread* find_thread (TiD kid) const;
        r4Kernel* find_kernel (KiD kid) const;

    } glob;

    // --------------------------------
    //
    struct Local {

        RWLock              rwlock;

        _r4Kernel::List     list_kernel;

    } local;

    // --------------------------------
    //
    struct Sched {

        Sched ();

        Mutex               mutex;
        CondVar             cond;
        int                 msgpipe_wfd;

        enum State {
            BUSY,
            WAIT
        }                   state;

        _EvCommon::Queue    pending;

        int                 io_requests;

    } sched;

    // --------------------------------

    TiD                 _tid;
    pthread_t           _os_thread;
    TimeSpec            _timestamp;

    bool                _event_loop_running;
    r4Kernel*           _current_kernel;    // may be set by an event

    /*
     * event local queue(s)
     */
    _EvCommon::Queue    _lqueue;    // local, intra-kernel messages
    _EvCommon::Queue    _pqueue;    // priority queue

    /*
     * alarms
     */
    DueSidAid_Map       _dsa_map;

    /*
     * I/O
     */
    FdModeSid_Map       _fms_map;

    struct FdSet {
        fd_set  lval;
        int     max_fd;
        // --------
        void    zero     ();
        void    add_fd   (int fd);
        fd_set* sel_set  ();
        bool    fd_isset (int fd) const;

    }                   _fdset[3];

    int                 _msgpipe_rfd;

};

// ===========================================================================

} ///// namespace coe

#endif

