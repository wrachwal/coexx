// coe-sys-sync.h

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

#ifndef __COE_SYS_SYNC_H
#define __COE_SYS_SYNC_H

#include <pthread.h>
#include "coe--local.h"

namespace coe { /////

// ---------------------------------------------------------------------------

typedef pthread_mutex_t  Sys_Mutex;
typedef pthread_rwlock_t Sys_RWLock;
typedef pthread_cond_t   Sys_CondVar;

// ---------------------------------------------------------------------------
// _Guard

struct _GuardSpy;

class _Guard : private _Noncopyable {
public:
    virtual ~_Guard ();
protected:
    friend struct _GuardSpy;
    virtual void _unlock () = 0;
    bool         _locked;
};

// ===========================================================================
// Mutex

class Mutex : private _Noncopyable {
public:
    Mutex ();
    ~Mutex ();

    static Mutex& from_sys (Sys_Mutex& mutex);

    class Guard : public _Guard {
    public:
        Guard (Mutex& mutex);
        Guard (Mutex& mutex, _Guard& parent);   // lock chaining
        ~Guard ();

        static void unlock (Guard& guard);  // explicit unlock

    private:
        friend struct _GuardSpy;
        virtual void _unlock ();
        Mutex&  _mutex;
    };

private:
    friend struct _GuardSpy;
    Sys_Mutex   _mutex;
    void   _lock ();
    void _unlock ();
};

// ===========================================================================
// RWLock

class RWLock : private _Noncopyable {
public:
    RWLock ();
    ~RWLock ();

    static RWLock& from_sys (Sys_RWLock& rwlock);

    enum Type { READ, WRITE };

    class Guard : public _Guard {
    public:
        Guard (RWLock& rwlock, Type type);
        Guard (RWLock& rwlock, Type type, _Guard& parent);  // lock chaining
        ~Guard ();

        static void unlock (Guard& guard);  // explicit unlock

    private:
        virtual void _unlock ();
        RWLock& _rwlock;
    };

private:
    Sys_RWLock  _rwlock;
    void   _lock (Type type);
    void _unlock ();
};

// ===========================================================================
// CondVar

class CondVar : private _Noncopyable {
public:
    CondVar ();
    ~CondVar ();

    static CondVar& from_sys (Sys_CondVar& cond);

    void wait      (Mutex::Guard& guard);
    bool timedwait (Mutex::Guard& guard, const timespec& abstime);

    void signal    ();
    void broadcast ();

private:
    Sys_CondVar _cond;
};

// ===========================================================================

} ///// namespace coe

#endif

