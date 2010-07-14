// coe-thread.h

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

#ifndef __COE_THREAD_H
#define __COE_THREAD_H

#include "coe-ident.h"
#include "coe-global.h"     // Factory<T>

#include <typeinfo>

namespace coe { /////

// ---------------------------------------------------------------------------

struct Stats_Thread;

// ===========================================================================
// Thread

struct d4Thread;
struct _TlsD;

class Thread : private _Noncopyable {
public:
    static TiD spawn_new (bool (*quit)(Thread&) = 0);

    TiD ID () const;
    void           set_tlabel (const std::string& label);
    const std::string& tlabel () const;

    void run_event_loop (bool (*quit)(Thread&) = 0);

    static Thread* current_thread ();
    Kernel* kernel ();

    typedef Stats_Thread Stats;
    void get_stats (Stats& stats);

    template<class T>
    T& tls ();
    static void* next_tls_info (void* iter, LocalStorageInfo& info);

private:
    friend struct d4Thread;
    Thread ();
    void* _get_user_tls (const _TlsD*);

    d4Thread*   _d4thread;
};

// ===========================================================================

} ///// namespace coe

#include "coe-thread--imp.h"

#endif

