// coe-session.h

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

#ifndef __COE_SESSION_H
#define __COE_SESSION_H

#include "coe-kernel.h"

struct r4Session;

// =======================================================================
// Session

class Session {
public:
    SiD ID () const;

protected:
    Session ();
    virtual ~Session ();

    void  set_heap (void* heap);        // handlers will get it in EvCtx
    void* get_heap () const;

    Kernel* start_as_new_kernel_session ();

    /*
     * session management
     */
    virtual void _start (EvCtx& ctx) = 0;

private:
    Session (const Session&);           // prohibited
    void operator= (const Session&);    // prohibited

    friend struct r4Kernel;
    friend struct r4Session;
    r4Session*   _r4session;
};

// =======================================================================

#endif

