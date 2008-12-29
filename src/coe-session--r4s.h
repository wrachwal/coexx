// $Id$

/*****************************************************************************
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
*****************************************************************************/

#ifndef __COE_SESSION__R4S_H
#define __COE_SESSION__R4S_H

#include "coe--event.h"
#include "coe--util.h"
#include <functional>
#include <vector>

namespace coe { /////

typedef void (*Unregistrar)(SiD);

// ===========================================================================
// r4Session

struct r4Session {

    Session*            _handle;
    SiD                 _sid;

    std::vector<Unregistrar>    _unregistrar;

    void*               _heap;

    r4Kernel*           _kernel;

    // related sessions
    r4Session*          _parent;
    dList<void, 0>      _list_children;

    //
    // `local' is to be protected the same way as r4Kernel::local.
    //
    struct Local {
        SiD             stopper;
    } local;

    // alarms
    IdentGenerator<AiD> _aid_generator;
    _EvAlarm::List      _list_alarm;

    // i/o
    _EvIO::List         _list_evio;

    // --------------------------------

    r4Session ();
    ~r4Session ();
    void destroy ();

    void stop_session_tree ();
    static void _call_stop (r4Session& root, r4Session& node);

    EvIO* find_io_watcher (int fd, IO_Mode mode);

private:
    friend struct _r4Session;
    dLink<r4Session>    _link_children;
};

// ------------------------------------

struct _r4Session {
    typedef dList<r4Session, offsetof(r4Session, _link_children)> ChildrenList;

    // -Wno-strict-aliasing
    static ChildrenList& list_children (r4Session& session)
        { return reinterpret_cast<ChildrenList&>(session._list_children); }
};

// ---------------------------------------------------------------------------
// AiDExistsPred

class AiDExistsPred : public std::unary_function<AiD, bool> {
public:
    AiDExistsPred (_EvAlarm::List& list) : _list(list) {}
    bool operator() (AiD aid) const
        {
            _EvAlarm::List::iterator i = _list.begin();
            while (i != _list.end()) {
                if ((*i)->aid() == aid)
                    return true;
                ++ i;
            }
            return false;
        }
private:
    _EvAlarm::List& _list;
};

// ===========================================================================

} ///// namespace coe

#endif

