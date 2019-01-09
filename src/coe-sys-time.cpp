// coe-sys-time.cpp

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

#include "coe-sys-time.h"

#include <ostream>

using namespace std;
using namespace coe;

// ===========================================================================
// TimeSpec

TimeSpec::TimeSpec (double sec)
{
    tv_sec  = time_t(sec);
    tv_nsec = long((sec - double(tv_sec)) * 1e9);
    if (tv_nsec < 0) {
        tv_sec --;
        tv_nsec += 1000000000L;
    }
}

int TimeSpec::compare (const TimeSpec& rhs) const
{
    if (    tv_sec < rhs.tv_sec)
        return -1;
    if (rhs.tv_sec <     tv_sec)
        return +1;

    if (    tv_nsec < rhs.tv_nsec)
        return -1;
    if (rhs.tv_nsec <     tv_nsec)
        return +1;

    return 0;
}

TimeSpec& TimeSpec::operator+= (const TimeSpec& add)
{
    tv_sec  += add.tv_sec;
    tv_nsec += add.tv_nsec;

    if (tv_nsec >= 1000000000L) {
        tv_sec ++;
        tv_nsec -= 1000000000L;
    }
    return *this;
}

TimeSpec& TimeSpec::operator-= (const TimeSpec& sub)
{
    tv_sec  -= sub.tv_sec;
    tv_nsec -= sub.tv_nsec;

    if (tv_nsec < 0) {
        tv_sec --;
        tv_nsec += 1000000000L;
    }
    return *this;
}

// ---------------------------------------------------------------------------

ostream& coe::operator<< (ostream& os, const TimeSpec& ts)
{
    return os << ts.tv_sec + 1e-9 * ts.tv_nsec;
}

