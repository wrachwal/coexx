// coe-session.h

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

#ifndef __COE_SESSION_H
#define __COE_SESSION_H

#include "coe-kernel.h"

namespace coe { /////

struct r4Session;   // private data

// ===========================================================================
// Session

class Session : private _Noncopyable {
public:
    void start_handler (const HandlerX& handler);
    SiD  start_session (Kernel& kernel,                  EventArg* arg=0);
    SiD  start_session (Kernel& kernel, Session& parent, EventArg* arg=0);

    SiD ID () const;
    void           set_slabel (const std::string& label);
    const std::string& slabel () const;

    bool unregistrar_set    (void (*)(SiD));
    bool unregistrar_remove (void (*)(SiD));

    void stop_handler (const Handler0& handler);
    bool stop_session ();

    static Session* current_session ();

    Session* parent_session ();
    Session*  child_session (Session* prev);

    bool call          (bool warn, Kernel& kernel, const CoeStr& ev, EventArg* arg=0);
    bool call_keep_arg (bool warn, Kernel& kernel, const CoeStr& ev, EventArg& arg);

protected:
    Session (const HandlerX& start_handler);
    virtual ~Session ();

private:
    friend struct r4Kernel;
    friend struct r4Session;
    r4Session*   _r4session;
};

// ===========================================================================

} ///// namespace coe

#endif

