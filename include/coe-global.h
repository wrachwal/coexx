// coe-global.h

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

#ifndef __COE_GLOBAL_H
#define __COE_GLOBAL_H

namespace coe { /////

// ===========================================================================
// Factory<T>

template<class T>
struct Factory {
    static T* create ();
    static void destroy (T* object);
};

// ------------------------------------

template<class T>
T* Factory<T>::create ()
    {
        return new T;
    }

template<class T>
void Factory<T>::destroy (T* object)
    {
        delete object;
    }

// ===========================================================================
// LocalStorageInfo

struct LocalStorageInfo {
    const std::type_info*   typeinfo;
    void*                 (*create)();
    void                  (*destroy)(void*);
    size_t                  index;
};

// ===========================================================================

} ///// namespace coe

#endif

