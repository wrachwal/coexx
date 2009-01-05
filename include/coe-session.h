// $Id$

/*****************************************************************************
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
*****************************************************************************/

#ifndef __COE_SESSION_H
#define __COE_SESSION_H

#include "coe-kernel.h"

namespace coe { /////

// ---------------------------------------------------------------------------

class Callback;

struct r4Session;   // private

// ===========================================================================
// Session

class Session : private _Noncopyable {
public:
    SiD ID () const;

    bool unregistrar_set    (void (*)(SiD));
    bool unregistrar_remove (void (*)(SiD));

    /*
     * Encapsulated `Callback' primitive
     */
    Callback* callback (const std::string& ev, ValParam* pfx=0);

protected:
    Session (StateCmd* start_handler);
    virtual ~Session ();

    void  set_heap (void* heap);        // handlers will get it in EvCtx
    void* get_heap () const;

    /*
     * Session Management
     */
    SiD start_session (Kernel& kernel, EventArg* arg=0);
    bool stop_session ();

    virtual void _stop  (EvCtx& ctx);

private:
    friend  class EvCtx;                // get_heap()
    friend struct r4Kernel;
    friend struct r4Session;
    r4Session*   _r4session;
};

// ---------------------------------------------------------------------------
// Callback

class Callback : private _Noncopyable {
public:
    ~Callback ();

    SiD session () const { return _target; }

    bool      call (Kernel& kernel);
    bool      call (Kernel& kernel, RefParam* arg);
    bool      call (Kernel& kernel, ValParam* arg);

    bool      post (Kernel& kernel, ValParam* arg=0);
    bool anon_post (                ValParam* arg=0);

private:
    friend class Session;
    Callback (SiD, const std::string&, ValParam*);

    SiD         _target;
    std::string _evname;
    ValParam*   _prefix;
};

// ===========================================================================

} ///// namespace coe

#endif

