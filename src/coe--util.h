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
    IdentGenerator () : _last(), _overflow(false) {}
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
    _last = _IdentTraits<_Id>::next_value(_last);
    if (_last == _IdentTraits<_Id>::last_value(_last)) {
        _overflow = true;
        _last = _IdentTraits<_Id>::first_value(_last);
    }
    if (_overflow) {

        //TODO
        //search _last, and if not found, accept the value,
        //else do iterate over map incrementing _last then compare.
        //the first value of _last which differs, is the one
        //we looked for.
        //if we reach value_last() in the meantime we have to start
        //from the beginning of the map.
        //algorithm would drop into infinite loop if map is full,
        //which however is unlikely.

        typename std::map<_Id, _Va>::const_iterator
            lwr = map.lower_bound(_last);
        typename std::map<_Id, _Va>::const_iterator
            upr = map.upper_bound(_last);
        lwr = upr;

        typename std::map<_Id, _Va>::const_iterator i = map.find(_last);
        if (i != map.end()) {   // exists, need to continue
            _last = _IdentTraits<_Id>::next_value(_last);
            if (_last == _IdentTraits<_Id>::last_value(_last)) {
                _last = _IdentTraits<_Id>::first_value(_last);
            }
            //TODO: finish this algo, then write simple tests
            //using '(un)signed char'
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
        { return _Id(std::numeric_limits<typename _Id::int_type>::max() - 1); }
    static _Id next_value (_Id prev)
        { return _Id(prev.nid() + 1); }
};

template<>              // specialization for SiD
struct _IdentTraits<SiD> {
    static SiD first_value (SiD pattern)
        { return SiD(pattern.kid(), 1); }
    static SiD last_value (SiD pattern)
        { return SiD(pattern.kid(), std::numeric_limits<SiD::int_type>::max() - 1); }
    static SiD next_value (SiD prev)
        { return SiD(prev.kid(), prev.nid() + 1); }
};

// =======================================================================

#endif

