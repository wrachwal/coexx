// $Id$

/*************************************************************************
Copyright (c) 2008 Waldemar Rachwal

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

#include <map>

#include "coe--event.h"
#include "coe-thread--d4t.h"

class Kernel;
class Session;
class StateCmd;

struct s4Kernel;

// =======================================================================

struct r4Kernel {

    d4Thread*           _thread;    // controlling thread
    Kernel*             _handle;

    KiD                 _kid;
    IdentGenerator<SiD> _sid_generator;

    s4Kernel*           _s4kernel;

    SessionContext      _current_context;

    // _s1ev_cmd : sid/1 x ev-name --> StateCmd*
    typedef std::pair<SiD::IntType, std::string> S1Ev;
    typedef std::map<S1Ev, StateCmd*>            S1Ev_Cmd;
    S1Ev_Cmd            _s1ev_cmd;

    // -------------------------------------------------------------------

    r4Kernel ();

    SiD get_next_unique_sid ();
    SiD start_session (Session* s);
    void set_heap_ptr (EvCtx& ctx);

    StateCmd* find_state_handler (SiD::IntType sid1, const std::string& ev);

    bool post__arg (SiD to, const std::string& ev, PostArg* arg);
    bool call__arg (SiD on, const std::string& ev, CallArg* arg);
    bool select__arg (int fd, IO_Mode mode, const std::string& ev, PostArg* arg);
    void state__cmd (const std::string& ev, StateCmd* cmd);

    void dispatch_evmsg (EvMsg* evmsg);
};

// =======================================================================

#endif

