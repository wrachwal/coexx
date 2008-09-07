// coe--event.cpp

#include "coe--event.h"
#include "coe-kernel--r4k.h"
#include "coe-session--r4s.h"

using namespace std;

// =======================================================================
// EvCommon

EvCommon::EvCommon (const string& name, PostArg* arg)
  : _target(NULL),
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

void EvMsg::dispatch ()
{
    _target->_kernel->dispatch_evmsg(this);
}

// =======================================================================
// EvAlarm

// =======================================================================
// EvIO

