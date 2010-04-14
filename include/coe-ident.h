// coe-ident.h

/*****************************************************************************
Copyright (c) 2008, 2009 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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

#ifndef __COE_IDENT_H
#define __COE_IDENT_H

#include <ostream>

namespace coe { /////

// ===========================================================================
// TiD -- thread identity

class TiD {
public:
    typedef int IntType;
             TiD ()          : _t(0) {}
    explicit TiD (IntType t) : _t(t) {}

    operator void* () const { return reinterpret_cast<void*>(isset()); }

    bool isset () const { return 0 != _t; }
    IntType id () const { return _t; }

    bool operator== (const TiD& rhs) const { return _t == rhs._t; }
    bool operator!= (const TiD& rhs) const { return _t != rhs._t; }
    bool operator<  (const TiD& rhs) const { return _t <  rhs._t; }

    static TiD NONE () { return TiD(); }

private:
    IntType _t;
};

// ----------------

std::ostream& operator<< (std::ostream& os, TiD tid);

// ---------------------------------------------------------------------------
// KiD -- kernel identity

class KiD {
public:
    typedef int IntType;
             KiD ()          : _k(0) {}
    explicit KiD (IntType k) : _k(k) {}

    operator void* () const { return reinterpret_cast<void*>(isset()); }

    bool isset () const { return 0 != _k; }
    IntType id () const { return _k; }

    bool operator== (const KiD& rhs) const { return _k == rhs._k; }
    bool operator!= (const KiD& rhs) const { return _k != rhs._k; }
    bool operator<  (const KiD& rhs) const { return _k <  rhs._k; }

    static KiD NONE () { return KiD(); }

private:
    IntType _k;
};

// ----------------

std::ostream& operator<< (std::ostream& os, KiD kid);

// ===========================================================================
// SiD -- session identity

class SiD {
public:
    typedef int IntType;
    SiD ()                 : _s(0), _k(0) {}
    SiD (KiD k, IntType s) : _s(s), _k(k) {}

    operator void* () const { return reinterpret_cast<void*>(isset()); }

    bool isset () const { return 0 != _s; }
    KiD    kid () const { return _k; }
    IntType id () const { return _s; }

    bool is_kernel () const { return 1 == _s; }

    bool operator== (const SiD& rhs) const
        { return _k == rhs._k && _s == rhs._s; }
    bool operator!= (const SiD& rhs) const
        { return _k != rhs._k || _s != rhs._s; }
    bool operator<  (const SiD& rhs) const
        { return _k < rhs._k || (_k == rhs._k && _s < rhs._s); }

    static SiD NONE () { return SiD(); }

private:
    IntType _s;
    KiD     _k;
};

// ----------------

std::ostream& operator<< (std::ostream& os, const SiD& sid);

// ---------------------------------------------------------------------------
// AiD -- alarm identity (per session resource)

class AiD {
public:
    typedef int IntType;
             AiD ()          : _a(0) {}
    explicit AiD (IntType a) : _a(a) {}

    operator void* () const { return reinterpret_cast<void*>(isset()); }

    bool isset () const { return 0 != _a; }
    IntType id () const { return _a; }

    bool operator== (const AiD& rhs) const { return _a == rhs._a; }
    bool operator!= (const AiD& rhs) const { return _a != rhs._a; }
    bool operator<  (const AiD& rhs) const { return _a <  rhs._a; }

    static AiD NONE () { return AiD(); }

private:
    IntType _a;
};

// ----------------

std::ostream& operator<< (std::ostream& os, AiD aid);

// ===========================================================================

} ///// namespace coe

#endif

