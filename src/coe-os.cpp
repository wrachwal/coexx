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
    if (RLock == type) {
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

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static pthread_mutex_t  mutex_0  =  PTHREAD_MUTEX_INITIALIZER;
static pthread_rwlock_t rwlock_0 = PTHREAD_RWLOCK_INITIALIZER;

static void check_guards_usage ()
{
    // lock hierarchy
    {
        Mutex   mutex_A;
        RWLock  rwlock_B;
        Mutex   mutex_C;
        {
            Mutex::Guard    guard_0(Mutex::from_sys(mutex_0));
            Mutex::Guard    guard_A(mutex_A);
            RWLock::Guard   guard_B(rwlock_B, RWLock::RLock);
            Mutex::Guard    guard_C(mutex_C);
        }
    }

    // lock chaining
    {
        Mutex   mutex_A;
        Mutex   mutex_B;
        Mutex   mutex_C;
        {
            RWLock::Guard   guard_0(RWLock::from_sys(rwlock_0), RWLock::WLock);
            Mutex::Guard    guard_A(mutex_A, guard_0);
            Mutex::Guard    guard_B(mutex_B, guard_A);
            Mutex::Guard    guard_C(mutex_C, guard_B);
        }
    }
}

// -----------------------------------------------------------------------
// Programming with POSIX Threads, by David R.Butenhof
// [cond.c] sec-3.3.2, p.77

static struct my_struct_t {
    Mutex   mutex;  // protects access to value
    CondVar cond;   // signals change to value
    int     value;  // access protected by mutex
    // -----------------
    my_struct_t () : value(0) {}
} g_data;

static int hibernation = 1;     // default to 1 second

/*
 * Thread start routine. It will set the main thread's predicate
 * and signal the conditional variable.
 */
static void*
wait_thread (void* arg)
{
    sleep(hibernation);

    Mutex::Guard    guard(g_data.mutex);

    g_data.value = 1;   // set predicate
    g_data.cond.signal();

    return NULL;
}

int main (int argc, char* argv[])
{
    check_guards_usage();

    /*
     * If an argument is specified, interpret it as the number
     * of seconds for wait_thread to sleep before signaling the
     * conditional variable. You can play with this to see the
     * condition wait below time out or wake normally.
     */
    if (argc > 1)
        hibernation = atoi(argv[1]);

    /*
     * Create wait_thread.
     */
    pthread_t   wait_thread_id;
    int status = pthread_create(&wait_thread_id, NULL, wait_thread, NULL);
    if (status != 0) {
        raise_error(status, "Create wait thread");
    }

    /*
     * Wait on the condition variable for 2 seconds, or until
     * signaled by the wait_thread. Normally, wait_thread
     * should signal. If you raise "hibernation" above 2
     * seconds, it will time out.
     */
    timespec    timeout;
    timeout.tv_sec  = time(NULL) + 2;
    timeout.tv_nsec = 0;

    Mutex::Guard    guard(g_data.mutex);

    while (g_data.value == 0) {

        if (g_data.cond.timedwait(guard, timeout)) {

            if (g_data.value != 0) {    // check predicate
                cout << "Condition was signaled." << endl;
            }
        }
        else {
            cout << "Condition wait timed out." << endl;
            // Without the following break statement when hibernation is 3 or longer,
            // the execution stays in the while loop forever.
            // Apparently, second and next timedwait(s) monopolize CPU completely,
            // and wait_thread has no chance to signal the condition.
            break;
        }
    }

    return 0;
}

