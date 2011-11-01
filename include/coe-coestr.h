// coe-coestr.h

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

#ifndef __COE_COESTR_H
#define __COE_COESTR_H

/*
 *  COEXX_COESTR
 *      0 : fallback to std::string (safe but inefficient)
 *      1 : no runtime checks
 *      2 : runtime detection of string literals; assert/throw on failure
 *      3 : like 2 but conversion to std::string on failure (safe and efficient)
 */

#ifndef COEXX_COESTR
    #ifdef  __linux__
        #define COEXX_COESTR    3
    #else
        #define COEXX_COESTR    0
    #endif
#endif

#include <string>
#include <cstring>

namespace coe { /////

#if COEXX_COESTR

// ---------------------------------------------------------------------------

class CoeStr;

std::ostream& operator<< (std::ostream& os, const CoeStr& str);

// ===========================================================================
// CoeStr

class CoeStr {
public:
    CoeStr ();
    CoeStr (const CoeStr& s);

    CoeStr (const char* STRING_LITERAL);            // <==
    CoeStr (const std::string& s);

    ~CoeStr ();

    CoeStr& operator= (const CoeStr& rhs);
    CoeStr& operator= (const std::string& rhs);
    CoeStr& operator= (const char* STRING_LITERAL); // <==

    bool empty () const;
    const char* c_str () const;
    void clear ();

    friend std::ostream& operator<< (std::ostream& os, const CoeStr& str);

    int compare (const CoeStr&      rhs) const;
    int compare (const std::string& rhs) const;
    int compare (const char*        rhs) const;

private:
    const std::string& _xs () const { return *(const std::string*)(void*)_xb.xmem; }
          std::string& _xs ()       { return *(      std::string*)(void*)_xb.xmem; }
    const char* _cs;
    union {
        char    xmem[sizeof(std::string)];  // aligned to hold std::string
///     size_t  hash;
    } _xb;
    static const char*  _Empty;
};

// ---------------------------------------------------------------------------

inline bool operator<  (const CoeStr& s1, const CoeStr& s2)      { return s1.compare(s2) < 0; }
inline bool operator<  (const CoeStr& s1, const std::string& s2) { return s1.compare(s2) < 0; }
inline bool operator<  (const std::string& s1, const CoeStr& s2) { return s2.compare(s1) > 0; }
inline bool operator<  (const CoeStr& s1, const char* s2)        { return s1.compare(s2) < 0; }
inline bool operator<  (const char* s1, const CoeStr& s2)        { return s2.compare(s1) > 0; }

inline bool operator>  (const CoeStr& s1, const CoeStr& s2)      { return s1.compare(s2) > 0; }
inline bool operator>  (const CoeStr& s1, const std::string& s2) { return s1.compare(s2) > 0; }
inline bool operator>  (const std::string& s1, const CoeStr& s2) { return s2.compare(s1) < 0; }
inline bool operator>  (const CoeStr& s1, const char* s2)        { return s1.compare(s2) > 0; }
inline bool operator>  (const char* s1, const CoeStr& s2)        { return s2.compare(s1) < 0; }

inline bool operator<= (const CoeStr& s1, const CoeStr& s2)      { return s1.compare(s2) <= 0; }
inline bool operator<= (const CoeStr& s1, const std::string& s2) { return s1.compare(s2) <= 0; }
inline bool operator<= (const std::string& s1, const CoeStr& s2) { return s2.compare(s1) >= 0; }
inline bool operator<= (const CoeStr& s1, const char* s2)        { return s1.compare(s2) <= 0; }
inline bool operator<= (const char* s1, const CoeStr& s2)        { return s2.compare(s1) >= 0; }

inline bool operator>= (const CoeStr& s1, const CoeStr& s2)      { return s1.compare(s2) >= 0; }
inline bool operator>= (const CoeStr& s1, const std::string& s2) { return s1.compare(s2) >= 0; }
inline bool operator>= (const std::string& s1, const CoeStr& s2) { return s2.compare(s1) <= 0; }
inline bool operator>= (const CoeStr& s1, const char* s2)        { return s1.compare(s2) >= 0; }
inline bool operator>= (const char* s1, const CoeStr& s2)        { return s2.compare(s1) <= 0; }

inline bool operator== (const CoeStr& s1, const CoeStr& s2)      { return s1.compare(s2) == 0; }
inline bool operator== (const CoeStr& s1, const std::string& s2) { return s1.compare(s2) == 0; }
inline bool operator== (const std::string& s1, const CoeStr& s2) { return s2.compare(s1) == 0; }
inline bool operator== (const CoeStr& s1, const char* s2)        { return s1.compare(s2) == 0; }
inline bool operator== (const char* s1, const CoeStr& s2)        { return s2.compare(s1) == 0; }

inline bool operator!= (const CoeStr& s1, const CoeStr& s2)      { return s1.compare(s2) != 0; }
inline bool operator!= (const CoeStr& s1, const std::string& s2) { return s1.compare(s2) != 0; }
inline bool operator!= (const std::string& s1, const CoeStr& s2) { return s2.compare(s1) != 0; }
inline bool operator!= (const CoeStr& s1, const char* s2)        { return s1.compare(s2) != 0; }
inline bool operator!= (const char* s1, const CoeStr& s2)        { return s2.compare(s1) != 0; }

// ===========================================================================
// ///////////////////////////////////////////////////////////////////////////

inline CoeStr::CoeStr () : _cs(_Empty)
    {
    }

inline CoeStr::CoeStr (const CoeStr& s) : _cs(s._cs)
    {
        if (! _cs)
            new (_xb.xmem) std::string(s._xs());
    }

#if ! ( COEXX_COESTR >= 2 )
inline CoeStr::CoeStr (const char* s) : _cs(s ? s : _Empty)
    {
    }

inline CoeStr& CoeStr::operator= (const char* rhs)
    {
        using std::string;
        if (! _cs)
            _xs().~string();
        _cs = rhs ? rhs : _Empty;
        return *this;
    }
#endif

inline CoeStr::CoeStr (const std::string& s) : _cs(0)
    {
        new (_xb.xmem) std::string(s);
    }

inline CoeStr::~CoeStr ()
    {
        using std::string;
        if (! _cs)
            _xs().~string();
    }

inline CoeStr& CoeStr::operator= (const CoeStr& rhs)
    {
        if (this != &rhs) {
            using std::string;
            if (! _cs)
                _xs().~string();
            if (! (_cs = rhs._cs))
                new (_xb.xmem) std::string(rhs._xs());
        }
        return *this;
    }

inline CoeStr& CoeStr::operator= (const std::string& rhs)
    {
        using std::string;
        if (! _cs)
            _xs().~string();
        _cs = 0;
        new (_xb.xmem) std::string(rhs);
        return *this;
    }

inline bool CoeStr::empty () const
    {
        return _cs ? *_cs == '\0' : _xs().empty();
    }

inline const char* CoeStr::c_str () const
    {
        return _cs ? _cs : _xs().c_str();
    }

inline void CoeStr::clear ()
    {
        using std::string;
        if (! _cs)
            _xs().~string();
        _cs = _Empty;
    }

//////////////////////////////////////////////////////////////////////////////
// ===========================================================================

#else   // COEXX_COESTR

typedef std::string CoeStr;

#endif

// ===========================================================================

} ///// namespace coe

#endif

