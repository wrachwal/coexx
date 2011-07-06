// coe-kernel.h

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

#ifndef __COE_KERNEL_H
#define __COE_KERNEL_H

#include "coe-ident.h"          // TiD, KiD, SiD, AiD
#include "coe-coestr.h"         // CoeStr
#include "coe-sys-time.h"       // TimeSpec
#include "coe-global.h"         // Factory<T>

#include <typeinfo>
#include "coe-kernel--dcl.h"
#include <memory>               // auto_ptr<>

namespace coe { /////

// ---------------------------------------------------------------------------

class Thread;
class Kernel;
class Session;          // coe-session.h
class Callback;
class EventContext;
struct Stats_Kernel;    // coe-stats.h

// handler context(s)
class IO_Ctx;

class EventArg;
    class RefParam;
    class ValParam;

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
    void           set_klabel (const std::string& label);
    const std::string& klabel () const;

    Thread& thread () const;
    bool move_to_thread (TiD tid);

    typedef Stats_Kernel Stats;
    void get_stats (Stats& stats);

    template<class T>
    T& kls ();
    static void* next_kls_info (void* iter, LocalStorageInfo& info);

           TimeSpec timestamp () const;
    static TimeSpec timeclock ();

    static Kernel* current_kernel ();
    Session&       session ();
    Session*  find_session (SiD sid);

    const EventContext& context ()         const;   // same as *context(0)
    const EventContext* context (int back) const;   // 0, 1, ... -1, -2, ...

    /*
     * Asynchronous Messages
     */
           bool      post          (SiD to, const CoeStr& ev, ValParam* vp=0);
           bool      post          (SiD to, const CoeStr& ev, std::auto_ptr<ValParam>& vp);
    static bool anon_post          (SiD to, const CoeStr& ev, ValParam* vp=0);
    static bool anon_post          (SiD to, const CoeStr& ev, std::auto_ptr<ValParam>& vp);
           bool     yield          (        const CoeStr& ev, ValParam* vp=0);

    /*
     * Synchronous Messages
     */
           bool      call          (SiD on, const CoeStr& ev, EventArg* arg=0);
           bool      call_keep_arg (SiD on, const CoeStr& ev, EventArg& arg);

    /*
     * Encapsulated `Callback'
     */
    Callback* callback (const CoeStr& ev, ValParam* pfx=0);

    /*
     * Timer Events (Delayed Messages)
     */
    // TODO: Name-Based Timers
    bool alarm     (const CoeStr& ev);          // reset
    bool alarm     (const CoeStr& ev, TimeSpec abs_time, ValParam* vp=0);
    bool alarm_add (const CoeStr& ev, TimeSpec abs_time, ValParam* vp=0);
    bool delay     (const CoeStr& ev, TimeSpec duration, ValParam* vp=0);
    bool delay_add (const CoeStr& ev, TimeSpec duration, ValParam* vp=0);
    //
    // Identifier-Based Timers
    AiD  alarm_remove (AiD aid);                // reset
    AiD  alarm_set    (const CoeStr& ev, TimeSpec abs_time, ValParam* vp=0);
    bool alarm_adjust (AiD aid, TimeSpec delta_secs);
    bool alarm_adjust (AiD aid, TimeSpec delta_secs, ValParam* vp);
    AiD  delay_set    (const CoeStr& ev, TimeSpec duration, ValParam* vp=0);
    bool delay_adjust (AiD aid, TimeSpec secs_from_now);
    bool delay_adjust (AiD aid, TimeSpec secs_from_now, ValParam* vp);
    //
    // TODO: Periodicity
    bool set_periodic       (AiD aid, TimeSpec interval);
    bool     periodic_clear (AiD aid);
    //
    void alarm_remove_all ();

    /*
     * I/O Watchers (Selects)
     */
    bool select (int fd, IO_Mode mode);         // reset
    bool select (int fd, IO_Mode mode, const CoeStr& ev, ValParam* vp=0);
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
    void state (const CoeStr& ev);              // reset
    void state (const CoeStr& ev, const HandlerX& handler);

    /*
     * A way to continue after a normal handler returns
     */
    void continuation (Handler0 cont);

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

    bool      call          (Kernel& kernel, EventArg* arg=0);
    bool      call_keep_arg (Kernel& kernel, EventArg& arg);

    bool      post          (Kernel& kernel, ValParam* vp=0);
    bool      post          (Kernel& kernel, std::auto_ptr<ValParam>& vp);
    bool anon_post          (                ValParam* vp=0);
    bool anon_post          (                std::auto_ptr<ValParam>& vp);

    static void* operator new               (std::size_t size);
    static void  operator delete (void* mem, std::size_t size);

private:
    friend class Kernel;
    Callback (SiD, const CoeStr&, ValParam*);

    SiD         _target;
    CoeStr      _evname;
    ValParam*   _prefix;
};

// ===========================================================================
// EventContext

class EventContext : private _Noncopyable {
public:
    enum Type { INIT, START, STOP, CALL, POST, ALARM, SELECT, SIGNAL };

    Type           type         () const;
    const Session& session      () const;   //XXX: or safer, but less flexible SiD
    CoeStr         state        () const;
    SiD            sender       () const;
    CoeStr         sender_state () const;

    AiD            alarm_id     () const;   // type() == ALARM

private:
    EventContext ();    // non-constructible
};

// ===========================================================================
// IO_Ctx

class IO_Ctx : private _Noncopyable {
public:
    int     fileno () const { return _fileno; }
    IO_Mode mode   () const { return _mode; }
private:
    friend struct r4Kernel;
    IO_Ctx (int f, IO_Mode m);
    int     _fileno;
    IO_Mode _mode;
};

// ===========================================================================
// vparam (p1[, ...p9])

#ifdef  COEXX_DOC_ONLY
struct ValParam {
    ValParam* clone () const;
    template<class _Arg> _Arg* argptr (size_t index);
};
#endif

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
template<class P1, class P2, class P3, class P4, class P5, class P6>
ValParam* vparam (const P1&, const P2&, const P3&, const P4&, const P5&, const P6&);
template<class P1, class P2, class P3, class P4, class P5, class P6, class P7>
ValParam* vparam (const P1&, const P2&, const P3&, const P4&, const P5&, const P6&, const P7&);
template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
ValParam* vparam (const P1&, const P2&, const P3&, const P4&, const P5&, const P6&, const P7&, const P8&);
template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
ValParam* vparam (const P1&, const P2&, const P3&, const P4&, const P5&, const P6&, const P7&, const P8&, const P9&);

// ---------------------------------------------------------------------------
// rparam (p1[, ...p9])

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
template<class P1, class P2, class P3, class P4, class P5, class P6>
RefParam* rparam (P1&, P2&, P3&, P4&, P5&, P6&);
template<class P1, class P2, class P3, class P4, class P5, class P6, class P7>
RefParam* rparam (P1&, P2&, P3&, P4&, P5&, P6&, P7&);
template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
RefParam* rparam (P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&);
template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
RefParam* rparam (P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&);

// ---------------------------------------------------------------------------
// handler (obj, memfun)
// handler (fun)

template<class Obj>
Handler0 handler (Obj& obj, void (Obj::*fun)(Kernel&));
template<class Obj, class P1>
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&));
template<class Obj, class P1, class P2>
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&));
template<class Obj, class P1, class P2, class P3>
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&));
template<class Obj, class P1, class P2, class P3, class P4>
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&));
template<class Obj, class P1, class P2, class P3, class P4, class P5>
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&));
template<class Obj, class P1, class P2, class P3, class P4, class P5, class P6>
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&));
template<class Obj, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&, P7&));
template<class Obj, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&));
template<class Obj, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
HandlerX handler (Obj& obj, void (Obj::*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&));

// ------------------------------------

Handler0 handler (void (*fun)(Kernel&));
template<class P1>
HandlerX handler (void (*fun)(Kernel&, P1&));
template<class P1, class P2>
HandlerX handler (void (*fun)(Kernel&, P1&, P2&));
template<class P1, class P2, class P3>
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&));
template<class P1, class P2, class P3, class P4>
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&));
template<class P1, class P2, class P3, class P4, class P5>
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&));
template<class P1, class P2, class P3, class P4, class P5, class P6>
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&));
template<class P1, class P2, class P3, class P4, class P5, class P6, class P7>
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&, P7&));
template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&));
template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
HandlerX handler (void (*fun)(Kernel&, P1&, P2&, P3&, P4&, P5&, P6&, P7&, P8&, P9&));

// ===========================================================================

} ///// namespace coe

#include "coe-kernel--imp.h"

#endif

