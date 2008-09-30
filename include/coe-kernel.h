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

    KiD ID () const;

    TimeSpec timestamp () const;

    void run_event_loop ();
    bool run_event_loop (TiD tid);  // transfer to `tid' thread

    SiD start_session (Session*);

    /*
     * Asynchronous Messages
     */
           bool      post (SiD to, const std::string& ev, PostArg* pp=0);
    static bool anon_post (SiD to, const std::string& ev, PostArg* pp=0);
           bool     yield (        const std::string& ev, PostArg* pp=0);

    /*
     * Synchronous Messages
     */
           bool      call (SiD on, const std::string& ev);
           bool      call (SiD on, const std::string& ev, CallArg* cp=0);
           bool      call (SiD on, const std::string& ev, PostArg* pp=0);

    /*
     * Timer Events (Delayed Messages)
     */
    // Name-Based Timers
    bool alarm     (const std::string ev);          // reset
    bool alarm     (const std::string ev, TimeSpec abs_time, PostArg* pp=0);
    bool alarm_add (const std::string ev, TimeSpec abs_time, PostArg* pp=0);
    bool delay     (const std::string ev, TimeSpec duration, PostArg* pp=0);
    bool delay_add (const std::string ev, TimeSpec duration, PostArg* pp=0);
    //
    // Identifier-Based Timers
    AiD  alarm_remove (AiD aid);                    // reset
    AiD  alarm_set    (const std::string ev, TimeSpec abs_time, PostArg* pp=0);
    bool alarm_adjust (AiD aid, TimeSpec delta_secs);
    AiD  delay_set    (const std::string ev, TimeSpec duration, PostArg* pp=0);
    bool delay_adjust (AiD aid, TimeSpec secs_from_now);
    //
    // Periodicity
    bool set_periodic       (AiD aid, TimeSpec interval);
    bool     periodic_clear (AiD aid);
    //
    void alarm_remove_all ();

    /*
     * I/O Watchers (Selects)
     */
    bool select (int fd, IO_Mode mode);         // reset
    bool select (int fd, IO_Mode mode, const std::string& ev, PostArg* pp=0);
    bool select_pause  (int fd, IO_Mode mode);
    bool select_resume (int fd, IO_Mode mode);

    /*
     * Event Handler Management
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
    AiD         alarm_id;   // set if timer event
private:
    friend struct   r4Kernel;
    explicit EvCtx (r4Kernel* k);
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
             TimeSpec ()                      { tv_sec = tv_nsec = 0; }
    explicit TimeSpec (const timespec& ts);
    explicit TimeSpec (double sec);
    explicit TimeSpec (time_t sec)            { tv_sec = sec; tv_nsec = 0; }
             TimeSpec (time_t sec, long nsec) { tv_sec = sec; tv_nsec = nsec; }

    static TimeSpec ZERO () { return TimeSpec(); }

    int compare (const TimeSpec& rhs) const;    // [ -1, 0, 1 ]

    TimeSpec& operator+= (const TimeSpec& add);
    TimeSpec& operator-= (const TimeSpec& sub);
};

// ------------------------------------

bool operator== (const TimeSpec& lhs, const TimeSpec& rhs);
bool operator!= (const TimeSpec& lhs, const TimeSpec& rhs);

bool operator<  (const TimeSpec& lhs, const TimeSpec& rhs);
bool operator<= (const TimeSpec& lhs, const TimeSpec& rhs);
bool operator>  (const TimeSpec& lhs, const TimeSpec& rhs);
bool operator>= (const TimeSpec& lhs, const TimeSpec& rhs);

TimeSpec operator+ (const TimeSpec& lhs, const TimeSpec& rhs);
TimeSpec operator- (const TimeSpec& lhs, const TimeSpec& rhs);

// =======================================================================
// pparam (p1[, ...])

template<class P1>
PostArg* pparam (const P1&);
template<class P1, class P2>
PostArg* pparam (const P1&, const P2&);
template<class P1, class P2, class P3>
PostArg* pparam (const P1&, const P2&, const P3&);
template<class P1, class P2, class P3, class P4>
PostArg* pparam (const P1&, const P2&, const P3&, const P4&);
template<class P1, class P2, class P3, class P4, class P5>
PostArg* pparam (const P1&, const P2&, const P3&, const P4&, const P5&);

// -----------------------------------------------------------------------
// cparam (p1[, ...])

template<class P1>
CallArg* cparam (P1&);
template<class P1, class P2>
CallArg* cparam (P1&, P2&);
template<class P1, class P2, class P3>
CallArg* cparam (P1&, P2&, P3&);
template<class P1, class P2, class P3, class P4>
CallArg* cparam (P1&, P2&, P3&, P4&);
template<class P1, class P2, class P3, class P4, class P5>
CallArg* cparam (P1&, P2&, P3&, P4&, P5&);

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

