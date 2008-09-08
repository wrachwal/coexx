// coe--event.cpp

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

// =======================================================================
// EvIO

