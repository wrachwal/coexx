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

#include "coe-session.h"
#include "coe-session--r4s.h"

#include <iostream>

using namespace std;
using namespace coe;

// =======================================================================

r4Session::r4Session ()
{
    _handle = NULL;
    _heap   = NULL;
    _kernel = NULL;
    _parent = NULL;
}

r4Session::~r4Session ()
{
    assert(_r4Session::list_children(*this).empty());
    assert(NULL == _link_children.next);
    //TODO: any cleanup?
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

EvIO* r4Session::find_io_watcher (int fd, IO_Mode mode)
{
   for (_EvIO::List::iterator i = _list_evio.begin(); i != _list_evio.end(); ++i) {
       if ((*i)->fd() == fd && (*i)->mode() == mode) {
           return *i;
       }
   }
   return NULL;
}

