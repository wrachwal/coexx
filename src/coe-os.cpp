// coe-os.cpp

#include "coe-os.h"
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
// SynchGuard

SynchGuard::~SynchGuard ()
{
}

// -----------------------------------------------------------------------
// _SpyGuard

struct _SpyGuard {
    static void unlock (SynchGuard& guard) { guard._unlock(); }
};

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

Mutex::Guard::Guard (Mutex& mutex, SynchGuard& parent) : _mutex(mutex)
{
    _locked = true;
    _mutex._lock();
    // lock chaining:
    //   once current `_mutex' is locked, unlock the `parent'
    _SpyGuard::unlock(parent);
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

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

int main ()
{
    // lock hierarchy
    {
        Mutex   mutex_A;
        Mutex   mutex_B;
        Mutex   mutex_C;
        {
            Mutex::Guard    guard_A(mutex_A);
            Mutex::Guard    guard_B(mutex_B);
            Mutex::Guard    guard_C(mutex_C);
        }
    }

    // lock chaining
    {
        Mutex   mutex_A;
        Mutex   mutex_B;
        Mutex   mutex_C;
        {
            Mutex::Guard    guard_A(mutex_A);
            Mutex::Guard    guard_B(mutex_B, guard_A);
            Mutex::Guard    guard_C(mutex_C, guard_B);
        }
    }
}

