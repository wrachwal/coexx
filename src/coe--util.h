// coe--util.h

#ifndef __COE__UTIL_H
#define __COE__UTIL_H

#include <map>
#include <limits>   // numeric_limits<>

// =======================================================================
// IdentGenerator<_Id>

template<typename> struct _IdentTraits;

template<typename _Id>
class IdentGenerator {
public:
             IdentGenerator ()         : _last(),     _overflow(false) {}
    explicit IdentGenerator (_Id last) : _last(last), _overflow(false) {}

    template<typename _Va>
    _Id generate_next (const std::map<_Id, _Va>& map);

private:
    _Id     _last;
    bool    _overflow;
};

// ------------------------------------

template<typename _Id>
    template<typename _Va>
_Id IdentGenerator<_Id>::generate_next (const std::map<_Id, _Va>& map)
{
    if (_last == _IdentTraits<_Id>::last_value(_last)) {
        _overflow = true;
        _last = _IdentTraits<_Id>::first_value(_last);
    }
    else {
        _last = _IdentTraits<_Id>::next_value(_last);
    }
    if (_overflow) {
        typename std::map<_Id, _Va>::const_iterator i = map.find(_last);
        if (i != map.end()) {   // exists, look for the first gap
            typename std::map<_Id, _Va>::const_iterator lwr = map.lower_bound(_last);
            typename std::map<_Id, _Va>::const_iterator upr = map.upper_bound(_last);
            do {
                // next value
                _last = _IdentTraits<_Id>::next_value(_last);
                if (_last == _IdentTraits<_Id>::last_value(_last))
                    _last = _IdentTraits<_Id>::first_value(_last);
                // next map key
                if (++i == upr)
                    i = lwr;
            } while (_last == (*i).first);
        }
    }
    return _last;
}

// ------------------------------------
// _IdentTraits<_Id>

template<typename _Id>  // traits for common ?iD
struct _IdentTraits {
    static _Id first_value (_Id/*pattern*/)
        { return _Id(1); }
    static _Id last_value (_Id/*pattern*/)
        { return _Id(std::numeric_limits<typename _Id::IntType>::max()); }
    static _Id next_value (_Id prev)
        { return _Id(prev.id() + 1); }
};

template<>              // specialization for SiD
struct _IdentTraits<SiD> {
    static SiD first_value (SiD pattern)
        { return SiD(pattern.kid(), 1); }
    static SiD last_value (SiD pattern)
        { return SiD(pattern.kid(), std::numeric_limits<SiD::IntType>::max()); }
    static SiD next_value (SiD prev)
        { return SiD(prev.kid(), prev.id() + 1); }
};

// =======================================================================

#endif

