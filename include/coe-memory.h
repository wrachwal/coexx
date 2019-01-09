// coe-memory.h

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

#ifndef __COE_MEMORY_H
#define __COE_MEMORY_H

#include <cstddef>
#include <inttypes.h>

namespace coe { /////

// ===========================================================================
// replaceable (de)allocation functions

/// _Sev_Callback
extern const size_t SIZEOF__sev_callback;
void*             allocate__sev_callback ();
void            deallocate__sev_callback (void*);

/// _Sev_Postback
extern const size_t SIZEOF__sev_postback;
void*             allocate__sev_postback ();
void            deallocate__sev_postback (void*);

// ===========================================================================
// Stats_Memory

struct Stats_Memory {
    struct Now { Now ();   size_t objects; } now;
    struct Sum { Sum (); uint64_t objects; } sum;
};

// ------------------------------------

void get_stats_memory__session      (Stats_Memory& stats);
void get_stats_memory__sev_callback (Stats_Memory& stats);
void get_stats_memory__sev_postback (Stats_Memory& stats);

// ===========================================================================

} ///// namespace coe

#endif

