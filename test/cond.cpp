// cond.cpp

#include "coe-sys.h"

#include <iostream>
#include <unistd.h> // sleep() on Linux

using namespace std;

// -----------------------------------------------------------------------

#define raise_error(code, text)                                             \
    do {                                                                    \
        cerr << text << " at \"" << __FILE__ << "\":" << __LINE__ << ": "   \
             << strerror(code) << " (" << code << ')' << endl;              \
        abort();                                                            \
    } while(0)


// -----------------------------------------------------------------------

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
            RWLock::Guard   guard_B(rwlock_B, RWLock::READ);
            Mutex::Guard    guard_C(mutex_C);
        }
    }

    // lock chaining
    {
        Mutex   mutex_A;
        Mutex   mutex_B;
        Mutex   mutex_C;
        {
            RWLock::Guard   guard_0(RWLock::from_sys(rwlock_0), RWLock::WRITE);
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

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

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

