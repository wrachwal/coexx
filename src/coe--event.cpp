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

#include "coe--event.h"
#include "coe-kernel--r4k.h"
#include "coe-session--r4s.h"

using namespace std;

// =======================================================================
// EvCommon

EvCommon::EvCommon (const string& name, PostArg* arg)
  : _target(NULL),
    _prio_order(-1),    // default is to favor local queue
    _name(name),
    _arg(arg)
{
}

EvCommon::~EvCommon ()
{
    assert(NULL == _link_queue.next);
    delete _arg;
    _arg = NULL;    // just in case ;)
}

PostArg* EvCommon::arg (PostArg* new_arg)
{
    PostArg* old_arg = _arg;
    _arg = new_arg;
    return old_arg;
}

void EvCommon::target (r4Session* session)
{
    assert(NULL == _link_queue.next);
    _target = session;
}

void EvCommon::prio_order (int po)
{
    assert(NULL == _link_queue.next);
    assert(po >= -1 && po < 1);     //TODO: modify to PQLen
    _prio_order = po;
}

// =======================================================================
// EvMsg

EvMsg::EvMsg (const string& name, PostArg* arg, SessionContext& cc)
  : EvCommon(name, arg)
{
    _source       = cc.session;
    _sender       = cc.session->_sid;
    _sender_state = cc.state;
}

EvMsg::EvMsg (const string& name, PostArg* arg)
  : EvCommon(name, arg),
    _source(NULL)
{
}

EvMsg::~EvMsg ()
{
}

void EvMsg::source (r4Session* session)
{
    assert(NULL == _link_queue.next);
    _source = session;
}

void EvMsg::dispatch ()
{
    _target->_kernel->dispatch_evmsg(this);
}

// =======================================================================
// EvAlarm

EvAlarm::EvAlarm (const string& name, PostArg* arg, SessionContext& cc)
  : EvCommon(name, arg)
{
    _target       = cc.session;
    _sender_state = cc.state;
}

EvAlarm::~EvAlarm ()
{
    assert(NULL == _link_alarm.next);
}

void EvAlarm::aid (AiD a)
{
    assert(NULL == _link_alarm.next);
    _aid = a;
}

void EvAlarm::time_due (TimeSpec& abs_time)
{
    assert(NULL == _link_queue.next);
    _time_due = abs_time;
}

void EvAlarm::dsa_iter (DueSidAid_Map::iterator iter)
{
    assert(NULL == _link_queue.next);
    _dsa_iter = iter;
}

void EvAlarm::dispatch ()
{
    _target->_kernel->dispatch_alarm(this);
}

// =======================================================================
// EvIO

EvIO::EvIO (const string& name, PostArg* arg, SessionContext& cc)
  : EvCommon(name, arg),
    _fd(-1),
    _mode(IO_read),
    _active(true)
{
    _target       = cc.session;
    _sender_state = cc.state;
}

EvIO::~EvIO ()
{
    assert(NULL == _link_evio.next);
}

void EvIO::fd (int f)
{
    assert(NULL == _link_evio.next);
    _fd = f;
}

void EvIO::mode (IO_Mode m)
{
    assert(NULL == _link_evio.next);
    _mode = m;
}

void EvIO::active (bool a)
{
    _active = a;
}

void EvIO::dispatch ()
{
    _target->_kernel->dispatch_evio(this);
}

