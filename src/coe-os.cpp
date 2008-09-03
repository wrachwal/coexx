// coe-os.cpp

#include "coe-os.h"
#include <errno.h>
#include <iostream>

using namespace std;

// -----------------------------------------------------------------------

#define raise_error(code, text)                                             \
    do {                                                                    \
        cerr << text << " at \"" << __FILE__ << "\":" << __LINE__ << ": "   \
             << strerror(code) << " (" << code << ')' << endl;              \
        abort();                                                            \
    } while(0)

// =======================================================================
// _Guard

_Guard::~_Guard ()
{
}

// -----------------------------------------------------------------------
// _SpyGuard

struct _GuardSpy {
    static void unlock (_Guard& guard);
};

void _GuardSpy::unlock (_Guard& guard)
{
    guard._unlock();
}

// =======================================================================
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

Mutex& Mutex::from_sys (pthread_mutex_t& mutex)
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

// -----------------------------------------------------------------------
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

void Mutex::Guard::_unlock ()
{
    if (_locked) {
        _locked = false;
        _mutex._unlock();
    }
}

// =======================================================================
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

RWLock& RWLock::from_sys (pthread_rwlock_t& rwlock)
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

// -----------------------------------------------------------------------
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

void RWLock::Guard::_unlock ()
{
    if (_locked) {
        _locked = false;
        _rwlock._unlock();
    }
}

// =======================================================================
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

CondVar& CondVar::from_sys (pthread_cond_t& cond)
{
    return reinterpret_cast<CondVar&>(cond);
}

void CondVar::wait (Mutex::Guard& guard)
{
    int status = pthread_cond_wait(&_cond, &guard._mutex._mutex);
    if (status != 0) {
        raise_error(status, "pthread_cond_wait");
    }
}

bool CondVar::timedwait (Mutex::Guard& guard, const timespec& abstime)
{
    int status = pthread_cond_timedwait(&_cond, &guard._mutex._mutex, &abstime);
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

