// coe--ev.cpp

#include "coe--ev.h"
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
    delete _arg;
    _arg = NULL;    // just in case ;)
}

// =======================================================================
// EvMsg

EvMsg::EvMsg (const string& name, PostArg* arg, SessionContext& cc)
  : EvCommon(name, arg)
{
    _sender       = cc.session->_sid;   //TODO: local/foreign adjustment
    _sender_state = cc.state;
}

void EvMsg::dispatch ()
{
    _target->_kernel->dispatch_evmsg(this);
}

