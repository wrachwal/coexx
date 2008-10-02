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

#ifndef __COE_MISC_H
#define __COE_MISC_H

namespace coe { /////

// =======================================================================
// owned_ptr<T>

template<class T>
class owned_ptr {
public:
             owned_ptr ()       : _ptr(0)   {}
    explicit owned_ptr (T* ptr) : _ptr(ptr) {}

    owned_ptr (const owned_ptr& rhs)
        : _ptr(rhs.release()) {}
    owned_ptr& operator= (const owned_ptr& rhs)
        {
            if (this != &rhs) {
                delete _ptr;
                _ptr = rhs.release();
            }
            return *this;
        }

    T& operator*  () const { return *_ptr; }
    T* operator-> () const { return _ptr; }

    T*     get () const { return _ptr; }
    T* release () const { T* tmp = _ptr; _ptr = 0; return tmp; }

private:
    mutable T*  _ptr;
};

// -----------------------------------------------------------------------
// owned_array_ptr<T>

template<class T>
class owned_array_ptr {
public:
             owned_array_ptr ()       : _tab(0)   {}
    explicit owned_array_ptr (T* tab) : _tab(tab) {}

    owned_array_ptr (const owned_array_ptr& rhs)
        : _tab(rhs.release()) {}
    owned_array_ptr& operator= (const owned_array_ptr& rhs)
        {
            if (this != &rhs) {
                delete[] _tab;
                _tab = rhs.release();
            }
            return *this;
        }

    T& operator[] (int i) const { return _tab[i]; }

    T*     get () const { return _tab; }
    T* release () const { T* tmp = _tab; _tab = 0; return tmp; }

private:
    mutable T*  _tab;
};

// =======================================================================

} ///// namespace coe

#endif

