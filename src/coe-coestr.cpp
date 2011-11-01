// coe-coestr.cpp

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

#include "coe-coestr.h"

#if COEXX_COESTR

#include "coe-sys-sync.h"

#include <ostream>

using namespace std;
using namespace coe;

const char* CoeStr::_Empty = "";

#if ( COEXX_COESTR >= 2 )

#include <iostream>
#include <stdexcept>    // domain_error
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>    // sort

#include <cassert>

// ---------------------------------------------------------------------------

namespace {
    typedef unsigned long long  Addr;
    typedef pair<Addr, Addr>    Region;     // [first, second)

    static struct Maps {
        Maps () : inited(false) {}
        volatile bool       inited;
        Mutex               mutex;
        vector<Region>      ro_seg;         // read-only segments
    } maps;
}

static void _read_proc_maps ()
{
    maps.ro_seg.clear();

    const char* proc = "/proc/self/maps";
    ifstream    ifs(proc);
    if (ifs.fail()) {
        cerr << "!!! [" << proc << "] could not open! :(" << endl;
        return;
    }

    string  line;
    while (getline(ifs, line)) {
        istringstream   iss(line);
        string  range, perm;
        Addr    beg, end;
        char    dash;
        bool    match = false;
        if (iss >> range >> perm) {
            istringstream   irs(range);
            if (irs >> hex >> beg >> dash >> end && dash == '-') {
                assert(beg <= end);
                match = true;
            }
        }
        if (! match) {
            cerr << "!!! [" << proc << "] not parsed ==> " << line << endl;
            continue;
        }
        if (0 == perm.compare(0, 2, "r-")) {
            maps.ro_seg.push_back(make_pair(beg, end));
        }
    }

    sort(maps.ro_seg.begin(), maps.ro_seg.end());
}

static void init_proc_maps ()
{
    Mutex::Guard    lock(maps.mutex);
    if (! maps.inited) {
        _read_proc_maps();
        maps.inited = true;
    }
}

static inline bool addr_in_range (Addr addr, const Region& range)
{
    return addr >= range.first && addr < range.second;
}

static bool readonly_segment_addr (const void *ptr)
{
    if (maps.ro_seg.empty())
        return false;
    const Addr  addr = reinterpret_cast<Addr>(ptr);
    const Region*   a = &maps.ro_seg[0];
    const Region*   b = &maps.ro_seg[maps.ro_seg.size() - 1];
    while (b - a > 1) {
        const Region*   m = a + (b - a) / 2;
        if      (addr <  m-> first) b = m;
        else if (addr >= m->second) a = m;
        else    return true;
    }
    return addr_in_range(addr, *a) || (a != b ? addr_in_range(addr, *b) : false);
}

// ===========================================================================
// CoeStr

CoeStr::CoeStr (const char* s)
{
    if (! maps.inited) {
        init_proc_maps();
    }

    if (s) {
        if (readonly_segment_addr(s)) {
            _cs = s;
        }
        else {
#if ( COEXX_COESTR >= 3 )
            _cs = NULL;
            new (_xb.xmem) std::string(s);
#else
            _cs = _Empty;
            assert(          ! "CoeStr(char*-but-non-literal) detected");
            throw domain_error("CoeStr(char*-but-non-literal) detected");
#endif
        }
    }
    else {
        _cs = _Empty;
    }
}

CoeStr& CoeStr::operator= (const char* rhs)
{
    if (! maps.inited) {
        init_proc_maps();
    }

    using std::string;
    if (! _cs)
        _xs().~string();

    if (rhs) {
        if (readonly_segment_addr(rhs)) {
            _cs = rhs;
        }
        else {
#if ( COEXX_COESTR >= 3 )
            _cs = NULL;
            new (_xb.xmem) std::string(rhs);
#else
            _cs = _Empty;
            assert(          ! "=(char*-but-non-literal) detected");
            throw domain_error("=(char*-but-non-literal) detected");
#endif
        }
    }
    else {
        _cs = _Empty;
    }

    return *this;
}

#endif  // ( COEXX_COESTR >= 2 )

// ---------------------------------------------------------------------------

int CoeStr::compare (const CoeStr& rhs) const
{
    if (_cs && rhs._cs)
        return std::strcmp(_cs, rhs._cs);
    else if (_cs)
        return - rhs._xs().compare(_cs);
    else if (rhs._cs)
        return _xs().compare(rhs._cs);
    else
        return _xs().compare(rhs._xs());
}

int CoeStr::compare (const string& rhs) const
{
    if (_cs)
        return - rhs.compare(_cs);
    else
        return _xs().compare(rhs);
}

int CoeStr::compare (const char* rhs) const
{
    if (_cs)
        return std::strcmp(_cs, rhs);
    else
        return _xs().compare(rhs);
}

// ===========================================================================

ostream& coe::operator<< (ostream& os, const CoeStr& str)
{
    if (str._cs)
        return std::operator<< (os, str._cs);
    else
        return std::operator<< (os, str._xs());
}

// ===========================================================================

#endif  // COEXX_COESTR

