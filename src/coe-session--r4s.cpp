// coe-session--r4s.cpp

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

#include "coe-session.h"
#include "coe-session--r4s.h"
#include "coe-thread--d4t.h"

#include <iostream>

using namespace std;
using namespace coe;

// ===========================================================================

r4Session::r4Session (StateCmd* start_handler)
:   _start_handler(start_handler)
{
    _handle       = NULL;
    _stop_handler = NULL;
    _heap         = NULL;
    _kernel       = NULL;
    _parent       = NULL;
}

r4Session::~r4Session ()
{
    assert(_r4Session::list_children(*this).empty());
    assert(NULL == _link_children.next);

    delete _start_handler;
    _start_handler = NULL;

    delete _stop_handler;
    _stop_handler = NULL;
}

void r4Session::destroy ()
{
    // first recursively destroy() all descendants
    _r4Session::ChildrenList::iterator  child(_r4Session::list_children(*this));
    while (child) {
        (*child++)->destroy();
    }

    //
    // next delete the whole session object representation
    //

    // (1) handle object
    delete _handle;
    // resource is detached in handle object destructor;
    // the assertion is to check that.
    assert(NULL == _handle);

    // (2) remove resource object from parent's list of children
    if (NULL != _parent) {
        r4Session*  removed = _r4Session::list_children(*_parent).remove(this);
                    removed = removed;
        assert(NULL != removed);
    }

    // (3) finally delete the resource object itself
    delete this;
}

// ---------------------------------------------------------------------------

static void _set_stopper (SiD stopper, r4Session& node)     // --@@--
{
    if (node.local.stopper.isset())
        return;

    node.local.stopper = stopper;

    _r4Session::ChildrenList::iterator  child(_r4Session::list_children(node));
    while (child) {
        _set_stopper(stopper, **child++);
    }
}

// ------------------------------------

void r4Session::_call_stop (r4Session& root, r4Session& node)
{
    if (node.local.stopper != root._sid)
        return;

    _r4Session::ChildrenList::iterator  child(_r4Session::list_children(node));
    while (child) {
        _call_stop(root, **child++);
    }

    assert(NULL != root._kernel);
    assert(root._kernel == node._kernel);

    root._kernel->call_stop(root, node);
}

// ------------------------------------

static void _remove_sid (SiD stopper, r4Session& node)      // --@@--
{
    if (node.local.stopper != stopper)
        return;

    Sid_Map::size_type  removed = node._kernel->local.sid_map.erase(node._sid);
                        removed = removed;
    assert(1 == removed);

    _r4Session::ChildrenList::iterator  child(_r4Session::list_children(node));
    while (child) {
        _remove_sid(stopper, **child++);
    }
}

// ------------------------------------

void r4Session::stop_session_tree ()
{
    assert(_sid.isset());
    assert(! local.stopper.isset());

    //
    // Recursively from this one mark all session(s) as being stopped.
    // NOTES:
    //  a) current session is called the `stopper'.
    //  b) sessions being stopped can do anything except the following:
    //     - starting new child sessions (as they are about to die)
    //     - being a target of any kind of event (but can be call'ed)
    //     - consequently any pending events will be simply ignored.
    //
    {
        // --@@--
        RWLock::Guard   guard(_kernel->local.rwlock, RWLock::WRITE);
        _set_stopper(_sid, *this);
    }

    //
    // Post system event to delete the session tree pointed by this.
    // Upon event dispatch, this->destroy() will be done.
    //
    {
        // --@@--
        Mutex::Guard    guard(_kernel->_thread->sched.mutex);
        _kernel->_thread->sched.pending.put_tail(new EvSys_DeleteSession(this));
    }

    //
    // Recursively from the bottom up, on each session visited
    // perform user defined cleanup code in that order:
    //  - call _stop_handler(), if registered.
    //  - call all unregistrar functions, if any.
    //
    _call_stop(*this, *this);

    //
    // Remove all session(s) from kernel's local.sid_map;
    //
    {
        // --@@--
        RWLock::Guard   guard(_kernel->local.rwlock, RWLock::WRITE);
        _remove_sid(_sid, *this);
    }
}

// ---------------------------------------------------------------------------

EvIO* r4Session::find_io_watcher (int fd, IO_Mode mode)
{
   for (_EvIO::List::iterator i = _list_evio.begin(); i != _list_evio.end(); ++i) {
       if ((*i)->fd() == fd && (*i)->mode() == mode) {
           return *i;
       }
   }
   return NULL;
}

