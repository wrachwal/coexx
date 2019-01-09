// coe-thread--imp.h

/*****************************************************************************
Copyright (c) 2008-2019 Waldemar Rachwał <waldemar.rachwal@gmail.com>

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

#if !defined(__COE_THREAD_H) || defined(__COE_THREAD__IMP_H)
#error "never include this header directly!"
#endif

#ifndef __COE_THREAD__IMP_H
#define __COE_THREAD__IMP_H

namespace coe { /////

// ===========================================================================
// Thread

template<class> class _TlsI;

// ------------------------------------
// _TlsD

struct _TlsD {

    _TlsD*              next;
    LocalStorageInfo    info;

    static const _TlsD* registry () { return _register(0); }

private:
    template<class> friend class _TlsI;
    template<class T>
    _TlsD (T* (*_create)(), void (*_destroy)(T*))
        :   next(0)
        {
            info.typeinfo = &typeid(T);
            info.create   = reinterpret_cast<void*(*)()>(_create);
            info.destroy  = reinterpret_cast<void(*)(void*)>(_destroy);
            _register(this);
        }
    static const _TlsD* _register (_TlsD* data);
};

// ------------------------------------
// _TlsI<T>

template<class T>
class _TlsI {
public:
    const _TlsD* data () const;
private:
    const static _TlsD  _data;
};

template<class T>
const _TlsD _TlsI<T>::_data(&Factory<T>::create, &Factory<T>::destroy);

template<class T>
const _TlsD* _TlsI<T>::data () const
    {
        return &_data;
    }

// ------------------------------------
// T& <-- tls<T>()

template<class T>
T& Thread::tls ()
    {
        return *static_cast<T*>(_get_user_tls(_TlsI<T>().data()));
    }

// ===========================================================================

} ///// namespace coe

#endif

