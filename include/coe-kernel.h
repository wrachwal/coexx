// coe-kernel.h

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

#ifndef __COE_KERNEL_H
#define __COE_KERNEL_H

#include "coe-ident.h"      // TiD, KiD, SiD, AiD
#include "coe-sys-time.h"   // TimeSpec
#include "coe-global.h"     // Factory<T>
#include "coe--local.h"
#include <typeinfo>

namespace coe { /////

// ---------------------------------------------------------------------------

class Thread;
class Kernel;
class Session;      // coe-session.h
class Callback;

// handler context(s)
class EvCtx;
class DatIO;

class RefParam;
class ValParam;

class StateCmd;
    class MFunCmd0;

// private data
struct r4Kernel;
struct _KlsD;

// ---------------------------------------------------------------------------

enum IO_Mode {
    IO_read  = 0,
    IO_write = 1,
    IO_error = 2
};

// ===========================================================================
// Kernel -- current session's interface object

class Kernel : private _Noncopyable {
public:
    static Kernel& create_new ();

    KiD ID () const;

    Thread& thread () const;
    bool move_to_thread (TiD tid);

    template<class T>
    T& kls ();
    static void* next_kls_info (void* iter, LocalStorageInfo& info);

    TimeSpec timestamp () const;

    Session&           session ();
    static SiD current_session ();

    /*
     * Asynchronous Messages
     */
           bool      post (SiD to, const std::string& ev, ValParam* vp=0);
    static bool anon_post (SiD to, const std::string& ev, ValParam* vp=0);
           bool     yield (        const std::string& ev, ValParam* vp=0);

    /*
     * Synchronous Messages
     */
           bool      call (SiD on, const std::string& ev);
           bool      call (SiD on, const std::string& ev, RefParam* rp);
           bool      call (SiD on, const std::string& ev, ValParam* vp);

    /*
     * Encapsulated `Callback'
     */
    Callback* callback (const std::string& ev, ValParam* pfx=0);

    /*
     * Timer Events (Delayed Messages)
     */
    // Name-Based Timers
    bool alarm     (const std::string ev);      // reset
    bool alarm     (const std::string ev, TimeSpec abs_time, ValParam* vp=0);
    bool alarm_add (const std::string ev, TimeSpec abs_time, ValParam* vp=0);
    bool delay     (const std::string ev, TimeSpec duration, ValParam* vp=0);
    bool delay_add (const std::string ev, TimeSpec duration, ValParam* vp=0);
    //
    // Identifier-Based Timers
    AiD  alarm_remove (AiD aid);                // reset
    AiD  alarm_set    (const std::string ev, TimeSpec abs_time, ValParam* vp=0);
    bool alarm_adjust (AiD aid, TimeSpec delta_secs);
    bool alarm_adjust (AiD aid, TimeSpec delta_secs, ValParam* vp);
    AiD  delay_set    (const std::string ev, TimeSpec duration, ValParam* vp=0);
    bool delay_adjust (AiD aid, TimeSpec secs_from_now);
    bool delay_adjust (AiD aid, TimeSpec secs_from_now, ValParam* vp);
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
    bool select (int fd, IO_Mode mode, const std::string& ev, ValParam* vp=0);
    bool select_pause  (int fd, IO_Mode mode);
    bool select_resume (int fd, IO_Mode mode);

    /*
     * TODO: `signal' watchers
     */

    /*
     * TODO: `idle' watchers
     */

    /*
     * Event Handler Management
     */
    void state (const std::string& ev);         // reset
    void state (const std::string& ev, StateCmd* handler);

private:
    friend struct r4Kernel;
    Kernel ();
    void* _get_user_kls (const _KlsD* data);

    friend class Session;
    friend class Callback;
    r4Kernel*   _r4kernel;
};

// ===========================================================================
// Callback

class Callback : private _Noncopyable {
public:
    ~Callback ();

    SiD session () const { return _target; }

    bool      call (Kernel& kernel);
    bool      call (Kernel& kernel, RefParam* arg);
    bool      call (Kernel& kernel, ValParam* arg);

    bool      post (Kernel& kernel, ValParam* arg=0);
    bool anon_post (                ValParam* arg=0);

private:
    friend class Kernel;
    Callback (SiD, const std::string&, ValParam*);

    SiD         _target;
    std::string _evname;
    ValParam*   _prefix;
};

// ===========================================================================
// EvCtx

class EvCtx : private _Noncopyable {
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
};

// ------------------------------------
// DatIO

class DatIO : private _Noncopyable {
public:
    const int       filedes;
    const IO_Mode   mode;
private:
    friend struct r4Kernel;
    DatIO (int f, IO_Mode m);
};

// ===========================================================================
// vparam (p1[, ...])

template<class P1>
ValParam* vparam (const P1&);
template<class P1, class P2>
ValParam* vparam (const P1&, const P2&);
template<class P1, class P2, class P3>
ValParam* vparam (const P1&, const P2&, const P3&);
template<class P1, class P2, class P3, class P4>
ValParam* vparam (const P1&, const P2&, const P3&, const P4&);
template<class P1, class P2, class P3, class P4, class P5>
ValParam* vparam (const P1&, const P2&, const P3&, const P4&, const P5&);

// ---------------------------------------------------------------------------
// rparam (p1[, ...])

template<class P1>
RefParam* rparam (P1&);
template<class P1, class P2>
RefParam* rparam (P1&, P2&);
template<class P1, class P2, class P3>
RefParam* rparam (P1&, P2&, P3&);
template<class P1, class P2, class P3, class P4>
RefParam* rparam (P1&, P2&, P3&, P4&);
template<class P1, class P2, class P3, class P4, class P5>
RefParam* rparam (P1&, P2&, P3&, P4&, P5&);

// ---------------------------------------------------------------------------
// handler (obj, memfun)
// handler (fun)

template<class Obj>
MFunCmd0* handler (Obj& obj, void (Obj::*memfun)(EvCtx&));
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

// ===========================================================================

} ///// namespace coe

#include "coe-kernel--imp.h"

#endif

