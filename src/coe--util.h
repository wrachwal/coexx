// $Id$

/*************************************************************************
Copyright (c) 2008 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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
*************************************************************************/

#ifndef __COE__UTIL_H
#define __COE__UTIL_H

#include <map>
#include <limits>   // numeric_limits<>

namespace coe { /////

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

    template<typename _ExistsPredicate>
    _Id generate_next (const _ExistsPredicate& exists);

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

template<typename _Id>
    template<typename _ExistsPredicate>
_Id IdentGenerator<_Id>::generate_next (const _ExistsPredicate& exists)
{
    if (_last == _IdentTraits<_Id>::last_value(_last)) {
        _overflow = true;
        _last = _IdentTraits<_Id>::first_value(_last);
    }
    else {
        _last = _IdentTraits<_Id>::next_value(_last);
    }
    if (_overflow) {
        while (exists(_last)) {
            // next value
            _last = _IdentTraits<_Id>::next_value(_last);
            if (_last == _IdentTraits<_Id>::last_value(_last))
                _last = _IdentTraits<_Id>::first_value(_last);
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

} ///// namespace coe

#endif

