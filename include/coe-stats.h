// coe-stats.h

/*****************************************************************************
Copyright (c) 2008-2019 Waldemar Rachwał <waldemar.rachwal@gmail.com>

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

#ifndef __COE_STATS_H
#define __COE_STATS_H

#include <cstdlib>

namespace coe { /////

class Kernel;
class Thread;

// ===========================================================================

struct Stats_Kernel {
    struct Counters {
        Counters ();
        size_t  sessions;
        size_t  handlers;
        size_t  alarms;
    } curr;
};

// ------------------------------------

struct Stats_Thread {
    struct Counters {
        Counters ();
        size_t  kernels;
        size_t  alarms;
        size_t  watchers_all;
        size_t  watchers_active;
        size_t  lqueue_size;
        size_t  pqueue_size;
    } curr;
};

// ===========================================================================

} ///// namespace coe

#endif

