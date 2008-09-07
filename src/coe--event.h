// coe--event.h

#ifndef __COE__EVENT_H
#define __COE__EVENT_H

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
public:
    virtual ~EvCommon ();
    virtual void dispatch () = 0;
protected:
    EvCommon (const std::string& name, PostArg* arg);
    friend struct EvCommonList;
    dLink<EvCommon> _link_queue;
//private:
public:
    r4Session*  _target;
    std::string _name;
    PostArg*    _arg;
};

// ------------------------------------

struct EvCommonList {
    typedef dList<EvCommon, offsetof(EvCommon, _link_queue)> Queue;
};

// -----------------------------------------------------------------------
// EvMsg

class EvMsg : public EvCommon {
public:
    EvMsg (const std::string& name, PostArg* arg, SessionContext& cc);  // post
    EvMsg (const std::string& name, PostArg* arg);                 // anon_post
    /*final*/ void dispatch ();
//private:
    r4Session*  _source;
    SiD         _sender;
    std::string _sender_state;
};

// -----------------------------------------------------------------------
// EvAlarm

class EvAlarm : public EvCommon {
public:
    EvAlarm (const std::string& name, PostArg* arg, SessionContext& cc);
    /*final*/ void dispatch ();
//private:
    //AiD       _alarm_id;
    TimeSpec    _time_due;
};

// -----------------------------------------------------------------------
// EvIO

class EvIO : public EvCommon {
public:
    EvIO (const std::string& name, PostArg* arg, SessionContext& cc);
    /*final*/ void dispatch ();
//private:
    int     _fd;
    IO_Mode _mode;
};

// =======================================================================

#endif

