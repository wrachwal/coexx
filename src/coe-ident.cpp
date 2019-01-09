// coe-ident.cpp

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

#include "coe-ident.h"

#include <ostream>
#include <cassert>

using namespace std;

// ===========================================================================
// TiD

ostream& coe::operator<< (ostream& os, TiD tid)
{
    if (tid.id())
        os << "{t" << tid.id() << '}';
    else
        os << "{t!}";
    return os;
}

// ---------------------------------------------------------------------------
// KiD

ostream& coe::operator<< (ostream& os, KiD kid)
{
    if (kid.id())
        os << "{k" << kid.id() << '}';
    else
        os << "{k!}";
    return os;
}

// ---------------------------------------------------------------------------
// SiD

ostream& coe::operator<< (ostream& os, const SiD& sid)
{
    SiD::IntType    s = sid.id();
    KiD::IntType    k = sid.kid().id();

    enum { X = 2, MOV = 2 };

    int _s = (0 == s || 1 == s) ? s : X;
    int _k = (0 == k)           ? k : X;

    switch ((_s << 2)  + _k) {
        default:                assert(0);
        // --- invalid forms
        //   _s          _k
        case (0 << MOV) + 0:    return os << "{s!}";
        case (0 << MOV) + X:    return os << "{s!k" << k << '}';
        case (1 << MOV) + 0:    return os << "{sk!}";
        case (X << MOV) + 0:    return os << "{s" << s << "k!}";
        // ----- valid forms
        //   _s          _k
        case (1 << MOV) + X:    return os << "{s:k" << k << '}';
        case (X << MOV) + X:    return os << "{s" << s << ":k" << k << '}';
    }
}

// ---------------------------------------------------------------------------
// AiD

ostream& coe::operator<< (ostream& os, AiD aid)
{
    if (aid.id())
        os << "<a" << aid.id() << '>';
    else
        os << "<a!>";
    return os;
}

