// coe-ident.h

#ifndef __COE_IDENT_H
#define __COE_IDENT_H

typedef int NiD;

// =======================================================================
// TiD -- thread identity

class TiD {
public:
             TiD ()      : _t(0) {}
    explicit TiD (NiD t) : _t(t) {}

    static TiD NONE () { return TiD(); }

    bool valid () const { return 0 != _t; }

    NiD nid () const { return _t; }

    bool operator== (const TiD& rhs) const { return _t == rhs._t; }
    bool operator<  (const TiD& rhs) const { return _t <  rhs._t; }

private:
    NiD _t;
};

// -----------------------------------------------------------------------
// KiD -- kernel identity

class KiD {
public:
             KiD ()      : _k(0) {}
    explicit KiD (NiD k) : _k(k) {}

    static KiD NONE () { return KiD(); }

    bool valid () const { return 0 != _k; }

    NiD nid () const { return _k; }

    bool operator== (const KiD& rhs) const { return _k == rhs._k; }
    bool operator<  (const KiD& rhs) const { return _k <  rhs._k; }

private:
    NiD _k;
};

// =======================================================================
// SiD -- session identity

class SiD {
public:
    SiD ()             : _k(0), _s(0) {}
    SiD (KiD k, NiD s) : _k(k), _s(s) {}

    static SiD NONE () { return SiD(); }

    bool valid () const { return 0 != _s; }     //FIXME: test _k as well?!

    KiD kid () const { return _k; }
    NiD nid () const { return _s; }

    bool is_kernel () const { return 1 == _s; }

    bool operator== (const SiD& rhs) const
        { return _k == rhs._k && _s == rhs._s; }
    bool operator<  (const SiD& rhs) const
        { return _k < rhs._k || _k == rhs._k && _s < rhs._s; }

private:
    KiD _k;
    NiD _s;
};

// -----------------------------------------------------------------------
// AiD -- alarm identity (per session resource)

class AiD {
public:
             AiD ()      : _a(0) {}
    explicit AiD (NiD a) : _a(a) {}

    static AiD NONE () { return AiD(); }

    bool valid () const { return 0 != _a; }

    NiD nid () const { return _a; }

    bool operator== (const AiD& rhs) const { return _a == rhs._a; }
    bool operator<  (const AiD& rhs) const { return _a <  rhs._a; }

private:
    NiD _a;
};

// =======================================================================

#endif

