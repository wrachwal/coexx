// coe-ident.h

#ifndef __COE_IDENT_H
#define __COE_IDENT_H

typedef unsigned long NiD;

// =======================================================================
// SiD

class SiD {
public:
    SiD () : _k(0), _s(0) {}

    static const SiD NONE;
    static const SiD KERNEL;

    bool operator== (const SiD& rhs) const
        { return _k == rhs._k && _s == rhs._s; }
    bool operator<  (const SiD& rhs) const
        { return _k < rhs._k || _k == rhs._k && _s < rhs._s; }

private:
    friend class r4Kernel;
    SiD (NiD k, NiD s) : _k(k), _s(s) {}
    NiD _k;
    NiD _s;
};

// =======================================================================

#endif

