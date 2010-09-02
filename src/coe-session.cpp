// coe-session.cpp

/*****************************************************************************
Copyright (c) 2008-2010 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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

#include "coe-session.h"
#include "coe-session--r4s.h"
#include "coe-thread--d4t.h"
#include "coe--errno.h"

#include <algorithm>    // find

using namespace std;
using namespace coe;

// ===========================================================================
// SiD

SiD::SiD (const Session* s)
:   _s(0)
{
    if (s) {
        *this = s->ID();
    }
}

// ===========================================================================
// Session

Session::Session (const HandlerX& start_handler)
{
    _r4session = new r4Session(start_handler);
}

Session::~Session ()
{
    if (stop_session()) {
        _r4session->_handle = NULL;     // detach from the resource object
    }
    else {
        delete _r4session;
    }
}

// ---------------------------------------------------------------------------

void Session::start_handler (const HandlerX& handler)
{
    _r4session->_start_handler = handler;
}

SiD Session::start_session (Kernel& kernel, EventArg* arg)
{
    r4Kernel*   r4k = kernel._r4kernel;
    assert(NULL != r4k);

    SiD sid = r4k->start_session(this, arg);
    delete arg;
    return sid;
}

// ---------------------------------------------------------------------------

void Session::stop_handler (const Handler0& handler)
{
    _r4session->_stop_handler = handler;
}

bool Session::stop_session ()
{
    if (! _r4session->_sid.isset())
        return false;
    if (_r4session->local.stopper.isset())
        return true;
    _r4session->stop_session_tree();    // --@@--
    return true;
}

// ---------------------------------------------------------------------------

SiD Session::ID () const
{
    return _r4session->_sid;
}

void Session::set_slabel (const string& label)
{
    _r4session->_slabel = label;
}

const string& Session::slabel () const
{
    return _r4session->_slabel;
}

// ---------------------------------------------------------------------------

bool Session::unregistrar_set (void (*unreg)(SiD))
{
    if (NULL == unreg)
        return false;

    vector<Unregistrar>&    unregvec = _r4session->_unregistrar;
    if (find(unregvec.begin(), unregvec.end(), unreg) != unregvec.end())
        return false;

    if (unregvec.empty())
        unregvec.reserve(5);    // instead of a few kB at the start
    unregvec.push_back(unreg);
    return true;
}

bool Session::unregistrar_remove (void (*unreg)(SiD))
{
    if (NULL == unreg)
        return false;

    vector<Unregistrar>&    unregvec = _r4session->_unregistrar;
    vector<Unregistrar>::iterator  i = find(unregvec.begin(), unregvec.end(), unreg);
    if (i == unregvec.end())
        return false;

    unregvec.erase(i);
    return true;
}

// ---------------------------------------------------------------------------

Session* Session::current_session ()
{
    d4Thread*   thread = d4Thread::get_d4t_tls();
    if (NULL != thread) {
        r4Kernel*   kernel = thread->_current_kernel;
        if (NULL != kernel) {
            assert(NULL != kernel->_current_context);
            assert(NULL != kernel->_current_context->session);
            return kernel->_current_context->session->_handle;
        }
    }
    return NULL;
}

// ---------------------------------------------------------------------------

Session* Session::get_session_parent ()
{
    r4Session*  parent = _r4session->_parent;
    return parent ? parent->_handle : NULL;
}

Session* Session::get_session_next_sibling (Session* prev)
{
    if (NULL == prev) {
        r4Session*  head = _r4Session::list_children(*_r4session).peek_head();
        return head ? head->_handle : NULL;
    }
    else {
        if (prev->_r4session->_parent != _r4session) {  // is `prev' really my child?
            return NULL;    // error case: `prev' is not mine!
        }

        r4Session*  next = prev->_r4session->_link_children.next;

        if (_r4Session::list_children(*_r4session).peek_head() == next) {
            return NULL;    // stop iteration
        }
        else {
            return next->_handle;
        }
    }
}

