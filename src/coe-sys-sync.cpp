// coe-sys-sync.cpp

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

#include "coe-sys-sync.h"

#include <cerrno>
#include <cstring>      // strerror
#include <cstdlib>      // abort
#include <iostream>

using namespace std;
using namespace coe;

// ---------------------------------------------------------------------------

#define raise_error(code, text)                                             \
    do {                                                                    \
        cerr << text << " at \"" << __FILE__ << "\":" << __LINE__ << ": "   \
             << strerror(code) << " (" << code << ')' << endl;              \
        abort();                                                            \
    } while(0)

// ===========================================================================
// _Guard

_Guard::~_Guard ()
{
}

// ---------------------------------------------------------------------------
// _GuardSpy

struct coe::_GuardSpy {
    static void unlock (_Guard& guard);
    static Sys_Mutex& sys_mutex (Mutex::Guard& guard);
};

// ------------------------------------

void _GuardSpy::unlock (_Guard& guard)
{
    guard._unlock();
}

Sys_Mutex& _GuardSpy::sys_mutex (Mutex::Guard& guard)
{
    return guard._mutex._mutex;
}

// ===========================================================================
// Mutex

Mutex::Mutex ()
{
    int status = pthread_mutex_init(&_mutex, NULL);
    if (status != 0) {
        raise_error(status, "pthread_mutex_init");
    }
}

Mutex::~Mutex ()
{
    int status = pthread_mutex_destroy(&_mutex);
    if (status != 0) {
        raise_error(status, "pthread_mutex_destroy");
    }
}

Mutex& Mutex::from_sys (Sys_Mutex& mutex)
{
    return reinterpret_cast<Mutex&>(mutex);
}

void Mutex::_lock ()
{
    int status = pthread_mutex_lock(&_mutex);
    if (status != 0) {
        raise_error(status, "pthread_mutex_lock");
    }
}

void Mutex::_unlock ()
{
    int status = pthread_mutex_unlock(&_mutex);
    if (status != 0) {
        raise_error(status, "pthread_mutex_unlock");
    }
}

// ---------------------------------------------------------------------------
// Mutex::Guard

Mutex::Guard::Guard (Mutex& mutex) : _mutex(mutex)
{
    _locked = true;
    _mutex._lock();
}

Mutex::Guard::Guard (Mutex& mutex, _Guard& parent) : _mutex(mutex)
{
    _locked = true;
    _mutex._lock();
    // lock chaining:
    //   once current `_mutex' is locked, unlock the `parent'
    _GuardSpy::unlock(parent);
}

Mutex::Guard::~Guard ()
{
    _unlock();  //FIXME: pure virtual, or scoped?
}

void Mutex::Guard::unlock (Guard& guard)
{
    guard._unlock();
}

void Mutex::Guard::_unlock ()
{
    if (_locked) {
        _locked = false;
        _mutex._unlock();
    }
}

// ===========================================================================
// RWLock

RWLock::RWLock ()
{
    int status = pthread_rwlock_init(&_rwlock, NULL);
    if (status != 0) {
        raise_error(status, "pthread_rwlock_init");
    }
}

RWLock::~RWLock ()
{
    int status = pthread_rwlock_destroy(&_rwlock);
    if (status != 0) {
        raise_error(status, "pthread_rwlock_destroy");
    }
}

RWLock& RWLock::from_sys (Sys_RWLock& rwlock)
{
    return reinterpret_cast<RWLock&>(rwlock);
}

void RWLock::_lock (Type type)
{
    if (READ == type) {
        int status = pthread_rwlock_rdlock(&_rwlock);
        if (status != 0) {
            raise_error(status, "pthread_rwlock_rdlock");
        }
    }
    else {
        int status = pthread_rwlock_wrlock(&_rwlock);
        if (status != 0) {
            raise_error(status, "pthread_rwlock_wrlock");
        }
    }
}

void RWLock::_unlock ()
{
    int status = pthread_rwlock_unlock(&_rwlock);
    if (status != 0) {
        raise_error(status, "pthread_rwlock_unlock");
    }
}

// ---------------------------------------------------------------------------
// RWLock::Guard

RWLock::Guard::Guard (RWLock& rwlock, Type type) : _rwlock(rwlock)
{
    _locked = true;
    _rwlock._lock(type);
}

RWLock::Guard::Guard (RWLock& rwlock, Type type, _Guard& parent) : _rwlock(rwlock)
{
    _locked = true;
    _rwlock._lock(type);
    // lock chaining:
    //   once current `_rwlock' is locked, unlock the `parent'
    _GuardSpy::unlock(parent);
}

RWLock::Guard::~Guard ()
{
    _unlock();  //FIXME: pure virtual, or scoped?
}

void RWLock::Guard::unlock (Guard& guard)
{
    guard._unlock();
}

void RWLock::Guard::_unlock ()
{
    if (_locked) {
        _locked = false;
        _rwlock._unlock();
    }
}

// ===========================================================================
// CondVar

CondVar::CondVar ()
{
    int status = pthread_cond_init(&_cond, NULL);
    if (status != 0) {
        raise_error(status, "pthread_cond_init");
    }
}

CondVar::~CondVar ()
{
    int status = pthread_cond_init(&_cond, NULL);
    if (status != 0) {
        raise_error(status, "pthread_cond_init");
    }
}

CondVar& CondVar::from_sys (Sys_CondVar& cond)
{
    return reinterpret_cast<CondVar&>(cond);
}

void CondVar::wait (Mutex::Guard& guard)
{
    int status = pthread_cond_wait(&_cond, &_GuardSpy::sys_mutex(guard));
    if (status != 0) {
        raise_error(status, "pthread_cond_wait");
    }
}

bool CondVar::timedwait (Mutex::Guard& guard, const timespec& abstime)
{
    int status = pthread_cond_timedwait(&_cond, &_GuardSpy::sys_mutex(guard), &abstime);
    if (status == 0) {
        return true;
    }
    else
    if (status == ETIMEDOUT) {
        return false;
    }
    else {
        raise_error(status, "pthread_cond_timedwait");
    }
}

void CondVar::signal ()
{
    int status = pthread_cond_signal(&_cond);
    if (status != 0) {
        raise_error(status, "pthread_cond_signal");
    }
}

void CondVar::broadcast ()
{
    int status = pthread_cond_broadcast(&_cond);
    if (status != 0) {
        raise_error(status, "pthread_cond_broadcast");
    }
}

