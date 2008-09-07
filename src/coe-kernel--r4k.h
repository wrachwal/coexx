// coe-kernel--r4k.h

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
    bool yield__arg (const std::string& ev, PostArg* arg);
    bool call__arg (SiD on, const std::string& ev, CallArg* arg);
    bool select__arg (int fd, IO_Mode mode, const std::string& ev, PostArg* arg);
    void state__cmd (const std::string& ev, StateCmd* cmd);

    void dispatch_evmsg (EvMsg* evmsg);

    void run_event_loop ();
};

// =======================================================================

#endif

