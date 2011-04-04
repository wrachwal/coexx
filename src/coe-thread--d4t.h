// coe-thread--d4t.h

/*****************************************************************************
Copyright (c) 2008-2010 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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

#include "coe-thread.h"
#include "coe-kernel--r4k.h"

#include <vector>               // _user_tls

#ifndef COEXX_SELECT_USE_FD_SET
#   ifdef NFDBITS
#       define COEXX_SELECT_USE_FD_SET 0
#   else
#       define COEXX_SELECT_USE_FD_SET 1
#   endif
#endif

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

    explicit d4Thread (bool quit_check_enabled);
    ~d4Thread ();

    static d4Thread* get_d4t_tls ();
    static void      set_d4t_tls (d4Thread* d4t);

    void* get_user_tls (const _TlsD* info);

    static void allocate_kid (r4Kernel& r4k);
           void allocate_tid ();

    static TimeSpec get_current_time ();

    void run_event_loop (bool (*quit)(Thread&));
    bool _quit_loop_check ();

    void enqueue_msg_event (EvMsg* evmsg);
    void _wakeup_waiting_thread ();
    void _pqueue_pending_events ();
    void _pqueue_expired_alarms ();
    //// _pqueue_io_events -- not a separate function, see _select_io()

    EvCommon* dequeue_event ();
    void _select_io (const TimeSpec* due);

    static EvMsg*  post_event (r4Kernel* source,             SiD to, EvMsg* evmsg);
    static void enqueue_event (r4Kernel* target, r4Session* session, EvMsg* evmsg);

    AiD  create_alarm (EvAlarm* evalm);
    void delete_alarm (EvAlarm* evalm);
    void adjust_alarm (EvAlarm* evalm, const TimeSpec& abs_time);

    bool create_io_watcher (EvIO* evio);
    bool delete_io_watcher (int fd, IO_Mode mode, r4Session* session);
    void delete_io_watcher (EvIO* evio);
    bool  pause_io_watcher (int fd, IO_Mode mode, r4Session* session);
    bool resume_io_watcher (int fd, IO_Mode mode, r4Session* session);

    static void _export_kernel_local_data (r4Kernel*            kernel);
           void _import_kernel_local_data (EvSys_Import_Kernel& import);

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
    struct Sched {

        Sched ();

        Mutex               mutex;
        CondVar             cond;

        int                 msgpipe_wfd;
        int                 msgpipe_flag;

        enum State {
            BUSY,
            WAIT
        }                   state;

        _EvCommon::Queue    pending;

        int                 io_requests;

    } sched;

    // --------------------------------

    Thread*             _handle;
    TiD                 _tid;
    std::string         _tlabel;
    pthread_t           _os_thread;
    TimeSpec            _timestamp;

    bool                _event_loop_running;
    bool                _quit_check_enabled;

    bool              (*_quit_check_pred)(Thread&);

    std::vector<void*>  _user_tls;

    _r4Kernel::List     _list_kernel;

    r4Kernel*           _current_kernel;    // set by EvUser events
    EvAlarm*            _dispatched_alarm;  // set to track deletion

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
        FdSet ();
        ~FdSet ();
        int         max_fd;
#if COEXX_SELECT_USE_FD_SET
        fd_set      lval;
#else
        fd_mask*    lvec;
        int         lvec_max;
#endif
        // --------
        void        zero     ();
        void        add_fd   (int fd);
        fd_set*     sel_set  ();
        bool        fd_isset (int fd) const;

    }                   _fdset[3];

    int                 _msgpipe_rfd;
};

// ===========================================================================

} ///// namespace coe

#endif

