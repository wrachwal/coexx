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

// -----------------------------------------------------------------------
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

// =======================================================================
// Mutex

class Mutex : private _Noncopyable {
public:
    Mutex ();
    ~Mutex ();

    static Mutex& from_sys (pthread_mutex_t& mutex);

    class Guard : public _Guard {
    public:
        Guard (Mutex& mutex);
        Guard (Mutex& mutex, _Guard& parent);   // lock chaining
        ~Guard ();
    //FIXME: private:
        virtual void _unlock ();
        Mutex&  _mutex;
    };

//FIXME: private:
    pthread_mutex_t _mutex;
    void   _lock ();
    void _unlock ();
};

// =======================================================================
// RWLock

class RWLock : private _Noncopyable {
public:
    RWLock ();
    ~RWLock ();

    static RWLock& from_sys (pthread_rwlock_t& rwlock);

    enum Type { READ, WRITE };

    class Guard : public _Guard {
    public:
        Guard (RWLock& rwlock, Type type);
        Guard (RWLock& rwlock, Type type, _Guard& parent);  // lock chaining
        ~Guard ();
    private:
        virtual void _unlock ();
        RWLock& _rwlock;
    };

private:
    pthread_rwlock_t    _rwlock;
    void   _lock (Type type);
    void _unlock ();
};

// =======================================================================
// CondVar

class CondVar : private _Noncopyable {
public:
    CondVar ();
    ~CondVar ();

    static CondVar& from_sys (pthread_cond_t& cond);

    void wait      (Mutex::Guard& guard);
    bool timedwait (Mutex::Guard& guard, const timespec& abstime);

    void signal    ();
    void broadcast ();

private:
    pthread_cond_t  _cond;
};

// =======================================================================

#endif

