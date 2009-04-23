// coe-thread--d4t.cpp

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

#include "coe-thread.h"
#include "coe-thread--d4t.h"
#include "coe-config--d4c.h"
#include "coe-kernel--r4k.h"
#include "coe-session--r4s.h"
#include <cerrno>

#include <sys/select.h> // select() on Cygwin
#include <unistd.h>     // read() on Linux
#include <iostream>

using namespace std;
using namespace coe;

#define TABLEN(tab)     int(sizeof(tab) / sizeof((tab)[0]))

d4Thread::Glob  d4Thread::glob;

// ---------------------------------------------------------------------------
// d4Thread::Glob

d4Thread* d4Thread::Glob::find_thread (TiD tid) const
{
    Tid_Map::const_iterator i = tid_map.find(tid);
    return i == tid_map.end() ? NULL : (*i).second;
}

r4Kernel* d4Thread::Glob::find_kernel (KiD kid) const
{
    Kid_Map::const_iterator i = kid_map.find(kid);
    return i == kid_map.end() ? NULL : (*i).second;
}

// ---------------------------------------------------------------------------
// d4Thread::Sched

d4Thread::Sched::Sched ()
  : msgpipe_wfd(-1),
    state(BUSY),
    io_requests(0)
{
}

// ---------------------------------------------------------------------------
// d4Thread::FdSet

void d4Thread::FdSet::zero ()
{
    FD_ZERO(&lval);
    max_fd = -1;
}

void d4Thread::FdSet::add_fd (int fd)
{
    FD_SET(fd, &lval);
    max_fd = max(fd, max_fd);
}

fd_set* d4Thread::FdSet::sel_set ()
{
    return max_fd >= 0 ? &lval : NULL;
}

bool d4Thread::FdSet::fd_isset (int fd) const
{
    return 0 != FD_ISSET(fd, &lval);
}

// ===========================================================================
// d4Thread

d4Thread::d4Thread ()
  : _event_loop_running(false),
    _current_kernel(NULL),
    _msgpipe_rfd(-1)
{
    _timestamp = get_current_time();
}

// ------------------------------------

d4Thread::~d4Thread ()
{
    if (_msgpipe_rfd >= 0) {
        close(_msgpipe_rfd);
        _msgpipe_rfd = -1;
    }
    if (sched.msgpipe_wfd >= 0) {
        close(sched.msgpipe_wfd);
        sched.msgpipe_wfd = -1;
    }
}

// ---------------------------------------------------------------------------

d4Thread* d4Thread::get_tls_data ()
{
    d4Config*   cfg = d4Config::instance();
    return static_cast<d4Thread*>(pthread_getspecific(cfg->key_d4t));
}

// ------------------------------------

void d4Thread::set_tls_data (d4Thread* d4t)
{
    d4Config*   cfg = d4Config::instance();
    int status = pthread_setspecific(cfg->key_d4t, (void*)d4t);
    if (status != 0) {
        perror("pthread_setspecific");  //TODO: appropriate error-handling
        abort();
    }
}

// ---------------------------------------------------------------------------

void d4Thread::allocate_tid ()
{
    assert(_tid == TiD::NONE());
    // --@@--
    RWLock::Guard   guard(glob.rwlock, RWLock::WRITE);

    _tid = glob.tid_generator.generate_next(glob.tid_map);

    glob.tid_map[_tid] = this;
}

// ------------------------------------

void d4Thread::allocate_kid (r4Kernel& r4k)
{
    assert(r4k._kid == KiD::NONE());
    assert(NULL != r4k._thread);

    // --@@--
    RWLock::Guard   g_guard(              glob.rwlock, RWLock::WRITE);
    RWLock::Guard   l_guard(r4k._thread->local.rwlock, RWLock::WRITE);

    r4k._kid = glob.kid_generator.generate_next(glob.kid_map);

    glob.kid_map[r4k._kid] = &r4k;
    r4k._thread->local.list_kernel.put_tail(&r4k);
}

// ---------------------------------------------------------------------------

TimeSpec d4Thread::get_current_time ()
{
    //
    //FIXME:
    //  1) There is no CLOCK_MONOTONIC #define on Solaris.
    //  2) On Linux it is but clock_gettime() fails with 'Invalid argument' errno :(
    //  On these systems and on Cygwin CLOCK_REALTIME seems to be in SYNC
    //  with what pthread_cond_timedwait() expects when condvar is default,
    //  i.e. created without any specific attributes (like setting clock type,
    //  the option not available everywhere).
    //
    clockid_t   clock_id = CLOCK_REALTIME;
    TimeSpec    now;
    if (clock_gettime(clock_id, &now) != 0) {
        //TODO
        perror("clock_gettime");
        abort();
    }
    return now;
}

// ---------------------------------------------------------------------------

void d4Thread::run_event_loop ()
{
    if (! _event_loop_running) {
        _event_loop_running = true;

        EvCommon*   ev;
        while ((ev = dequeue_event()) != NULL) {
            ev->dispatch();
            _current_kernel = NULL;
        }

        _event_loop_running = false;
    }
}

// ---------------------------------------------------------------------------

void d4Thread::enqueue_msg_event (EvMsg* evmsg)
{
    if (evmsg->prio_order() < 0) {
        assert(this == get_tls_data());

        _lqueue.put_tail(evmsg);
    }
    else {
        // --@@--
        Mutex::Guard    guard(sched.mutex);

        sched.pending.put_tail(evmsg);

        if (   Sched::WAIT == sched.state
            && 1 == sched.pending.size())   // was empty
        {
            _wakeup_waiting_thread();
        }
    }
}

// ------------------------------------

void d4Thread::_wakeup_waiting_thread ()    // --@@--
{
    if (sched.io_requests) {
        // write a byte to notification pipe
        ssize_t nbytes = write(sched.msgpipe_wfd, "@", 1);
                nbytes = nbytes;
        assert(1 == nbytes);
    }
    else {
        sched.cond.signal();
    }
}

// ------------------------------------

void d4Thread::_pqueue_pending_events ()    // --@@--
{
    while (! sched.pending.empty()) {
        EvCommon*   event = sched.pending.get_head();
        assert(event->prio_order() >= 0);
        _pqueue.put_tail(event);
    }
}

// ------------------------------------

void d4Thread::_pqueue_expired_alarms ()    // --@@--
{
    // add 1ns to construct fine upper bound key
    TimeSpec    now = get_current_time() + TimeSpec(0, 1);

    DueSidAid_Map::iterator upr =
        _dsa_map.upper_bound(DueSidAid_Key(now, SiD::NONE(), AiD::NONE()));

    //FIXME: on linux i get this assert quite often; debug it.
#if 0
    assert(upr != _dsa_map.begin());
#endif

    for (DueSidAid_Map::iterator i = _dsa_map.begin(); i != upr; ++i) {
        (*i).second->dsa_iter(invalid_dsa_iter());
        _pqueue.put_tail((*i).second);
    }

    _dsa_map.erase(_dsa_map.begin(), upr);  // complexity at most O(log(size()) + N)
}

// ---------------------------------------------------------------------------

EvCommon* d4Thread::dequeue_event ()
{
    for (;;) {

        /******************************
                    taking
         ******************************/

        if (! _lqueue.empty()) {
            assert(Sched::BUSY == sched.state);
            _timestamp = get_current_time();
            return _lqueue.get_head();
        }

        if (! _pqueue.empty()) {
            assert(Sched::BUSY == sched.state);
            _timestamp = get_current_time();
            return _pqueue.get_head();
        }

        // --@@--
        Mutex::Guard    guard(sched.mutex);

        if (! sched.pending.empty()) {
            _pqueue_pending_events();
            continue;
        }

        //TODO:
        // if there are idle events with priority <= of the last normal event:
        // - poll for current i/o and check timer events, and if there is none,
        // put idle(s) into _pqueue.
        // - mark this condition, so next time do normal block for i/o and timer
        // events, otherwise we'd eat all cpu for endless polling.
        //

        /******************************
                    waiting...
         ******************************/

        sched.state = Sched::WAIT;          // no queued event (must wait)

        if (sched.io_requests) {

            if (_msgpipe_rfd < 0) {
                int fd[2];
                int status = pipe(fd);
                if (status != 0) {
                    perror("pipe");
                    abort();
                }
                _msgpipe_rfd      = fd[0];
                sched.msgpipe_wfd = fd[1];
            }

            Mutex::Guard::unlock(guard);    // `unlock' before select()

            if (_dsa_map.empty()) {
                _select_io(NULL);           // --@@--
            }
            else {
                TimeSpec due = (*_dsa_map.begin()).first.due;
                _select_io(&due);           // --@@--
            }

            //
            // _select_io() is in charge to change `sched.state'
            //
        }
        else {

            for (;;) {

                bool    time_expired = false;

                if (_dsa_map.empty()) {
                    sched.cond.wait(guard);
                }
                else {
                    TimeSpec due = (*_dsa_map.begin()).first.due;
                    if (! sched.cond.timedwait(guard, due)) {
                        time_expired = true;
                    }
                }

                _pqueue_pending_events();

                if (time_expired) {
                    _pqueue_expired_alarms();
                }

                if (! _pqueue.empty()) {
                    sched.state = Sched::BUSY;
                    break;
                }

            } ///// for (;;)
        }

    } ///// for (;;)
}

// ------------------------------------

void d4Thread::_select_io (const TimeSpec* due)
{
    assert(     _msgpipe_rfd >= 0);
    assert(sched.msgpipe_wfd >= 0);

    for (;;) {

        /******************************
                    select
         ******************************/

        for (int i = 0; i < TABLEN(_fdset); ++i)
            _fdset[i].zero();

        for (FdModeSid_Map::iterator i = _fms_map.begin(); i != _fms_map.end(); ++i) {
            EvIO*   evio = (*i).second;
            if (evio->active()) {
                _fdset[evio->mode()].add_fd(evio->fd());
            }
        }

        assert(0 == FD_ISSET(     _msgpipe_rfd, &_fdset[IO_read ].lval));
        assert(0 == FD_ISSET(sched.msgpipe_wfd, &_fdset[IO_write].lval));

        _fdset[IO_read].add_fd(_msgpipe_rfd);

        int max_fd = -1;
        for (int i = 0; i < TABLEN(_fdset); ++i)
            max_fd = max(max_fd, _fdset[i].max_fd);

        struct timeval  tmo;

        if (NULL != due) {
            TimeSpec    delta = *due - get_current_time();
            if (delta > TimeSpec::ZERO()) {
                delta += TimeSpec(0, 999);
                tmo.tv_sec  = delta.tv_sec;
                tmo.tv_usec = delta.tv_nsec / 1000;
            }
            else {
                tmo.tv_sec  = 0;
                tmo.tv_usec = 0;
            }
        }

        int result = select(max_fd + 1,
                            _fdset[IO_read ].sel_set(),
                            _fdset[IO_write].sel_set(),
                            _fdset[IO_error].sel_set(),
                            (due ? &tmo : NULL));

        if (result == -1 && EINTR == errno) {
            continue;
        }

        if (result < 0) {
            perror("select");   //TODO
            abort();
        }

        /******************************
                update (guarded)
         ******************************/

        // --@@--
        Mutex::Guard    guard(sched.mutex);

        _pqueue_pending_events();

        if (result == 0) {
            _pqueue_expired_alarms();
        }
        else {

            // read a byte from the notification pipe
            if (_fdset[IO_read].fd_isset(_msgpipe_rfd)) {
                char    buf[32];
                ssize_t nbytes = read(_msgpipe_rfd, buf, 8);    // try 8, but 1 expected
                        nbytes = nbytes;
                assert(1 == nbytes);
                -- result;
            }

            //
            // _pqueue_io_events
            //
            if (result > 0) {
                for (FdModeSid_Map::iterator i = _fms_map.begin(); i != _fms_map.end(); ++i) {
                    EvIO*   evio = (*i).second;
                    if (evio->active() && _fdset[evio->mode()].fd_isset(evio->fd())) {
                        _pqueue.put_tail(evio);
                    }
                }
            }
        }

        if (! _pqueue.empty()) {
            sched.state = Sched::BUSY;
            break;
        }

    } ///// for (;;)
}

// ---------------------------------------------------------------------------

bool d4Thread::post_event (r4Kernel* source, SiD to, EvMsg* evmsg)
{
    if (NULL != source && to.kid() == source->_kid) {   // intra-kernel post

        r4Session*  session = source->_current_context->session;
        if (to != session->_sid) {  // check if not a `yield' case
            session = source->local.find_session(to);
        }

        if (NULL != session) {
            if (! session->local.stopper.isset()) {
                return post_event(source, session, evmsg);  // --@@--
            }
        }
    }
    else {          // inter-kernel post

        // --@@--
        RWLock::Guard   guard(glob.rwlock, RWLock::READ);

        r4Kernel*   target = glob.find_kernel(to.kid());
        if (NULL != target &&
            NULL != target->_thread)
        {
            // --@@--
            RWLock::Guard   guard(target->local.rwlock, RWLock::READ);

            r4Session*  session = target->local.find_session(to);
            if (NULL != session) {

                if (! session->local.stopper.isset()) {

                    // inter-thread post

                    evmsg->source(NULL);

                    if (evmsg->prio_order() < 0) {
                        evmsg->prio_order(0);   //TODO: get from config
                    }

                    return post_event(target, session, evmsg);  // --@@--
                }
            }
        }
    }

#if 1
    // `to' not found
    //errno = ???
    {
        cerr << "---\nPOSTing (" << evmsg->name() << ") failed: target "
                                 << to << " not found\n"
             << "  sender " << evmsg->sender() << " at state "
                            << evmsg->sender_state() << "."
             << endl;
    }
#endif

    delete evmsg;
    return false;
}

// ------------------------------------

bool d4Thread::post_event (r4Kernel* target, r4Session* session, EvMsg* evmsg)
{
    // quite a bunch of paranoic checks
    assert(NULL != target);
    assert(NULL != target->_thread);
    assert(NULL != session);
    assert(target == session->_kernel);

    evmsg->target(session);     // target session must be always known

    target->_thread->enqueue_msg_event(evmsg);          // --@@--

    return true;
}

// ---------------------------------------------------------------------------

AiD d4Thread::create_alarm (SetupAlarmMode mode, const TimeSpec& ts, EvAlarm* evalm)
{
    r4Session*  session = evalm->target();
    assert(NULL != session);
    r4Kernel*   kernel  = session->_kernel;
    assert(NULL != kernel);

    // generate unique `aid' for the alarm event
    AiD aid = session->_aid_generator.generate_next(
                    AiDExistsPred(session->_list_alarm)
                );
    evalm->aid(aid);

    // calculate time `due' (optionally remove name-based alarms)
    TimeSpec    due;

    if (mode == _DELAY_SET) {
        //FIXME: get_current_time() or _timestamp
        due = _timestamp + ts;
    }
    else {
        assert(0);
        return AiD::NONE();
    }
    evalm->time_due(due);

    // insert alarm event in to due/sid/aid map (sorted by time due--main index key)
    DueSidAid_Key   dsa(due, session->_sid, aid);

    pair<DueSidAid_Map::iterator, bool> insert_dsa =
        this->_dsa_map.insert(DueSidAid_Map::value_type(dsa, evalm));
    assert(true == insert_dsa.second);

    evalm->dsa_iter(insert_dsa.first);  // store iterator for fast removal

    // insert alarm event in to (intra-kernel)sid/aid map
    Sid1Aid_Key     s1a(session->_sid, aid);

    pair<Sid1Aid_Map::iterator, bool>   insert_s1a =
        kernel->_s1a_map.insert(Sid1Aid_Map::value_type(s1a, evalm));
    assert(true == insert_s1a.second);

    evalm->s1a_iter(insert_s1a.first);  // store iterator for fast removal

    // add event to list in the session (event's owner)
    session->_list_alarm.put_head(evalm);

    return aid;
}

// ------------------------------------

void d4Thread::delete_alarm (EvAlarm* evalm)
{
    assert(Sched::BUSY == sched.state);

    r4Session*  session = evalm->target();
    assert(NULL != session);
    r4Kernel*   kernel  = session->_kernel;
    assert(NULL != kernel);

    if (evalm->enqueued()) {
        _pqueue.remove(evalm);
    }

    if (evalm->dsa_iter() != invalid_dsa_iter()) {
#if 1
        //XXX: extensive consistency check (enable only for debugging)
        DueSidAid_Key   dsa(evalm->time_due(), session->_sid, evalm->aid());
        assert(_dsa_map.find(dsa) == evalm->dsa_iter());
#endif
        _dsa_map.erase(evalm->dsa_iter());
    }

#if 1
    {
        //XXX: extensive consistency check (enable only for debugging)
        Sid1Aid_Key     s1a(session->_sid, evalm->aid());
        assert(kernel->_s1a_map.find(s1a) == evalm->s1a_iter());
    }
#endif
    kernel->_s1a_map.erase(evalm->s1a_iter());

    session->_list_alarm.remove(evalm);

    delete evalm;
}

// ---------------------------------------------------------------------------

bool d4Thread::create_io_watcher (EvIO* new_evio)
{
    assert(Sched::BUSY == sched.state);

    r4Session*  session = new_evio->target();
    assert(NULL != session);
    short       fd      = new_evio->fd();
    IO_Mode     mode    = new_evio->mode();

    EvIO*   old_evio = session->find_io_watcher(fd, mode);

    if (NULL != old_evio) {
        delete old_evio->arg(new_evio->arg(NULL));
        delete new_evio;
    }
    else {
        FdModeSid_Key   fms(fd, mode, session->_sid);

        pair<FdModeSid_Map::iterator, bool> insert =
            _fms_map.insert(FdModeSid_Map::value_type(fms, new_evio));
        assert(true == insert.second);

        session->_list_evio.put_tail(new_evio);

        ++ sched.io_requests;
    }

    return true;
}

// ------------------------------------

bool d4Thread::delete_io_watcher (int fd, IO_Mode mode, r4Session* session)
{
    assert(Sched::BUSY == sched.state);
    assert(NULL != session);

    EvIO*   evio = session->find_io_watcher(fd, mode);
    if (NULL == evio) {
        //errno = ???
        return false;
    }
    assert(evio->target() == session);
    assert(evio->fd()     == fd);
    assert(evio->mode()   == mode);

    delete_io_watcher(evio);
    return true;
}

// ------------------------------------

void d4Thread::delete_io_watcher (EvIO* evio)
{
    assert(Sched::BUSY == sched.state);

    r4Session*  session = evio->target();
    assert(NULL != session);

    if (evio->enqueued()) {
        _pqueue.remove(evio);
    }

    if (evio->active()) {
        evio->active(false);
        -- sched.io_requests;
        assert(sched.io_requests >= 0);
    }

    _fms_map.erase(FdModeSid_Key(evio->fd(), evio->mode(), session->_sid));

    session->_list_evio.remove(evio);

    delete evio;
}

// ------------------------------------

bool d4Thread::pause_io_watcher (int fd, IO_Mode mode, r4Session* session)
{
    assert(Sched::BUSY == sched.state);
    assert(NULL != session);

    EvIO*   evio = session->find_io_watcher(fd, mode);
    if (NULL == evio) {
        //errno = ???
        return false;
    }

    if (evio->enqueued()) {
        _pqueue.remove(evio);
    }

    if (evio->active()) {
        evio->active(false);
        -- sched.io_requests;
        assert(sched.io_requests >= 0);
    }

    return true;
}

// ------------------------------------

bool d4Thread::resume_io_watcher (int fd, IO_Mode mode, r4Session* session)
{
    assert(NULL != session);
    assert(Sched::BUSY == sched.state);

    EvIO*   evio = session->find_io_watcher(fd, mode);
    if (NULL == evio) {
        //errno = ???
        return false;
    }

    if (! evio->active()) {
        evio->active(true);
        assert(sched.io_requests >= 0);
        ++ sched.io_requests;
    }

    return true;
}

// ---------------------------------------------------------------------------

void d4Thread::_export_kernel_local_data (r4Kernel* kernel)
{
    assert(NULL != kernel);
    TiD         target_tid = kernel->_target_thread;

    kernel->_target_thread = TiD::NONE();   // reset member data

    d4Thread*   source     = kernel->_thread;
    TiD         source_tid = source->_tid;

    if (! target_tid.isset() || target_tid == source_tid) {
        return;
    }

    /**********************************
            lock (hierarchy)
     **********************************/

    // --@@--
    // RWLock::WRITE due to kernel's thread re-assignment
    RWLock::Guard   gg_guard(glob.rwlock, RWLock::WRITE);

    d4Thread*   target = glob.find_thread(target_tid);
    if (NULL == target) {
        return;
    }

    // enforce strict locking hierarchy between two (equivalent) threads
    d4Thread*   tt[2] = { source, target };
    assert(tt[0] != tt[1]);
    if (tt[1] < tt[0]) {
        swap(tt[0], tt[1]); // order based on d4Thread* values
    }

    // --@@--
    RWLock::Guard   l0_guard(tt[0]->local.rwlock, RWLock::WRITE);
    RWLock::Guard   l1_guard(tt[1]->local.rwlock, RWLock::WRITE);

    // --@@--
    Mutex::Guard    s0_guard(tt[0]->sched.mutex);
    Mutex::Guard    s1_guard(tt[1]->sched.mutex);

    /**********************************
                    move
     **********************************/

    kernel->_thread = target;   // thread re-assignment

    //      source              target / import
    //      ======     --->     ===============
    // (1)  local.list_kernel   target->local.list_kernel
    // (2)  _lqueue             import->lqueue
    // (3)  _pqueue             import->pqueue
    // (4)  sched.pending       import->pqueue (appended)
    // (5)  _dsa_map            import->dsa_map
    // (6)  _fms_map            import->fms_map

    EvSys_Import_Kernel*    import = new EvSys_Import_Kernel(kernel);

    // (1)
    target->local.list_kernel.put_tail(
        source->local.list_kernel.remove(kernel));
    assert(target->local.list_kernel.peek_tail() == kernel);

    // (2)
    for (_EvCommon::Queue::iterator i = source->_lqueue.begin();
         i != source->_lqueue.end();
         /*empty*/)
    {
        if ((*i)->is_event_of(kernel->_kid)) {
            import->lqueue.put_tail(
                source->_lqueue.remove(*i++));
        }
        else {
            ++i;
        }
    }

    // (3)
    for (_EvCommon::Queue::iterator i = source->_pqueue.begin();
         i != source->_pqueue.end();
         /*empty*/)
    {
        if ((*i)->is_event_of(kernel->_kid)) {
            import->pqueue.put_tail(
                source->_pqueue.remove(*i++));
        }
        else {
            ++i;
        }
    }

    // (4)
    for (_EvCommon::Queue::iterator i = source->sched.pending.begin();
         i != source->sched.pending.end();
         /*empty*/)
    {
        if ((*i)->is_event_of(kernel->_kid)) {
            import->pqueue.put_tail(
                source->sched.pending.remove(*i++));
        }
        else {
            ++i;
        }
    }

    // (5)
    for (DueSidAid_Map::iterator i = source->_dsa_map.begin();
         i != source->_dsa_map.end();
         /*empty*/)
    {
        if ((*i).first.sid.kid() == kernel->_kid) {
            import->dsa_map.insert(*i);
            source->_dsa_map.erase(i++);
        }
        else {
            ++i;
        }
    }

    // (6)
    for (FdModeSid_Map::iterator i = source->_fms_map.begin();
         i != source->_fms_map.end();
         /*empty*/)
    {
        if ((*i).first.sid.kid() == kernel->_kid) {
            import->fms_map.insert(*i);
            if ((*i).second->active())
                -- source->sched.io_requests;
            source->_fms_map.erase(i++);
        }
        else {
            ++i;
        }
    }

    //
    // signal target thread to import the kernel
    //
    target->sched.pending.put_head(import);

    if (   Sched::WAIT == target->sched.state
        && 1 == target->sched.pending.size())   // was empty
    {
        target->_wakeup_waiting_thread();
    }
}

// ------------------------------------

void d4Thread::_import_kernel_local_data (EvSys_Import_Kernel& import)
{
    // --@@--
    Mutex::Guard    guard(sched.mutex);

    // (2) import.lqueue ---> _lqueue
    while (! import.lqueue.empty()) {
        _lqueue.put_tail(import.lqueue.get_head());
    }

    // (3+4) import.pqueue ---> _pqueue
    while (! import.pqueue.empty()) {
        _pqueue.put_tail(import.pqueue.get_head());
    }

    // (5) import.dsa_map ---> _dsa_map
    for (DueSidAid_Map::iterator i = import.dsa_map.begin();
         i != import.dsa_map.end();
         ++i)
    {
        pair<DueSidAid_Map::iterator, bool> insert = _dsa_map.insert(*i);
        (*i).second->dsa_iter(insert.first);
    }

    // (6) import.fms_map ---> _fms_map
    for (FdModeSid_Map::iterator i = import.fms_map.begin();
         i != import.fms_map.end();
         ++i)
    {
        _fms_map.insert(*i);
        if ((*i).second->active())
            ++ sched.io_requests;
    }
}

// ---------------------------------------------------------------------------

namespace {
    struct _Arg {
        _Arg (d4Thread* d) : data(d) {}
        d4Thread*   data;       // owned by new thread
        Mutex       mutex;
        CondVar     cond;
        TiD         tid;        // predicate
    };
}

// ------------------------------------

void* d4Thread::_thread_entry (void* arg)
{
    _Arg*   init = (_Arg*)arg;

    d4Thread*   d4t = init->data;

    pthread_detach(pthread_self());
    d4Thread::set_tls_data(d4t);
    d4t->allocate_tid();            // --@@--

    // notify the creator
    init->tid = d4t->_tid;  // set predicate
    init->cond.signal();    // notify the caller

    //
    // Now we can start running event loop (forever)
    // Before we enter dequeue_event(), the `sched.state' is in safe BUSY state.
    //
    d4t->run_event_loop();

    return NULL;
}

// ===========================================================================

TiD Thread::spawn_new ()
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

    int status = pthread_create(&d4t->_os_thread, NULL, &d4Thread::_thread_entry, &arg);
    if (status != 0) {
        delete d4t;
        errno = status;
        return TiD::NONE();
    }

    while (! arg.tid.isset()) {
        arg.cond.wait(guard);
    }

    return arg.tid;
}

