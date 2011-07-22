// coe-meta.h

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

#ifndef __COE_META_H
#define __COE_MEAT_H

#include "coe--local.h"

#include <cstdlib>      // size_t
#include <typeinfo>
#include <cassert>

namespace coe { /////

// ===========================================================================
// TypeInfo

class TypeInfo {
public:
    TypeInfo () : _ti(0) {}
    TypeInfo (const std::type_info& ti) : _ti(&ti) {}
    const char* name () const { return _ti ? _ti->name() : 0; }
private:
    const std::type_info*   _ti;
};

// ===========================================================================
// init_meta_info<Type, Info>

template<class Type, class Info> struct init_meta_info;

// ===========================================================================
// Meta<Info>

template<class, class> class Ctti;

template<class Info>
struct Meta : private _Noncopyable {
    const Meta* next;
    size_t      indx;
    Info        info;
    static const Meta* registry () { return head; }
private:
    template<class, class> friend class Ctti;
    template<class Type>
    Meta (Type*) : next(head), indx(next ? next->indx + 1 : 1)
        {
            init_meta_info<Type, Info>()(info);
            head = this;
        }
    static Meta*    head;
};

// ------------------------------------

template<class Info>
Meta<Info>* Meta<Info>::head = 0;

// ===========================================================================
// Ctti<Type, Info>

template<class Type, class Info>
class Ctti {
public:
    static const Meta<Info>* meta () { return &_meta; }
private:
    static const Meta<Info> _meta;
};

// ------------------------------------

template<class Type, class Info>
const Meta<Info> Ctti<Type, Info>::_meta((Type*)0);

// ===========================================================================

} ///// namespace coe

#endif
