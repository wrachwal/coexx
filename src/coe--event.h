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

#ifndef __COE__EVENT_H
#define __COE__EVENT_H

#include "coe-kernel.h"         // IO_Mode
#include "coe--list.h"
#include <map>

namespace coe { /////

// -----------------------------------------------------------------------
// forward(s)

class EvCommon;
    class EvMsg;
    class EvAlarm;
    class EvIO;

// -----------------------------------------------------------------------
// SessionContext

struct r4Session;

struct SessionContext {
    SessionContext ()
        : parent(NULL), session(NULL) {}
    SessionContext (r4Session* session, const std::string& state)
        : parent(NULL), session(session), state(state) {}
    SessionContext* parent;
    r4Session*      session;
    std::string     state;
};

// -----------------------------------------------------------------------
// DueSidAid_Key

struct DueSidAid_Key {
    DueSidAid_Key () {}
    DueSidAid_Key (const TimeSpec& d, const SiD& s, AiD a)
        : due(d), sid(s), aid(a) {}
    bool operator< (const DueSidAid_Key& rhs) const
        {
            return due < rhs.due  || ((due == rhs.due
                && sid < rhs.sid) ||  (sid == rhs.sid
                && aid < rhs.aid));
        }
    // ------------
    TimeSpec    due;
    SiD         sid;
    AiD         aid;
};

// ------------------------------------

typedef std::map<DueSidAid_Key, EvAlarm*>   DueSidAid_Map;

// -----------------------------------------------------------------------
// FdModeSid_Key

struct FdModeSid_Key {
    FdModeSid_Key () : fd(-1), mode(IO_read) {}
    FdModeSid_Key (int f, IO_Mode m, const SiD& s)
        : fd(f), mode(m), sid(s) {}
    bool operator< (const FdModeSid_Key& rhs) const
        {
            return fd   < rhs.fd    || ((fd   == rhs.fd
                && mode < rhs.mode) ||  (mode == rhs.mode
                && sid  < rhs.sid));
        }
    // ------------
    int     fd;
    IO_Mode mode;
    SiD     sid;
};
// ------------------------------------

typedef std::map<FdModeSid_Key, EvIO*>  FdModeSid_Map;

// =======================================================================
// EvCommon

class ValParam;

class EvCommon {
public:
    virtual ~EvCommon ();

    virtual void dispatch () = 0;
    virtual bool is_event_of (KiD kernel) const = 0;

    bool enqueued () const { return NULL != _link_queue.next; }

    int  prio_order () const { return _prio_order; }
    void prio_order (int po);

protected:
    EvCommon () : _prio_order(-1) {}    // default is to favor local queue

    friend struct _EvCommon;
    dLink<EvCommon> _link_queue;

    int         _prio_order;    // [ -1, 0 .. PQLen )
};

// ------------------------------------

struct _EvCommon {
    typedef dList<EvCommon, offsetof(EvCommon, _link_queue)> Queue;
};

// -----------------------------------------------------------------------
// EvUser

class EvUser : public EvCommon {
public:
    ~EvUser ();

    /*final*/ bool is_event_of (KiD kernel) const;

    const std::string& name () const { return _name; }

    ValParam* arg () const { return _arg; }
    ValParam* arg (ValParam* new_arg);

    r4Session* target () const { return _target; }
    void       target (r4Session* session);

    const std::string& sender_state () const { return _sender_state; }

protected:
    EvUser (const std::string& name, ValParam* arg);

    r4Session*  _target;
    std::string _sender_state;
    std::string _name;
    ValParam*   _arg;
};

// =======================================================================
// EvMsg

class EvMsg : public EvUser {
public:
    EvMsg (const std::string& name, ValParam* arg, SessionContext& cc);  // post
    EvMsg (const std::string& name, ValParam* arg);                 // anon_post
    ~EvMsg ();

    /*final*/ void dispatch ();

    void       source (r4Session* r4s);
    r4Session* source () const { return _source; }

    SiD sender () const { return _sender; }

    ValParam* pfx () const { return _prefix; }
    ValParam* pfx (ValParam* new_pfx);

private:
    r4Session*  _source;
    SiD         _sender;
    ValParam*   _prefix;    // used by postback(s)
};

// -----------------------------------------------------------------------
// EvAlarm

class EvAlarm : public EvUser {
public:
    EvAlarm (const std::string& name, ValParam* arg, SessionContext& cc);
    ~EvAlarm ();

    /*final*/ void dispatch ();

    void aid (AiD a);
    AiD  aid () const { return _aid; }

    void            time_due (TimeSpec& abs_time);
    const TimeSpec& time_due () const { return _time_due; }

    void                    dsa_iter (DueSidAid_Map::iterator iter);
    DueSidAid_Map::iterator dsa_iter () const { return _dsa_iter; }

private:
    friend struct _EvAlarm;
    dLink<EvAlarm> _link_alarm;

    AiD                     _aid;
    TimeSpec                _time_due;
    DueSidAid_Map::iterator _dsa_iter;
};

// ------------------------------------

struct _EvAlarm {
    typedef dList<EvAlarm, offsetof(EvAlarm, _link_alarm)> List;
};

// -----------------------------------------------------------------------
// EvIO

class EvIO : public EvUser {
public:
    EvIO (int fd, IO_Mode mode, const std::string& name, ValParam* arg, SessionContext& cc);
    ~EvIO ();

    /*final*/ void dispatch ();

    int     fd   () const { return _fd; }
    IO_Mode mode () const { return _mode; }

    void active (bool a);
    bool active () const { return _active; }

private:
    friend struct _EvIO;
    dLink<EvIO> _link_evio;

    int         _fd;
    IO_Mode     _mode;
    bool        _active;
};

// ------------------------------------

struct _EvIO {
    typedef dList<EvIO, offsetof(EvIO, _link_evio)> List;
};

// =======================================================================
// EvSys_Export_Kernel
// EvSys_Import_Kernel

class EvSys_Export_Kernel : public EvCommon {
public:
    EvSys_Export_Kernel (r4Kernel *kernel) : _kernel(kernel) {}
    ~EvSys_Export_Kernel ();

    /*final*/ void dispatch ();
    /*final*/ bool is_event_of (KiD kernel) const;

private:
    r4Kernel*   _kernel;
};

// ------------------------------------

class EvSys_Import_Kernel : public EvCommon {
public:
    EvSys_Import_Kernel (r4Kernel *kernel) : _kernel(kernel) { _prio_order = 0; }
    ~EvSys_Import_Kernel ();

    /*final*/ void dispatch ();
    /*final*/ bool is_event_of (KiD kernel) const;

    //
    // public data members used by `d4Thread' APIs:
    //      _export_kernel_local_data
    //      _import_kernel_local_data
    //
    _EvCommon::Queue    lqueue;
    _EvCommon::Queue    pqueue;
    DueSidAid_Map       dsa_map;
    FdModeSid_Map       fms_map;

private:
    r4Kernel*   _kernel;
};

// =======================================================================

} ///// namespace coe

#endif

