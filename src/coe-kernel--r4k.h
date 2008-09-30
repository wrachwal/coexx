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

#ifndef __COE_KERNEL__R4K_H
#define __COE_KERNEL__R4K_H

#include "coe--event.h"
#include "coe--util.h"          // IdentGenerator<>
#include "coe-sys.h"            // RWLock, ...

class Kernel;
class Session;
class StateCmd;

struct d4Thread;
struct s4Kernel;

// -----------------------------------------------------------------------

typedef std::map<SiD, r4Session*> Sid_Map;

// =======================================================================

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
    IdentGenerator<SiD> _sid_generator;

    s4Kernel*           _s4kernel;

    dLink<r4Kernel>     _link_kernel;

    SessionContext*     _current_context;
    SessionContext      _kernel_session_context;

    // _s1ev_cmd : sid/1 x ev-name --> StateCmd*
    typedef std::pair<SiD::IntType, std::string> S1Ev;
    typedef std::map<S1Ev, StateCmd*>            S1Ev_Cmd;
    S1Ev_Cmd            _s1ev_cmd;

    // -------------------------------------------------------------------

    r4Kernel ();

    SiD start_session (Session* s);
    void _allocate_sid (r4Session* r4s);

    StateCmd* find_state_handler (SiD::IntType sid1, const std::string& ev);
    void state__cmd (const std::string& ev, StateCmd* cmd);

    bool call__arg (SiD on, const std::string& ev, PostArg* pfx, EventArg* arg);

    void dispatch_evmsg (EvMsg* evmsg);
    void dispatch_alarm (EvAlarm* alarm);
    void dispatch_evio  (EvIO* evio);
};

// -----------------------------------------------------------------------

struct r4KernelStore {
    typedef dList<r4Kernel, offsetof(r4Kernel, _link_kernel)> List;
};

// =======================================================================

#endif

