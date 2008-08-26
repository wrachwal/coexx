// coe-os.h

#ifndef __COE_OS_H
#define __COE_OS_H

#include <pthread.h>

// -----------------------------------------------------------------------
// _Noncopyable

class _Noncopyable {
public:
    _Noncopyable () {}
private:
    _Noncopyable (const _Noncopyable&);
    _Noncopyable& operator= (const _Noncopyable&);
};

// =======================================================================
// Guard

struct _SpyGuard;

class SynchGuard : private _Noncopyable {
public:
    virtual ~SynchGuard ();
protected:
    friend struct _SpyGuard;
    virtual void _unlock () = 0;
    bool         _locked;
};

// =======================================================================
// Mutex

class Mutex : private _Noncopyable {
public:
    Mutex ();
    ~Mutex ();

    class Guard : public SynchGuard {
    public:
        Guard (Mutex& mutex);
        Guard (Mutex& mutex, SynchGuard& parent);   // lock chaining
        ~Guard ();
    private:
        virtual void _unlock ();
        Mutex&  _mutex;
    };

private:
    pthread_mutex_t _mutex;
    void   _lock ();
    void _unlock ();
};

// =======================================================================

#endif

