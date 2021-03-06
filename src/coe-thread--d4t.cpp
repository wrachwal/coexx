// coe-thread--d4t.cpp

/*****************************************************************************
Copyright (c) 2008-2019 Waldemar Rachwał <waldemar.rachwal@gmail.com>

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

#include "coe-thread--d4t.h"
#include "coe-config--d4c.h"
#include "coe-kernel--r4k.h"
#include "coe-session--r4s.h"

#include <cerrno>
#include <cstdio>       // perror
#include <iostream>

#include <sys/select.h> // select() on Cygwin
#include <unistd.h>     // read() on Linux
#include <time.h>       // clock_gettime() on POSIX

using namespace std;
using namespace coe;

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
    msgpipe_flag(0),
    state(BUSY),
    io_requests(0)
{
}

// ---------------------------------------------------------------------------
// d4Thread::FdSet

d4Thread::FdSet::FdSet ()
:   max_fd(-1)
#if ! COEXX_SELECT_USE_FD_SET
,   lvec(NULL)
,   lvec_max(0)
#endif
{
}

d4Thread::FdSet::~FdSet ()
{
#if ! COEXX_SELECT_USE_FD_SET
    if (NULL != lvec) {
        free(lvec);
        lvec = NULL;
    }
    lvec_max = 0;
#endif
}

void d4Thread::FdSet::zero ()
{
    max_fd = -1;
#if COEXX_SELECT_USE_FD_SET
    FD_ZERO(&lval);
#else
    if (NULL != lvec) {
        memset(lvec, 0, lvec_max * sizeof(lvec[0]));
    }
#endif
}

#if ! COEXX_SELECT_USE_FD_SET
void d4Thread::FdSet::fit_fd (int fd)
{
    int word = fd / NFDBITS;
    if (word >= lvec_max) {
        assert(NFDBITS == 8 * sizeof(lvec[0]));
        int new_max = word + 1;
        lvec = (fd_mask*)realloc(lvec, new_max * sizeof(lvec[0]));
        while (lvec_max < new_max) {
            lvec[lvec_max ++] = 0;
        }
    }
}
#endif

void d4Thread::FdSet::add_fd (int fd)
{
    assert(fd >= 0);
#if COEXX_SELECT_USE_FD_SET
    assert(("coexx: fd >= FD_SETSIZE passed to FD_SET-based select backend",
            fd < FD_SETSIZE));
    FD_SET(fd, &lval);
#else
    fit_fd(fd);
    lvec[fd / NFDBITS] |= (1UL << (fd % NFDBITS));
#endif
    max_fd = max(fd, max_fd);
}

fd_set* d4Thread::FdSet::sel_set ()
{
#if COEXX_SELECT_USE_FD_SET
    return max_fd >= 0 ? &lval : NULL;
#else
    return max_fd >= 0 ? (fd_set*)lvec : NULL;
#endif
}

bool d4Thread::FdSet::fd_isset (int fd) const
{
#if COEXX_SELECT_USE_FD_SET
    return 0 != FD_ISSET(fd, &lval);
#else
    int     word = fd / NFDBITS;
    fd_mask mask = 1UL << (fd % NFDBITS);
    assert(word < lvec_max);
    return 0 != (lvec[word] & mask);
#endif
}

// ===========================================================================
// d4Thread

d4Thread::d4Thread (bool quit_check_enabled)
:   _handle(NULL),
    _event_loop_running(false),
    _quit_check_enabled(quit_check_enabled),
    _quit_check_pred(NULL),
    _current_kernel(NULL),
    _dispatched_alarm(NULL),
    _msgpipe_rfd(-1)
{
    _handle = new Thread;
    _handle->_d4thread = this;

    _timestamp = get_current_time();

    // initiate (user's) thread local storage
    const _TlsD*    head = _TlsD::registry();
    if (NULL != head) {
        _user_tls.resize(head->info.index + 1);
    }
}

// ------------------------------------

d4Thread::~d4Thread ()
{
    // close auxiliary descriptors (if opened)
    if (_msgpipe_rfd >= 0) {
        close(_msgpipe_rfd);
        _msgpipe_rfd = -1;
    }
    if (sched.msgpipe_wfd >= 0) {
        close(sched.msgpipe_wfd);
        sched.msgpipe_wfd = -1;
    }

    // deallocate (user's) thread local storage
    for (const _TlsD* info = _TlsD::registry(); NULL != info; info = info->next) {
        assert(info->info.index < _user_tls.size());
        void*   tls = _user_tls[info->info.index];
        if (NULL != tls) {
            (*info->info.destroy)(tls);
        }
    }

    delete _handle;
    _handle = NULL;
}

// ---------------------------------------------------------------------------

d4Thread* d4Thread::get_d4t_tls ()
{
    d4Config*   cfg = d4Config::instance();
    return static_cast<d4Thread*>(pthread_getspecific(cfg->key_d4t));
}

// ------------------------------------

void d4Thread::set_d4t_tls (d4Thread* d4t)
{
    d4Config*   cfg = d4Config::instance();
    int status = pthread_setspecific(cfg->key_d4t, (void*)d4t);
    if (status != 0) {
        perror("pthread_setspecific");  //TODO: appropriate error-handling
        abort();
    }
}

// ---------------------------------------------------------------------------

void* d4Thread::get_user_tls (const _TlsD* info)
{
    assert(info->info.index < _user_tls.size());
    void*   tls = _user_tls[info->info.index];
    if (NULL == tls) {
        tls = _user_tls[info->info.index] = (*info->info.create)();
    }
    return tls;
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
    RWLock::Guard   guard(glob.rwlock, RWLock::WRITE);

    r4k._kid = glob.kid_generator.generate_next(glob.kid_map);
    r4k._thread->_list_kernel.put_tail(&r4k);

    glob.kid_map[r4k._kid] = &r4k;
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

void d4Thread::run_event_loop (bool (*quit)(Thread&))
{
    if (! _event_loop_running) {

        _event_loop_running = true;
        _quit_check_pred    = quit;

        bool initial_quit_check_enabled = _quit_check_enabled;
        if ( initial_quit_check_enabled) {
            // --@@--
            RWLock::Guard   guard(glob.rwlock, RWLock::WRITE);
            // threads initially _quit_check_enabled need their _tid's
            // to be re-inserted in glob.tid_map when run_event_loop()
            // is called for the second and next times.
            glob.tid_map[_tid] = this;
        }

        do {

            EvCommon*   ev;
            while ((ev = dequeue_event()) != NULL) {
                ev->dispatch();
                _current_kernel = NULL;
            }

        } while (! _quit_loop_check());

        _quit_check_enabled = initial_quit_check_enabled;
        _event_loop_running = false;
    }
}

// ------------------------------------

bool d4Thread::_quit_loop_check ()
{
    assert(_list_kernel.empty());

    // There is no kernel, there were no events, but beware: a system event may
    // be queued in the meantime, including one which would transfer a foreign
    // kernel to this thread, so in the critical section we re-check _pending
    // queue emptiness, and if still empty, temporarily disable the thread in
    // the glob.tid_map, and then outside the critical section safely call
    // user's _quit_check_pred function.

    Tid_Map::iterator   t;
    {
        // --@@--
        RWLock::Guard   guard(glob.rwlock, RWLock::WRITE);

        if (! _pqueue.empty()) {    // got system event
            return false;
        }

        t = glob.tid_map.find(_tid);

        assert(t != glob.tid_map.end());
        assert(this == (*t).second);

        // To prevent _tid being reused (very unlikely condition) by the
        // tid_generator, only the value is reset, rather than the whole pair
        // removed.
        (*t).second = NULL;
    }

    assert(NULL != _quit_check_pred);

    if ((*_quit_check_pred)(*_handle)) {    // call user's predicate

        // --@@--
        RWLock::Guard   guard(glob.rwlock, RWLock::WRITE);

        glob.tid_map.erase(t);  // remove completely

        return true;
    }
    else {
        // --@@--
        RWLock::Guard   guard(glob.rwlock, RWLock::WRITE);

        assert(t != glob.tid_map.end());
        assert(NULL == (*t).second);

        (*t).second = this;     // restore

        return false;
    }
}

// ---------------------------------------------------------------------------

void d4Thread::enqueue_msg_event (EvMsg* evmsg)
{
    if (evmsg->prio_order() < 0) {
        assert(this == get_d4t_tls());

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
        if (! sched.msgpipe_flag) {
            ssize_t nbytes = write(sched.msgpipe_wfd, "@", 1);
                    nbytes = nbytes;
            assert(1 == nbytes);
            sched.msgpipe_flag = 1;
        }
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
    TimeSpec    upr_time = _timestamp + TimeSpec(0, 1);

    DueSidAid_Map::iterator upr =
        _dsa_map.upper_bound(DueSidAid_Key(upr_time, SiD::NONE(), AiD::NONE()));

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

            _timestamp = get_current_time();

            // pqueue alarms expired in the meantime
            DueSidAid_Map::iterator i;
            while ((i = _dsa_map.begin()) != _dsa_map.end() && _timestamp >= i->first.due) {
                i->second->dsa_iter(invalid_dsa_iter());
                _pqueue.put_tail(i->second);
                _dsa_map.erase(i);
            }

            _pqueue_pending_events();
            continue; ///
        }

        //TODO:
        // if there are idle events with priority <= of the last normal event:
        // - poll for current i/o and check timer events, and if there is none,
        // put idle(s) into _pqueue.
        // - mark this condition, so next time do normal block for i/o and timer
        // events, otherwise we'd eat all cpu for endless polling.
        //

        if (_quit_check_enabled) {
            if (_list_kernel.empty() && NULL != _quit_check_pred)
                return NULL;
        }
        else {
            if (! _list_kernel.empty())
                _quit_check_enabled = true;
        }

        /******************************
                    waiting...
         ******************************/

        sched.state = Sched::WAIT;          // no queued event (must wait)

        assert(sched.io_requests >= 0);
#if 0
        if (sched.io_requests)
            cout << "********** SELECT (" << sched.io_requests << ")" << endl;
        else
            cout << ";;;;;;;;;; COND-VAR" << endl;
#endif

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

                if (_dsa_map.empty()) {
                    sched.cond.wait(guard);
                }
                else {
                    TimeSpec due = (*_dsa_map.begin()).first.due;
                    sched.cond.timedwait(guard, due);
                }

                _pqueue_pending_events();

                _timestamp = get_current_time();

                if (! _dsa_map.empty() && _timestamp >= (*_dsa_map.begin()).first.due) {
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
            file descriptors sets
        ******************************/

        _fdset[IO_read ].zero();
        _fdset[IO_write].zero();
        _fdset[IO_error].zero();

        for (FdModeSid_Map::iterator i = _fms_map.begin(); i != _fms_map.end(); ++i) {
            EvIO*   evio = (*i).second;
            if (evio->active()) {
                _fdset[evio->mode()].add_fd(evio->fd());
            }
        }

        // highest descriptor including '_msgpipe_rfd' which...
        const int max_fd = max(_fdset[IO_read ].max_fd,
                           max(_fdset[IO_write].max_fd,
                           max(_fdset[IO_error].max_fd,
                               _msgpipe_rfd)));     // will be add_fd()'ed after assert(s)

#if COEXX_SELECT_USE_FD_SET
        // check if      _msgpipe_rfd not clobbered
        assert(0 == FD_ISSET(     _msgpipe_rfd, &_fdset[IO_read ].lval));
        assert(0 == FD_ISSET(     _msgpipe_rfd, &_fdset[IO_write].lval));
        assert(0 == FD_ISSET(     _msgpipe_rfd, &_fdset[IO_error].lval));
        // check if sched.msgpipe_wfd not clobbered
        assert(0 == FD_ISSET(sched.msgpipe_wfd, &_fdset[IO_read ].lval));
        assert(0 == FD_ISSET(sched.msgpipe_wfd, &_fdset[IO_write].lval));
        assert(0 == FD_ISSET(sched.msgpipe_wfd, &_fdset[IO_error].lval));
#else
        // align all lvec(s) to the longest, enlarged to assumption made in checks
        {
            int max_fd_and_pipes = max(max_fd,  // _msgpipe_rfd included in max_fd
                                       sched.msgpipe_wfd);
            _fdset[IO_read ].fit_fd(max_fd_and_pipes);
            _fdset[IO_write].fit_fd(max_fd_and_pipes);
            _fdset[IO_error].fit_fd(max_fd_and_pipes);
        }

        // check if      _msgpipe_rfd not clobbered
        {
            int     word =         _msgpipe_rfd / NFDBITS;
            fd_mask mask = 1UL << (_msgpipe_rfd % NFDBITS);
            assert(0 == (_fdset[IO_read ].lvec[word] & mask));
            assert(0 == (_fdset[IO_write].lvec[word] & mask));
            assert(0 == (_fdset[IO_error].lvec[word] & mask));
        }
        // check if sched.msgpipe_wfd not clobbered
        {
            int     word =         sched.msgpipe_wfd / NFDBITS;
            fd_mask mask = 1UL << (sched.msgpipe_wfd % NFDBITS);
            assert(0 == (_fdset[IO_read ].lvec[word] & mask));
            assert(0 == (_fdset[IO_write].lvec[word] & mask));
            assert(0 == (_fdset[IO_error].lvec[word] & mask));
        }
#endif

        // finally add '_msgpipe_rfd'
        _fdset[IO_read].add_fd(_msgpipe_rfd);

        /******************************
                    timeout
        ******************************/

        struct timeval  tmo;

        if (NULL != due) {
            TimeSpec    delta = *due - get_current_time();
            if (delta > TimeSpec::ZERO()) {
                delta += TimeSpec(0, 999);  // ns(s) to us(s) round up
                tmo.tv_sec  = delta.tv_sec;
                tmo.tv_usec = delta.tv_nsec / 1000;
            }
            else {  // polling
                tmo.tv_sec  = 0;
                tmo.tv_usec = 0;
            }
        }

        /******************************
                    select
        ******************************/

        int result = select(max_fd + 1,
                            _fdset[IO_read ].sel_set(),
                            _fdset[IO_write].sel_set(),
                            _fdset[IO_error].sel_set(),
                            (due ? &tmo : NULL));

        if (result == -1) {

            if (errno == EINTR) {

                /*
                 * From POSIX `select' page
                 * (http://www.opengroup.org/onlinepubs/9699919799/):
                 * On failure, the objects pointed to by the readfds,
                 * writefds, and errorfds arguments shall not be modified.
                 * If the timeout interval expires without the specified
                 * condition being true for any of the specified file
                 * descriptors, the objects pointed to by the readfds,
                 * writefds, and errorfds arguments shall have all bits set
                 * to 0.
                 *
                 * From Linux `select' manpage:
                 * On error, -1 is returned, and errno is set appropriately;
                 * the  sets  and  timeout become undefined, so do not rely
                 * on their contents after an error.
                 *
                 **/

                continue;       // goto (file descriptors sets)
            }

            //
            // diagnostics on EBADF and possibly EINVAL
            //
            int last_errno = errno;
            {
                char    errbuf[1024];
                // _msgpipe_rfd
                assert(_msgpipe_rfd >= 0);
                _fdset[IO_read].zero();
                _fdset[IO_read].add_fd(_msgpipe_rfd);
                struct timeval  no_wait = {0, 0};
                if (select(_msgpipe_rfd + 1, _fdset[IO_read].sel_set(), NULL, NULL, &no_wait) == -1) {
                    sprintf(errbuf, "select(internal-read-pipe-FD=%d)", _msgpipe_rfd);
                    perror(errbuf);
                }
                // user's fd(s)
                for (FdModeSid_Map::iterator i = _fms_map.begin(); i != _fms_map.end(); ++i) {
                    EvIO*   evio = (*i).second;
                    if (evio->active()) {
                        int fd = evio->fd();
                        assert(fd >= 0);
                        IO_Mode mode = evio->mode();
                        assert(mode >= 0 && mode < 3);
                        fd_set* selset[3] = {NULL, NULL, NULL};
                        FdSet*  oneset = &_fdset[mode];
                        oneset->zero();
                        oneset->add_fd(fd);
                        selset[mode] = oneset->sel_set();
                        struct timeval  no_wait = {0, 0};
                        if (select(fd + 1, selset[0], selset[1], selset[2], &no_wait) == -1) {
                            sprintf(errbuf, "select(FD=%d, MODE=%d, EV=%s)", fd, int(mode), evio->name().c_str());
                            perror(errbuf);
                        }
                    }
                }
            }
            errno = last_errno;
            perror("select");
            abort();
        }

        /******************************
                update (guarded)
        ******************************/

        // --@@--
        Mutex::Guard    guard(sched.mutex);

        _pqueue_pending_events();

        _timestamp = get_current_time();

        if (NULL != due && _timestamp >= *due) {
            _pqueue_expired_alarms();
        }

        if (result > 0) {

            // read a byte from the notification pipe
            if (_fdset[IO_read].fd_isset(_msgpipe_rfd)) {

                char    buf[16];
                ssize_t nbytes = read(_msgpipe_rfd, buf, 8);    // try 8, expected 1

                if (nbytes == -1) {
                    if (errno == EINTR || errno == EAGAIN) {
                        continue;       // goto (file descriptors sets)
                    }
                    perror("read(pipe)");
                    abort();
                }
                else {
                    if (nbytes != 1) {
                        for (int i = 0; i < nbytes; ++i)
                            buf[i] = buf[i] == '@' ? '@' : '.';
                        (cerr << "read(pipe): " << nbytes
                              << " bytes \"").write(buf, nbytes) << "\" read!" << endl;
                        abort();
                    }
                    sched.msgpipe_flag = 0;     // pipe emptied
                }

                -- result;
            }

            //
            // _pqueue_io_events
            //
            if (result > 0) {

                if (sched.msgpipe_flag) {
                    /*
                     * `msgpipe' might not luck into the select altogether with other
                     * descriptors, but definitely should do the next time.
                     * If the pipe has been signalled I increment the flag in order to
                     * isolate a potential application bug which emptying the
                     * pipe breaks its signalling capability.
                     */
                    sched.msgpipe_flag ++;
                }
                if (sched.msgpipe_flag > 2) {
                    /*
                     * If tests confirm this never occurs, or if so, only due to a bug
                     * as stated in the comment above, remove the following
                     * assert completely or raise an exception.
                     */
                    assert(sched.msgpipe_flag <= 2);
                    sched.msgpipe_flag = 0;
                }

                for (FdModeSid_Map::iterator i = _fms_map.begin(); i != _fms_map.end(); ++i) {
                    EvIO*   evio = (*i).second;
                    if (evio->active() && _fdset[evio->mode()].fd_isset(evio->fd())) {
                        _pqueue.put_tail(evio);
                    }
                }
            }
        }

        //
        // check the predicate
        //
        if (! _pqueue.empty()) {
            sched.state = Sched::BUSY;
            return;
        }

    } ///// for (;;) -- file descriptors sets
}

// ---------------------------------------------------------------------------

EvMsg* d4Thread::post_event (r4Kernel* source, SiD to, EvMsg* evmsg)
{
    if (NULL != source && to.kid() == source->_kid) {   // intra-kernel post

        r4Session*  session = source->_current_context->session;
        if (to != session->_sid) {  // check if not a `yield' case
            session = source->local.find_session(to);
        }

        if (NULL != session) {
            if (! session->local.stopper.isset()) {
                enqueue_event(source, session, evmsg);      // --@@--
                return NULL;
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

                    enqueue_event(target, session, evmsg);      // --@@--
                    return NULL;
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

    return evmsg;
}

// ------------------------------------

void d4Thread::enqueue_event (r4Kernel* target, r4Session* session, EvMsg* evmsg)
{
    // quite a bunch of paranoic checks
    assert(NULL != target);
    assert(NULL != target->_thread);
    assert(NULL != session);
    assert(target == session->_kernel);

    evmsg->target(session);     // target session must be always known

    target->_thread->enqueue_msg_event(evmsg);          // --@@--
}

// ---------------------------------------------------------------------------

AiD d4Thread::create_alarm (EvAlarm* evalm)
{
    r4Session*  session = evalm->target();
    assert(NULL != session);
    r4Kernel*   kernel  = session->_kernel;
    assert(NULL != kernel);

    // generate unique `aid' for the alarm event
    assert(! evalm->aid().isset());
    AiD aid = session->_aid_generator.generate_next(
                    AiDExistsPred(session->_list_alarm)
                );
    evalm->aid(aid);

    // insert alarm event in to due/sid/aid map (sorted by time due--main index key)
    TimeSpec        due = evalm->time_due();
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

    // s1a_iter() already assert'ed in r4Kernel::delete_alarm()
    kernel->_s1a_map.erase(evalm->s1a_iter());

    session->_list_alarm.remove(evalm);

    delete evalm;
}

// ------------------------------------

void d4Thread::adjust_alarm (EvAlarm* evalm, const TimeSpec& abs_time)
{
    assert(Sched::BUSY == sched.state);

    r4Session*  session = evalm->target();
    assert(NULL != session);
    r4Kernel*   kernel  = session->_kernel;
    assert(NULL != kernel);

    // just enqueued? - remove when it'd need to wait
    if (evalm->enqueued()) {
        if (_timestamp < abs_time) {
            _pqueue.remove(evalm);
        }
    }

    // erase from due/sid/aid map if present
    if (evalm->dsa_iter() != invalid_dsa_iter()) {
#if 1
        //XXX: extensive consistency check (enable only for debugging)
        DueSidAid_Key   dsa(evalm->time_due(), session->_sid, evalm->aid());
        assert(_dsa_map.find(dsa) == evalm->dsa_iter());
#endif
        _dsa_map.erase(evalm->dsa_iter());
    }

    // change expiration time now
    evalm->time_due(abs_time);

    // re-insert alarm event in to due/sid/aid map
    DueSidAid_Key   dsa(abs_time, session->_sid, evalm->aid());

    pair<DueSidAid_Map::iterator, bool> insert_dsa =
        this->_dsa_map.insert(DueSidAid_Map::value_type(dsa, evalm));
    assert(true == insert_dsa.second);

    evalm->dsa_iter(insert_dsa.first);  // store iterator for fast removal
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

        // update name, arg, and sender_state
        old_evio->name_change(new_evio->name());
        ValParam*   old_arg = old_evio->arg_change(new_evio->arg_change(NULL));
        if (NULL != old_arg) {
            old_arg->destroy();
        }
        old_evio->sender_state(new_evio->sender_state());

        new_evio->active(false);    // precondtion for safe delete
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

    FdModeSid_Key   fms(evio->fd(), evio->mode(), session->_sid);
    size_t  erased = _fms_map.erase(fms);
    assert(1 == erased);

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
    Mutex::Guard    s0_guard(tt[0]->sched.mutex);
    Mutex::Guard    s1_guard(tt[1]->sched.mutex);

    /**********************************
                    move
     **********************************/

    kernel->_thread = target;   // thread re-assignment

    //      source              import
    //      ======     --->     ===============
    // (1)  _list_kernel        import->_kernel
    // (2)  _lqueue             import->lqueue
    // (3)  _pqueue             import->pqueue
    // (4)  sched.pending       import->pqueue (appended)
    // (5)  _dsa_map            import->dsa_map
    // (6)  _fms_map            import->fms_map

    // (1)
    EvSys_Import_Kernel*    import = new EvSys_Import_Kernel(
                                                source->_list_kernel.remove(kernel)
                                            );

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

    // (1) import._kernel ---> _list_kernel
    _list_kernel.put_tail(import.kernel());

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

