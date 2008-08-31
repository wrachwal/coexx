// coe--ev.h

#ifndef __COE__EV_H
#define __COE__EV_H

#include "coe-kernel.h"     // IO_Mode
#include "coe--list.h"

class PostArg;
struct r4kernel;
struct r4Session;

// -----------------------------------------------------------------------
// SessionContext

struct SessionContext {
    SessionContext () : session(NULL) {}
    r4Session*  session;
    std::string state;
};

// =======================================================================
// EvCommon

class EvCommon {
    friend struct EvCommonList;
    dLink<EvCommon> _link_queue;
public:
    virtual ~EvCommon ();
    virtual void dispatch () = 0;
//protected:
    EvCommon (const std::string& name, PostArg* arg);
    r4Session*  _target;
    std::string _name;
    PostArg*    _arg;
};

struct EvCommonList {
    typedef dList<EvCommon, offsetof(EvCommon, _link_queue)> Queue;
};

// -----------------------------------------------------------------------
// EvMsg

class EvMsg : public EvCommon {
public:
    EvMsg (const std::string& name, PostArg* arg, SessionContext& cc);
    /*final*/ void dispatch ();
//private:
    SiD         _sender;
    std::string _sender_state;
};

// -----------------------------------------------------------------------
// EvAlarm

#if 0
class EvAlarm : public EvCommon {
public:
    /*final*/ void dispatch ();
//private:
    //AiD   _alarm_id;
    double  _time_due;
};

// -----------------------------------------------------------------------
// EvIO

class EvIO : public EvCommon {
public:
    /*final*/ void dispatch ();
//private:
    int     _fd;
    IO_Mode _mode;
};
#endif

// =======================================================================

#endif

