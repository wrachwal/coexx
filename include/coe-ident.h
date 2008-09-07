// coe-ident.h

#ifndef __COE_IDENT_H
#define __COE_IDENT_H

// =======================================================================
// TiD -- thread identity

class TiD {
public:
    typedef int IntType;
             TiD ()          : _t(0) {}
    explicit TiD (IntType t) : _t(t) {}

    bool valid () const { return 0 != _t; }
    IntType id () const { return _t; }

    bool operator== (const TiD& rhs) const { return _t == rhs._t; }
    bool operator<  (const TiD& rhs) const { return _t <  rhs._t; }

    static TiD NONE () { return TiD(); }

private:
    IntType _t;
};

// -----------------------------------------------------------------------
// KiD -- kernel identity

class KiD {
public:
    typedef int IntType;
             KiD ()          : _k(0) {}
    explicit KiD (IntType k) : _k(k) {}

    bool valid () const { return 0 != _k; }
    IntType id () const { return _k; }

    bool operator== (const KiD& rhs) const { return _k == rhs._k; }
    bool operator<  (const KiD& rhs) const { return _k <  rhs._k; }

    static KiD NONE () { return KiD(); }

private:
    IntType _k;
};

// =======================================================================
// SiD -- session identity

class SiD {
public:
    typedef int IntType;
    SiD ()                 : _s(0), _k(0) {}
    SiD (KiD k, IntType s) : _s(s), _k(k) {}

    bool valid () const { return 0 != _s; }     //FIXME: test _k as well?!
    KiD    kid () const { return _k; }
    IntType id () const { return _s; }

    bool is_kernel () const { return 1 == _s; }

    bool operator== (const SiD& rhs) const
        { return _k == rhs._k && _s == rhs._s; }
    bool operator<  (const SiD& rhs) const
        { return _k < rhs._k || _k == rhs._k && _s < rhs._s; }

    static SiD NONE () { return SiD(); }

private:
    IntType _s;
    KiD     _k;
};

// -----------------------------------------------------------------------
// AiD -- alarm identity (per session resource)

class AiD {
public:
    typedef int IntType;
             AiD ()          : _a(0) {}
    explicit AiD (IntType a) : _a(a) {}

    bool valid () const { return 0 != _a; }
    IntType id () const { return _a; }

    bool operator== (const AiD& rhs) const { return _a == rhs._a; }
    bool operator<  (const AiD& rhs) const { return _a <  rhs._a; }

    static AiD NONE () { return AiD(); }

private:
    IntType _a;
};

// =======================================================================

#endif

