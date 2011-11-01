// coe-sys-time.h

/*****************************************************************************
Copyright (c) 2008-2011 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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

#ifndef __COE_SYS_TIME_H
#define __COE_SYS_TIME_H

#include <time.h>           // timespec
#include <iosfwd>           // ostream

namespace coe { /////

// ===========================================================================
// TimeSpec

struct TimeSpec : public timespec {
             TimeSpec ()                      { tv_sec = tv_nsec = 0; }
    explicit TimeSpec (const timespec& ts);
    explicit TimeSpec (double sec);
    explicit TimeSpec (time_t sec)            { tv_sec = sec; tv_nsec = 0; }
             TimeSpec (time_t sec, long nsec) { tv_sec = sec; tv_nsec = nsec; }

    static TimeSpec ZERO () { return TimeSpec(); }

    int compare (const TimeSpec& rhs) const;    // [ -1, 0, 1 ]

    TimeSpec& operator+= (const TimeSpec& add);
    TimeSpec& operator-= (const TimeSpec& sub);
};

// ---------------------------------------------------------------------------

inline bool operator== (const TimeSpec& lhs, const TimeSpec& rhs)
    { return lhs.tv_sec == rhs.tv_sec && lhs.tv_nsec == rhs.tv_nsec; }

inline bool operator!= (const TimeSpec& lhs, const TimeSpec& rhs)
    { return lhs.tv_sec != rhs.tv_sec || lhs.tv_nsec != rhs.tv_nsec; }

inline bool operator<  (const TimeSpec& lhs, const TimeSpec& rhs)
    { return lhs.compare(rhs) <  0; }

inline bool operator<= (const TimeSpec& lhs, const TimeSpec& rhs)
    { return lhs.compare(rhs) <= 0; }

inline bool operator>  (const TimeSpec& lhs, const TimeSpec& rhs)
    { return lhs.compare(rhs) >  0; }

inline bool operator>= (const TimeSpec& lhs, const TimeSpec& rhs)
    { return lhs.compare(rhs) >= 0; }

inline TimeSpec operator+ (const TimeSpec& lhs, const TimeSpec& rhs)
    {
        TimeSpec result(lhs);
        result += rhs;
        return result;
    }
inline TimeSpec operator- (const TimeSpec& lhs, const TimeSpec& rhs)
    {
        TimeSpec result(lhs);
        result -= rhs;
        return result;
    }

// ---------------------------------------------------------------------------

std::ostream& operator<< (std::ostream& os, const TimeSpec& ts);

// ===========================================================================

} ///// namespace coe

#endif

