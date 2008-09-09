// coe-session.cpp

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
    return _r4session ? _r4session->_sid : SiD::NONE();
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

Kernel* Session::start_as_new_kernel_session ()
{
    return NULL;    //TODO
}

