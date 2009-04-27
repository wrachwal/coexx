// coe--event.cpp

/*****************************************************************************
Copyright (c) 2008, 2009 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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

#include "coe--event.h"
#include "coe--context.h"
#include "coe-thread--d4t.h"
#include "coe-session--r4s.h"

using namespace std;
using namespace coe;

// ===========================================================================
// EvCommon

EvCommon::~EvCommon ()
{
    assert(NULL == _link_queue.next);
}

void EvCommon::prio_order (int po)
{
    assert(NULL == _link_queue.next);
    assert(po >= -1 && po < 1);     //TODO: modify to PQLen
    _prio_order = po;
}

// ===========================================================================
// EvUser

EvUser::EvUser (const string& name, ValParam* arg)
:   _target(NULL),
    _name(name),
    _arg(arg)
{
}

EvUser::~EvUser ()
{
    delete _arg;
    _arg = NULL;    // just in case
}

ValParam* EvUser::arg (ValParam* new_arg)
{
    ValParam* old_arg = _arg;
    _arg = new_arg;
    return old_arg;
}

void EvUser::target (r4Session* session)
{
    assert(NULL == _link_queue.next);
    _target = session;
}

bool EvUser::is_event_of (KiD kernel) const
{
    assert(NULL != _target);
    assert(NULL != _target->_kernel);
    return _target->_kernel->_kid == kernel;
}

// ===========================================================================
// EvMsg

EvMsg::EvMsg (const string& name, ValParam* arg, ExecuteContext& cc)
:   EvUser(name, arg),
    _prefix(NULL)
{
    _source       = cc.session;
    _sender       = cc.session->_sid;
    _sender_state = cc.state;
}

EvMsg::EvMsg (const string& name, ValParam* arg)
:   EvUser(name, arg),
    _source(NULL),
    _prefix(NULL)
{
}

EvMsg::~EvMsg ()
{
    delete _prefix;
    _prefix = NULL;     // just in case
}

void EvMsg::source (r4Session* session)
{
    assert(NULL == _link_queue.next);
    _source = session;
}

ValParam* EvMsg::pfx (ValParam* new_pfx)
{
    ValParam* old_prefix = _prefix;
    _prefix = new_pfx;
    return old_prefix;
}

void EvMsg::dispatch ()
{
    r4Kernel*       kernel = _target->_kernel;
    kernel->_thread->_current_kernel = kernel;
    kernel->dispatch_evmsg(this);
}

void EvMsg::describe (ostream& os) const
{
    os << "POST " << _target->_sid << " at (" << _name << ") <- "
                  << _sender       << " at (" << _sender_state << ")";
}

// ===========================================================================
// EvAlarm

EvAlarm::EvAlarm (const TimeSpec& abs_time, const string& name, ValParam* arg,
                  ExecuteContext& cc)
:   EvUser(name, arg),
    _time_due(abs_time)
{
    _target       = cc.session;
    _sender_state = cc.state;
    _dsa_iter     = invalid_dsa_iter();
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

void EvAlarm::time_due (const TimeSpec& abs_time)
{
    assert(NULL == _link_queue.next);
    _time_due = abs_time;
}

void EvAlarm::dsa_iter (DueSidAid_Map::iterator iter)
{
    assert(NULL == _link_queue.next);
    _dsa_iter = iter;
}

void EvAlarm::s1a_iter (Sid1Aid_Map::iterator iter)
{
    assert(NULL == _link_queue.next);
    _s1a_iter = iter;
}

void EvAlarm::dispatch ()
{
    r4Kernel*       kernel = _target->_kernel;
    kernel->_thread->_current_kernel = kernel;
    kernel->dispatch_alarm(this);
}

void EvAlarm::describe (ostream& os) const
{
    os << "ALARM " << _target->_sid << " at (" << _name << " <- "
                                               << _sender_state << ")";
}

// ===========================================================================
// EvIO

EvIO::EvIO (int fd, IO_Mode mode, const string& name, ValParam* arg,
            ExecuteContext& cc)
:   EvUser(name, arg),
    _fd(fd),
    _mode(mode),
    _active(true)
{
    _target       = cc.session;
    _sender_state = cc.state;
}

EvIO::~EvIO ()
{
    assert(NULL == _link_evio.next);
    assert(! _active);
}

void EvIO::active (bool a)
{
    _active = a;
}

void EvIO::dispatch ()
{
    r4Kernel*       kernel = _target->_kernel;
    kernel->_thread->_current_kernel = kernel;
    kernel->dispatch_evio(this);
}

void EvIO::describe (ostream& os) const
{
    os << "I/O " << _target->_sid << " at (" << _name << " <- "
                                             << _sender_state << ")";
}

// ===========================================================================
// EvSys_Export_Kernel

EvSys_Export_Kernel::~EvSys_Export_Kernel ()
{
}

void EvSys_Export_Kernel::dispatch ()
{
    d4Thread::_export_kernel_local_data(_kernel);           // --@@--
    delete this;
}

bool EvSys_Export_Kernel::is_event_of (KiD kernel) const
{
    return _kernel->_kid == kernel;
}

// ===========================================================================
// EvSys_Import_Kernel

EvSys_Import_Kernel::~EvSys_Import_Kernel ()
{
}

void EvSys_Import_Kernel::dispatch ()
{
    _kernel->_thread->_import_kernel_local_data(*this);     // --@@--
    delete this;
}

bool EvSys_Import_Kernel::is_event_of (KiD kernel) const
{
    return _kernel->_kid == kernel;
}

// ===========================================================================
// EvSys_DeleteSession

EvSys_DeleteSession::~EvSys_DeleteSession ()
{
}

void EvSys_DeleteSession::dispatch ()
{
    assert(_session->local.stopper.isset());
    assert(_session->local.stopper == _session->_sid);

    _session->destroy();
    delete this;
}

bool EvSys_DeleteSession::is_event_of (KiD kernel) const
{
    return _session->_kernel->_kid == kernel;
}

// ===========================================================================

static DueSidAid_Map    __AlwaysEmpty_DSA;

DueSidAid_Map::iterator coe::invalid_dsa_iter ()
{
    return __AlwaysEmpty_DSA.end();
}

