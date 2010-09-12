// coe-kernel--r4k.h

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

#ifndef __COE_KERNEL__R4K_H
#define __COE_KERNEL__R4K_H

#include "coe--context.h"
#include "coe--util.h"          // IdentGenerator<>
#include "coe-sys-sync.h"       // RWLock, ...

#include <vector>               // _user_kls

namespace coe { /////

// ---------------------------------------------------------------------------

class Kernel;
class Session;

struct d4Thread;
struct s4Kernel;

// ---------------------------------------------------------------------------

typedef std::map<SiD, r4Session*> Sid_Map;

// ===========================================================================

struct r4Kernel {

    // --------------------------------
    //         | rwlock |
    // thread  |  R | W |
    // ------------------
    //  local  |  0 | W |
    // foreign |  R | ! |
    //
    struct Local {

        RWLock          rwlock;

        Sid_Map         sid_map;    //TODO: hash_map

        // --------

        r4Session* find_session (SiD sid) const;

    } local;

    // --------------------------------

    d4Thread*           _thread;            // driving thread
    TiD                 _target_thread;
    Kernel*             _handle;

    KiD                 _kid;
    std::string         _klabel;

    std::vector<void*>  _user_kls;

    IdentGenerator<SiD> _sid_generator;

    s4Kernel*           _s4kernel;

    dLink<r4Kernel>     _link_kernel;

    ExecuteContext*     _current_context;
    ExecuteContext      _kernel_session_context;

    /*
     * alarms
     */
    Sid1Aid_Map         _s1a_map;   //TODO: hash_map

    /*
     * handlers ("commands")
     */
    // _s1ev_cmd : sid/1 x ev-name --> HandlerX
    typedef std::pair<SiD::IntType, CoeStr> S1Ev;
    typedef std::map<S1Ev, HandlerX>        S1Ev_Cmd;
    S1Ev_Cmd            _s1ev_cmd;

    // -----------------------------------------------------------------------

    r4Kernel ();
    ~r4Kernel ();

    void* get_user_kls (const _KlsD* info);

    void _allocate_sid (r4Session* r4s);

    SiD start_session (Session* s, r4Session* parent, EventArg* arg);
    void call_stop  (r4Session& root, r4Session& node);

    bool delete_alarm (AiD aid);
    bool adjust_alarm (AiD aid, const TimeSpec& abs_time, bool update, ValParam* new_arg);

    HandlerX find_state_handler (SiD::IntType sid1, const CoeStr& ev);
    void state__cmd (const CoeStr& ev, const HandlerX& cmd);

    bool call__arg (SiD on, const CoeStr& ev, ValParam* pfx, EventArg* arg);
    bool call_event_handler (r4Session* session, const CoeStr& ev, EventArg* arg);

    void dispatch_evmsg (EvMsg* evmsg);
    void dispatch_alarm (EvAlarm* alarm);
    void dispatch_evio  (EvIO* evio);
};

// ---------------------------------------------------------------------------

struct _r4Kernel {
    typedef dList<r4Kernel, offsetof(r4Kernel, _link_kernel)> List;
};

// ===========================================================================

} ///// namespace coe

#endif

