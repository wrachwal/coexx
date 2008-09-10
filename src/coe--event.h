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

#ifndef __COE__EVENT_H
#define __COE__EVENT_H

#include "coe-kernel.h"         // IO_Mode
#include "coe--list.h"
#include <map>

// forward(s)
class EvCommon;
    class EvMsg;
    class EvAlarm;
    class EvIO;

// -----------------------------------------------------------------------
// SessionContext

struct r4Session;

struct SessionContext {
    SessionContext () : session(NULL) {}
    r4Session*  session;
    std::string state;
};

// -----------------------------------------------------------------------
// DueSidAid_Key

struct DueSidAid_Key {
    DueSidAid_Key () {}
    DueSidAid_Key (const TimeSpec& d, const SiD& s, AiD a)
        : due(d), sid(s), aid(a) {}
    bool operator< (const DueSidAid_Key& rhs) const
        {
            return due < rhs.due || (due == rhs.due
                && sid < rhs.sid || (sid == rhs.sid
                && aid < rhs.aid));
        }
    // ------------
    TimeSpec    due;
    SiD         sid;
    AiD         aid;
};

// ------------------------------------

typedef std::map<DueSidAid_Key, EvAlarm*>   DueSidAid_Map;

// =======================================================================
// EvCommon

class PostArg;

class EvCommon {
public:
    virtual ~EvCommon ();

    virtual void dispatch () = 0;

    const std::string& name () const { return _name; }
    PostArg*            arg () const { return _arg; }

    r4Session* target () const { return _target; }
    void       target (r4Session* session);

    const std::string& sender_state () const { return _sender_state; }

    short prio_order () const { return _prio_order; }
    void  prio_order (int po);

protected:
    EvCommon (const std::string& name, PostArg* arg);

    friend struct EvCommonStore;
    dLink<EvCommon> _link_queue;

    r4Session*  _target;
    std::string _sender_state;
    short       _prio_order;    // [ -1, 0 .. PQLen )
    std::string _name;
    PostArg*    _arg;
};

// ------------------------------------

struct EvCommonStore {
    typedef dList<EvCommon, offsetof(EvCommon, _link_queue)> Queue;
};

// -----------------------------------------------------------------------
// EvMsg

class EvMsg : public EvCommon {
public:
    EvMsg (const std::string& name, PostArg* arg, SessionContext& cc);  // post
    EvMsg (const std::string& name, PostArg* arg);                 // anon_post
    ~EvMsg ();

    /*final*/ void dispatch ();

    void       source (r4Session* r4s);
    r4Session* source () const { return _source; }

    SiD sender () const { return _sender; }

private:
    r4Session*  _source;
    SiD         _sender;
};

// -----------------------------------------------------------------------
// EvAlarm

class EvAlarm : public EvCommon {
public:
    EvAlarm (const std::string& name, PostArg* arg, SessionContext& cc);
    ~EvAlarm ();

    /*final*/ void dispatch ();

    void aid (AiD a);
    AiD  aid () const { return _aid; }

    void            time_due (TimeSpec& abs_time);
    const TimeSpec& time_due () const { return _time_due; }

    void                    dsa_iter (DueSidAid_Map::iterator iter);
    DueSidAid_Map::iterator dsa_iter () const { return _dsa_iter; }

private:
    friend struct EvAlarmStore;
    dLink<EvAlarm> _link_alarm;

    AiD                     _aid;
    TimeSpec                _time_due;
    DueSidAid_Map::iterator _dsa_iter;
};

// ------------------------------------

struct EvAlarmStore {
    typedef dList<EvAlarm, offsetof(EvAlarm, _link_alarm)> List;
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

