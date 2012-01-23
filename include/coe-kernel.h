// coe-kernel.h

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

#ifndef __COE_KERNEL_H
#define __COE_KERNEL_H

#include "coe-ident.h"          // TiD, KiD, SiD, AiD
#include "coe-coestr.h"         // CoeStr
#include "coe-sys-time.h"       // TimeSpec
#include "coe-global.h"         // Factory<T>

#include "coe-kernel--dcl.h"
#include <memory>               // unique_ptr<>

namespace coe { /////

// ---------------------------------------------------------------------------

class Thread;
class Kernel;
class Session;          // coe-session.h
class Callback;
class Postback;
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
           bool      post          (SiD to, const CoeStr& ev, std::unique_ptr<ValParam>& vp);
    static bool anon_post          (SiD to, const CoeStr& ev, ValParam* vp=0);
    static bool anon_post          (SiD to, const CoeStr& ev, std::unique_ptr<ValParam>& vp);
           bool     yield          (        const CoeStr& ev, ValParam* vp=0);

    template<class Ev>
    bool post (SiD to)
        {
            return this->_post_oev_0(to, &typeid(Ev), typename Ev::args_type());
        }
    template<class Ev>
    bool post (SiD to, ValParam_<typename Ev::args_type> vp)
        {
            //TODO compile-time type assertion put here
            return this->_post_oev_x(to, &typeid(Ev), vp);
        }

    /*
     * Synchronous Messages
     */
           bool      call          (SiD on, const CoeStr& ev, EventArg* arg=0);
           bool      call_keep_arg (SiD on, const CoeStr& ev, EventArg& arg);

    /*
     * Encapsulated `Callback' and `Postback'
     */
    Callback callback (const CoeStr& ev, ValParam* pfx=0);
    Postback postback (const CoeStr& ev, ValParam* pfx=0);

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

    template<class Ev>
    void state ()                               // reset
        {
            this->_state_oev(&typeid(Ev));
        }
    template<class Ev>
    void state (const Handler_<typename Ev::args_type>& handler)
        {
            this->_state_oev(&typeid(Ev), handler);
        }

    /*
     * A way to continue after a normal handler returns
     */
    void continuation (Handler0 cont);

private:
    friend struct r4Kernel;
    Kernel ();
    void* _get_user_kls (const _KlsD* data);
    bool _post_oev_0 (SiD to, const std::type_info* ev, Nil);
    bool _post_oev_x (SiD to, const std::type_info* ev, ValParam* vp);
    void _state_oev (const std::type_info* ev);
    void _state_oev (const std::type_info* ev, HandlerX handler);

    friend class Session;
    friend class Callback;
    friend class Postback;
    r4Kernel*   _r4kernel;
};

// ===========================================================================
// Callback

struct _Sev_Callback;

class Callback {
public:
    Callback () : _impl(0) {}
    Callback (const Callback& rhs);
    Callback& operator= (const Callback& rhs);
    ~Callback () { reset(); }

    void reset ();

    operator _SafeBool<Callback, _Sev_Callback*>::Type () const
        { return _impl ? &Callback::_impl : 0; }
    bool operator== (const Callback& rhs) const { return _impl == rhs._impl; }
    bool operator!= (const Callback& rhs) const { return _impl != rhs._impl; }

    SiD session () const;

    bool call          (Kernel& kernel, EventArg* arg=0);
    bool call_keep_arg (Kernel& kernel, EventArg& arg);

private:
    friend class Kernel;
    Callback (SiD, const CoeStr&, ValParam*);
    _Sev_Callback*  _impl;
};

// ===========================================================================
// Postback

struct _Sev_Postback;

class Postback {
public:
    Postback () : _impl(0) {}
    Postback (const Postback& rhs);
    Postback& operator= (const Postback& rhs);
    ~Postback () { reset(); }

    void reset ();

    operator _SafeBool<Postback, _Sev_Postback*>::Type () const
        { return _impl ? &Postback::_impl : 0; }
    bool operator== (const Postback& rhs) const { return _impl == rhs._impl; }
    bool operator!= (const Postback& rhs) const { return _impl != rhs._impl; }

    SiD session () const;

    bool      post (Kernel& kernel, ValParam* vp=0);
    bool      post (Kernel& kernel, std::unique_ptr<ValParam>& vp);
    bool anon_post (                ValParam* vp=0);
    bool anon_post (                std::unique_ptr<ValParam>& vp);

private:
    friend class Kernel;
    Postback (SiD, const CoeStr&, ValParam*);
    _Sev_Postback*  _impl;
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

template<COE_T(1, class P)> ValParam_<typename List1<COE_T(1, P)>::type>
    vparam (COE_TA(1, const P, &p));
template<COE_T(2, class P)> ValParam_<typename List2<COE_T(2, P)>::type>
    vparam (COE_TA(2, const P, &p));
template<COE_T(3, class P)> ValParam_<typename List3<COE_T(3, P)>::type>
    vparam (COE_TA(3, const P, &p));
template<COE_T(4, class P)> ValParam_<typename List4<COE_T(4, P)>::type>
    vparam (COE_TA(4, const P, &p));
template<COE_T(5, class P)> ValParam_<typename List5<COE_T(5, P)>::type>
    vparam (COE_TA(5, const P, &p));
template<COE_T(6, class P)> ValParam_<typename List6<COE_T(6, P)>::type>
    vparam (COE_TA(6, const P, &p));
template<COE_T(7, class P)> ValParam_<typename List7<COE_T(7, P)>::type>
    vparam (COE_TA(7, const P, &p));
template<COE_T(8, class P)> ValParam_<typename List8<COE_T(8, P)>::type>
    vparam (COE_TA(8, const P, &p));
template<COE_T(9, class P)> ValParam_<typename List9<COE_T(9, P)>::type>
    vparam (COE_TA(9, const P, &p));

// ---------------------------------------------------------------------------
// rparam (p1[, ...p9])

template<COE_T(1, class P)> RefParam_<typename List1<COE_T(1, P)>::type>
    rparam (COE_TA(1, P, &p));
template<COE_T(2, class P)> RefParam_<typename List2<COE_T(2, P)>::type>
    rparam (COE_TA(2, P, &p));
template<COE_T(3, class P)> RefParam_<typename List3<COE_T(3, P)>::type>
    rparam (COE_TA(3, P, &p));
template<COE_T(4, class P)> RefParam_<typename List4<COE_T(4, P)>::type>
    rparam (COE_TA(4, P, &p));
template<COE_T(5, class P)> RefParam_<typename List5<COE_T(5, P)>::type>
    rparam (COE_TA(5, P, &p));
template<COE_T(6, class P)> RefParam_<typename List6<COE_T(6, P)>::type>
    rparam (COE_TA(6, P, &p));
template<COE_T(7, class P)> RefParam_<typename List7<COE_T(7, P)>::type>
    rparam (COE_TA(7, P, &p));
template<COE_T(8, class P)> RefParam_<typename List8<COE_T(8, P)>::type>
    rparam (COE_TA(8, P, &p));
template<COE_T(9, class P)> RefParam_<typename List9<COE_T(9, P)>::type>
    rparam (COE_TA(9, P, &p));

// ---------------------------------------------------------------------------
// handler (obj, memfun)
// handler (fun)

template<class Obj> Handler0
    handler (Obj& obj, void (Obj::*fun)(Kernel&));
template<class Obj, COE_T(1, class P)> Handler_<typename List1<COE_T(1, P)>::type>
    handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(1, P, &p)));
template<class Obj, COE_T(2, class P)> Handler_<typename List2<COE_T(2, P)>::type>
    handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(2, P, &p)));
template<class Obj, COE_T(3, class P)> Handler_<typename List3<COE_T(3, P)>::type>
    handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(3, P, &p)));
template<class Obj, COE_T(4, class P)> Handler_<typename List4<COE_T(4, P)>::type>
    handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(4, P, &p)));
template<class Obj, COE_T(5, class P)> Handler_<typename List5<COE_T(5, P)>::type>
    handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(5, P, &p)));
template<class Obj, COE_T(6, class P)> Handler_<typename List6<COE_T(6, P)>::type>
    handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(6, P, &p)));
template<class Obj, COE_T(7, class P)> Handler_<typename List7<COE_T(7, P)>::type>
    handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(7, P, &p)));
template<class Obj, COE_T(8, class P)> Handler_<typename List8<COE_T(8, P)>::type>
    handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(8, P, &p)));
template<class Obj, COE_T(9, class P)> Handler_<typename List9<COE_T(9, P)>::type>
    handler (Obj& obj, void (Obj::*fun)(Kernel&, COE_TA(9, P, &p)));

// ------------------------------------

Handler0
    handler (void (*fun)(Kernel&));
template<COE_T(1, class P)> Handler_<typename List1<COE_T(1, P)>::type>
    handler (void (*fun)(Kernel&, COE_TA(1, P, &p)));
template<COE_T(2, class P)> Handler_<typename List2<COE_T(2, P)>::type>
    handler (void (*fun)(Kernel&, COE_TA(2, P, &p)));
template<COE_T(3, class P)> Handler_<typename List3<COE_T(3, P)>::type>
    handler (void (*fun)(Kernel&, COE_TA(3, P, &p)));
template<COE_T(4, class P)> Handler_<typename List4<COE_T(4, P)>::type>
    handler (void (*fun)(Kernel&, COE_TA(4, P, &p)));
template<COE_T(5, class P)> Handler_<typename List5<COE_T(5, P)>::type>
    handler (void (*fun)(Kernel&, COE_TA(5, P, &p)));
template<COE_T(6, class P)> Handler_<typename List6<COE_T(6, P)>::type>
    handler (void (*fun)(Kernel&, COE_TA(6, P, &p)));
template<COE_T(7, class P)> Handler_<typename List7<COE_T(7, P)>::type>
    handler (void (*fun)(Kernel&, COE_TA(7, P, &p)));
template<COE_T(8, class P)> Handler_<typename List8<COE_T(8, P)>::type>
    handler (void (*fun)(Kernel&, COE_TA(8, P, &p)));
template<COE_T(9, class P)> Handler_<typename List9<COE_T(9, P)>::type>
    handler (void (*fun)(Kernel&, COE_TA(9, P, &p)));

// ===========================================================================

} ///// namespace coe

#include "coe-kernel--imp.h"

#endif

