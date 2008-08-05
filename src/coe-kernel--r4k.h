// coe-kernel--r4k.h

#ifndef __COE_KERNEL__R4K_H
#define __COE_KERNEL__R4K_H

#include <map>

class Kernel;
class Session;
class StateCmd;

struct s4Kernel;

// =======================================================================

struct r4Kernel {

    Kernel*     _handle;
    r4Kernel*   _parent;
    s4Kernel*   _s4kernel;

    r4Session*  _current_session;

    NiD         _last_sid;
    bool        _sid_wrap;

    typedef std::map<std::string, StateCmd*> State2Cmd;
    State2Cmd   _state_cmd;

    // -------------------------------------------------------------------

    r4Kernel ();

    SiD get_next_unique_sid ();
    bool start_session (Session* s);
    void set_heap_ptr (EvCtx& ctx);

    StateCmd* find_state_handler (const std::string& ev);

    bool post__arg (SiD to, const std::string& ev, PostArg* arg);
    bool yield__arg (const std::string& ev, PostArg* arg);
    bool call__arg (SiD on, const std::string& ev, CallArg* arg);
    bool select__arg (int fd, Kernel::IO_Mode mode, const std::string& ev, PostArg* arg);
    void state__cmd (const std::string& ev, StateCmd* cmd);

    void run_event_loop ();
};

// =======================================================================

#endif

