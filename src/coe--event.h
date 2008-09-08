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

    const std::string& name () const { return _name; }
    PostArg*            arg () const { return _arg; }

    r4Session* target () const { return _target; }
    void       target (r4Session* session);

    short prio_order () const { return _prio_order; }
    void  prio_order (int po);

protected:
    EvCommon (const std::string& name, PostArg* arg);

    friend struct EvCommonList;
    dLink<EvCommon> _link_queue;

    r4Session*  _target;
    short       _prio_order;    // [ -1, 0 .. PQLen )
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

    SiD                sender       () const { return _sender; }
    const std::string& sender_state () const { return _sender_state; }

    /*final*/ void dispatch ();

    void       source (r4Session* r4s);
    r4Session* source () const { return _source; }

private:
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

private:
    AiD         _aid;
    TimeSpec    _time_due;
};

// -----------------------------------------------------------------------
// EvIO

class EvIO : public EvCommon {
public:
    EvIO (const std::string& name, PostArg* arg, SessionContext& cc);

    /*final*/ void dispatch ();

private:
    int         _fd;
    IO_Mode     _mode;
};

// =======================================================================

#endif

