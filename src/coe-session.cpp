// coe-session.cpp

#include "coe-session.h"
#include "coe-session--r4s.h"

// =======================================================================

Session::Session () : _r4session(NULL)
{
    _r4session = new r4Session;
}

Session::~Session ()
{
    if (NULL != _r4session) {   // resource is attached
        r4Session*  res = _r4session;
        // detach resource from this object
        _r4session   = NULL;
        res->_handle = NULL;
        // release detached resource
        res->release_resource();
    }
}

SiD Session::ID () const
{
    return _r4session ? _r4session->_sid : SiD::NONE;
}

void Session::set_heap (void* heap)
{
    if (NULL != _r4session)
        _r4session->_heap = heap;
}

void* Session::get_heap () const
{
    return _r4session ? _r4session->_heap : NULL;
}

