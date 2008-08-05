// coe-session--r4s.cpp

#include "coe-session.h"
#include "coe-session--r4s.h"

// =======================================================================

r4Session::r4Session ()
{
    _handle = NULL;
    _heap   = NULL;
    _kernel = NULL;
    _parent = NULL;
    _children_list = NULL;
}

void r4Session::release_resource ()
{
    if (NULL != _handle) {  // resource is attached
        Session* session = _handle;
        // detach resource from handle object
        _handle             = NULL;
        session->_r4session = NULL;
        // ...then destroy handle object
        delete session;
    }
}

