// coe-kernel.h

#ifndef __COE_KERNEL_H
#define __COE_KERNEL_H

#include "coe-ident.h"
#include "coe-kernel--dcl.h"
#include <time.h>       // timespec

class Kernel;
class Session;

struct r4Kernel;

// handlers' context
class                   EvCtx;
template<class> struct TEvCtx;
class                   DatIO;

struct TimeSpec;

// -----------------------------------------------------------------------

enum IO_Mode {
    IO_read  = 0,
    IO_write = 1,
    IO_error = 2
};

// =======================================================================
// Kernel

class Kernel {
public:
    static Kernel& create_new ();

    void run_event_loop ();
    bool run_event_loop (TiD tid);

    SiD start_session (Session*);

    /*
     * post (SiD, EV_NAME[, PARAM...])
     */
           bool      post (SiD to, const std::string& ev, PostArg* pp=0);
    static bool anon_post (SiD to, const std::string& ev, PostArg* pp=0);

    /*
     * yield (EV_NAME[, PARAM...]])
     */
    bool yield (const std::string& ev, PostArg* pp=0);

    /*
     * call (SiD, EV_NAME[, PARAM...])
     */
    bool call (SiD on, const std::string& ev, CallArg* cp);

    /*
     * Name-Based Timers
     * =================
     * alarm     (EV_NAME)                          -- reset
     * alarm     (EV_NAME, ABS_TIME[, PARAM...])
     * alarm_add (EV_NAME, ABS_TIME[, PARAM...])
     * delay     (EV_NAME, DURATION_SECS[, PARAM...])
     * delay_add (EV_NAME, DURATION_SECS[, PARAM...])
     */
    bool alarm     (const std::string ev);          // reset
    bool alarm     (const std::string ev, TimeSpec abs_time, PostArg* pp=0);
    bool alarm_add (const std::string ev, TimeSpec abs_time, PostArg* pp=0);
    bool delay     (const std::string ev, TimeSpec duration, PostArg* pp=0);
    bool delay_add (const std::string ev, TimeSpec duration, PostArg* pp=0);

    /*
     * Identifier-Based Timers
     * =======================
     * alarm_remove_all ()
     * alarm_remove (TID)
     * alarm_set    (EV_NAME, EPOCH_TIME[, PARAM...])
     * alarm_adjust (TID, DELTA_SECS)
     * delay_set    (EV_NAME, DURATION_SECS[, PARAM...])
     * delay_adjust (EV_NAME, SECS_FROM_NOW)
     */

    /*
     * select        (FD, MODE)                 -- reset
     * select        (FD, MODE, EV_NAME[, PARAM...])
     * select_pause  (FD, MODE)                 ## MODE := IO_read | IO_write
     * select_resume (FD, MODE)                 ## MODE := IO_read | IO_write
     */
    bool select (int fd, IO_Mode mode);         // reset
    bool select (int fd, IO_Mode mode, const std::string ev, PostArg* pp=0);
    bool select_pause  (int fd, IO_Mode mode);
    bool select_resume (int fd, IO_Mode mode);

    /*
     * state (EV_NAME)                          -- reset
     * state (EV_NAME, OBJECT, METHOD)
     * state (EV_NAME, FUNCTION)
     */
    void state (const std::string& ev);         // reset
    void state (const std::string& ev, StateCmd* handler);

private:
    Kernel ();
    Kernel (const Kernel&);         // prohibited
    void operator= (const Kernel&); // prohibited

    friend struct r4Kernel;
    r4Kernel    *_r4kernel;
};

// =======================================================================
// EvCtx

class EvCtx {
public:
    void*       heap;
    Kernel&     kernel;
    Session&    session;
    std::string state;
    SiD         sender;
    std::string sender_state;
    //TODO:     timer;      // set if timer event
private:
    friend struct r4Kernel;
    EvCtx (Kernel& k, Session& s);
    EvCtx (const EvCtx&);           // prohibited
    void operator= (const EvCtx&);  // prohibited
};

// ------------------------------------
// TEvCtx<Heap>

template<class Heap>
struct TEvCtx : public EvCtx {
    TEvCtx ();
    operator bool () const { return NULL != heap; }
    Heap* operator-> () const { return static_cast<Heap*>(heap); }
    operator Heap* () const { return static_cast<Heap*>(heap); }
};

// ------------------------------------
// DatIO

class DatIO {
public:
    const int       filedes;
    const IO_Mode   mode;
private:
    friend struct r4Kernel;
    DatIO (int f, IO_Mode m);
    DatIO (const DatIO&);           // prohibited
    void operator= (const EvCtx&);  // prohibited
};

// =======================================================================
// TimeSpec

struct TimeSpec : public timespec {
             TimeSpec ()                      { tv_sec = 0;   tv_nsec = 0; }
    explicit TimeSpec (const timespec& ts);
    explicit TimeSpec (double sec);
    explicit TimeSpec (time_t sec)            { tv_sec = sec; tv_nsec = 0; }
             TimeSpec (time_t sec, long nsec) { tv_sec = sec; tv_nsec = nsec; }
};

// =======================================================================
// vparam (p1[, ...])

template<class P1>
PostArg* vparam (const P1&);
template<class P1, class P2>
PostArg* vparam (const P1&, const P2&);
template<class P1, class P2, class P3>
PostArg* vparam (const P1&, const P2&, const P3&);
template<class P1, class P2, class P3, class P4>
PostArg* vparam (const P1&, const P2&, const P3&, const P4&);
template<class P1, class P2, class P3, class P4, class P5>
PostArg* vparam (const P1&, const P2&, const P3&, const P4&, const P5&);

// -----------------------------------------------------------------------
// rparam (p1[, ...])

template<class P1>
CallArg* rparam (P1&);
template<class P1, class P2>
CallArg* rparam (P1&, P2&);
template<class P1, class P2, class P3>
CallArg* rparam (P1&, P2&, P3&);
template<class P1, class P2, class P3, class P4>
CallArg* rparam (P1&, P2&, P3&, P4&);
template<class P1, class P2, class P3, class P4, class P5>
CallArg* rparam (P1&, P2&, P3&, P4&, P5&);

// -----------------------------------------------------------------------
// handler (obj, memfun)
// handler (fun)

template<class Obj>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(EvCtx&));
template<class Obj, class P1>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(EvCtx&, P1&));
template<class Obj, class P1, class P2>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(EvCtx&, P1&, P2&));
template<class Obj, class P1, class P2, class P3>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(EvCtx&, P1&, P2&, P3&));
template<class Obj, class P1, class P2, class P3, class P4>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(EvCtx&, P1&, P2&, P3&, P4&));
template<class Obj, class P1, class P2, class P3, class P4, class P5>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(EvCtx&, P1&, P2&, P3&, P4&, P5&));

template<class Heap, class Obj>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&));
template<class Heap, class Obj, class P1>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, P1&));
template<class Heap, class Obj, class P1, class P2>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, P1&, P2&));
template<class Heap, class Obj, class P1, class P2, class P3>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, P1&, P2&, P3&));
template<class Heap, class Obj, class P1, class P2, class P3, class P4>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, P1&, P2&, P3&, P4&));
template<class Heap, class Obj, class P1, class P2, class P3, class P4, class P5>
StateCmd* handler (Obj& obj, void (Obj::*memfun)(TEvCtx<Heap>&, P1&, P2&, P3&, P4&, P5&));

// ------------------------------------

StateCmd* handler (void (*fun)(EvCtx&));
template<class P1>
StateCmd* handler (void (*fun)(EvCtx&, P1&));
template<class P1, class P2>
StateCmd* handler (void (*fun)(EvCtx&, P1&, P2&));
template<class P1, class P2, class P3>
StateCmd* handler (void (*fun)(EvCtx&, P1&, P2&, P3&));
template<class P1, class P2, class P3, class P4>
StateCmd* handler (void (*fun)(EvCtx&, P1&, P2&, P3&, P4&));
template<class P1, class P2, class P3, class P4, class P5>
StateCmd* handler (void (*fun)(EvCtx&, P1&, P2&, P3&, P4&, P5&));

template<class Heap>
StateCmd* handler (void (*fun)(TEvCtx<Heap>&));
template<class Heap, class P1>
StateCmd* handler (void (*fun)(TEvCtx<Heap>&, P1&));
template<class Heap, class P1, class P2>
StateCmd* handler (void (*fun)(TEvCtx<Heap>&, P1&, P2&));
template<class Heap, class P1, class P2, class P3>
StateCmd* handler (void (*fun)(TEvCtx<Heap>&, P1&, P2&, P3&));
template<class Heap, class P1, class P2, class P3, class P4>
StateCmd* handler (void (*fun)(TEvCtx<Heap>&, P1&, P2&, P3&, P4&));
template<class Heap, class P1, class P2, class P3, class P4, class P5>
StateCmd* handler (void (*fun)(TEvCtx<Heap>&, P1&, P2&, P3&, P4&, P5&));

// =======================================================================

#include "coe-kernel--imp.h"

#endif

