// mem-postback-sev.cpp

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

#include "coe-memory.h"
#include "coe-sys-sync.h"

using namespace std;
using namespace coe;

// ---------------------------------------------------------------------------

static Mutex        s_Mutex;
static Stats_Memory s_Stats;

// ===========================================================================
// get_stats_memory__sev_postback()

void coe::get_stats_memory__sev_postback (Stats_Memory& stats)
{
    Mutex::Guard    lock(::s_Mutex);
    stats = ::s_Stats;
}

// ===========================================================================
// Postback

void* coe::allocate__sev_postback ()
{
    // --@@--
    {
        Mutex::Guard    lock(::s_Mutex);
        s_Stats.now.objects += 1;
        s_Stats.sum.objects += 1;
    }
    return ::operator new(SIZEOF__sev_postback);
}

void coe::deallocate__sev_postback (void* mem)
{
    // --@@--
    {
        Mutex::Guard    lock(::s_Mutex);
        s_Stats.now.objects -= 1;
    }
    ::operator delete(mem);
}

