// sigsel1.cpp -- posix timers (SIGEV_SIGNAL), signals, and select

#include <iostream>
#include <cerrno>
#include <csignal>
#include <cassert>

#include <pthread.h>
#include <sys/select.h> // select()
#include <unistd.h>     // read() on Linux

using namespace std;

#define TABLEN(tab)     int(sizeof(tab) / sizeof((tab)[0]))

#if defined(__CYGWIN__)
// Cygwin seems not to have RT signals :( or not many (SIGRTMIN==SIGRTMAX).
const int TimeSigNo = SIGUSR1;
const int KickSigNo = SIGUSR2;
#else
const int TimeSigNo = SIGRTMIN;
const int KickSigNo = SIGRTMIN + 1;
#endif

static const clockid_t   g_ClockID = CLOCK_REALTIME;
static timer_t           g_TimerID;
static pthread_t         timer_tid;

// =======================================================================
// worker threads

struct Worker {
    pthread_t   tid;
    int         pipe_fd[2];
} g_Worker[10];

static void set_next_timeout (int secs)
{
    struct timespec niltime = { 0, 0 };
    struct timespec nowtime;

    if (clock_gettime(::g_ClockID, &nowtime) != 0) {
        perror("clock_gettime");
        abort();
    }

    nowtime.tv_sec += secs;

    struct itimerspec   settime;
    settime.it_interval = niltime;  // zero
    settime.it_value    = nowtime;

    if (timer_settime(::g_TimerID, TIMER_ABSTIME, &settime, NULL) != 0) {
        perror("timer_settime");
        abort();
    }
}

static void kick_sig_handler (int sigNo, siginfo_t* info, void* context)
{
    //
    // what i am doing here is probably not best in signal catching function
    // (pthread_... functions called). but don't worry--i won't need any of
    // this in coe anyway.
    //
    pthread_t   tid = pthread_self();
    for (int tn = 0; tn < TABLEN(g_Worker); ++tn) {
        if (pthread_equal(tid, g_Worker[tn].tid)) {
            cout << "thread #: " << tn << " is handling " << sigNo << " signal" << endl;
            return;
        }
    }
    cerr << "kick_sig_handler: unknown thread!" << endl;
    abort();
}

static void* worker_thread_entry (void* arg)
{
    Worker* pt = (Worker*)arg;
    int     tn = pt - g_Worker; // thread number (0-based)

    //
    // unblock kicking signal
    //
    sigset_t    sigSet;
    sigemptyset(&sigSet);
    sigaddset(&sigSet, ::KickSigNo);
    int status = pthread_sigmask(SIG_UNBLOCK, &sigSet, NULL);
    if (status != 0) {
        cerr << "pthread_sigmask: " << strerror(status) << endl;
        abort();
    }

    //
    // install signal action
    //
    struct sigaction    sigAction;
    sigemptyset(&sigSet);
    sigAction.sa_sigaction = &::kick_sig_handler;
    sigAction.sa_mask      =  sigSet;
    sigAction.sa_flags     =  SA_SIGINFO;

    if (sigaction(::KickSigNo, &sigAction, NULL) != 0) {
        perror("sigaction");
        abort();
    }

    //
    // select() without `timeout'
    //
    while (1) {

        fd_set  rset;
        FD_ZERO(&rset);
        FD_SET(pt->pipe_fd[0], &rset);

        int bits = select(pt->pipe_fd[0] + 1, &rset, NULL, NULL, NULL);

        if (bits == -1) {       // interrupted by a signal
            if (errno == EINTR) {

                cout << "thread #" << tn << ": select interrupted by a signal" << endl;

                int secs = (0 == tn % 2) ? 6 + tn : 2 + tn;
                cout << "thread #" << tn << " set next timeout by "
                     << secs << " seconds." << endl;

                set_next_timeout(secs);
            }
            else {
                perror("select");
                abort();
            }
        }
        else
        if (bits == 0) {
            cerr << "select: unexpected timeout returned!" << endl;
            abort();
        }
        else {                  // input data
            if (FD_ISSET(pt->pipe_fd[0], &rset)) {
                char    buf[128];
                int nbytes = read(pt->pipe_fd[0], buf, sizeof(buf) - 1);
                if (nbytes > 0) {
                    buf[nbytes] = 0;
                    cout << buf << endl;
                }
            }
        }
    }
}

// =======================================================================
// timer thread

static void notify_workers (int sigNo)
{
    static int odd = 0; //UGLY :)

    cout << "\n\n\ntime_sig_handler: sigNo=" << sigNo << " received -- kicking worker "
         << (odd ? "ODD" : "EVEN") << " threads..." << endl;

    for (int i = 0; i < TABLEN(g_Worker); ++i) {

        if (i % 2 == odd) {

            Worker* pt = &g_Worker[i];

            int status = pthread_kill(pt->tid, ::KickSigNo);
            if (status != 0) {
                cerr << "pthread_kill: " << strerror(status) << endl;
                abort();
            }
        }
    }

    odd ^= 1;
}

static void* timer_thread_entry (void* arg)
{
    sigset_t    sigSet;

    //
    // block ALL signals
    //
    sigfillset(&sigSet);
    pthread_sigmask(SIG_BLOCK, &sigSet, NULL);

    //
    // synchronously wait in loop for timer signal
    //
    // IMPORTANT NOTE:
    //      TimeSigNo has to be BLOCKED (done in previous step).
    //
    sigemptyset(&sigSet);
    sigaddset(&sigSet, ::TimeSigNo);

    while (1) {

        //
        //CAUTION:
        //  sigwait() may consume all pending signals
        //  of a given type if there are more in the queue
        //  at the time when sigwait() is called.
        //  This behavior is implementation-dependent.
        //see also:
        //  http://opengroup.org/onlinepubs/009695399/functions/sigwait.html
        //

        int actSig;
        int result = sigwait(&sigSet, &actSig);
        if (result == 0) {
            cout << __FUNCTION__ << ": sigwait: signal -> " << actSig << endl;
            notify_workers(actSig);
        }
        else {
            cout << __FUNCTION__ << ": sigwait: " << strerror(errno) << endl;
        }
    }
}

// =======================================================================

int main ()
{
    cout << "SIGRTMIN = " << SIGRTMIN << endl;
    cout << "SIGRTMAX = " << SIGRTMAX << endl;

    // --------------------------------
    // First block Time+Kick signals
    {
        sigset_t    sigSet;
        sigemptyset(&sigSet);
        sigaddset(&sigSet, ::TimeSigNo);
        sigaddset(&sigSet, ::KickSigNo);
        pthread_sigmask(SIG_BLOCK, &sigSet, NULL);
    }

    // --------------------------------
    // create POSIX timer
    //
    {
        sigevent    signalSpec;

        signalSpec.sigev_notify = SIGEV_SIGNAL;
        signalSpec.sigev_signo  = ::TimeSigNo;

        if (timer_create(::g_ClockID, &signalSpec, &g_TimerID) != 0) {
            perror("timer_create");
            abort();
        }
    }

    // --------------------------------
    // create timer thread
    //
    {
        int status = pthread_create(&timer_tid, NULL, &::timer_thread_entry, NULL);
        if (status != 0) {
            cerr << "pthread_create: " << strerror(status) << endl;
            abort();
        }
    }

    // --------------------------------
    // create worker threads (will be kicked by the timer thread)
    //
    for (int i = 0; i < TABLEN(g_Worker); ++i) {

        Worker* pt = &g_Worker[i];

        // create internal pipe
        if (pipe(pt->pipe_fd) != 0) {
            perror("pipe");
            abort();
        }

        // send intro message to thread
        char    intro[128];
        sprintf(intro, "--> You are thread #%d", i);
        write(pt->pipe_fd[1], intro, strlen(intro));

        // create thread
        int status = pthread_create(&pt->tid, NULL, &::worker_thread_entry, pt);
        if (status != 0) {
            cerr << "pthread_create: " << strerror(status) << endl;
            abort();
        }
    }

    set_next_timeout(1);    // initiate first timeout

    //
    // endless loop
    //
    int x = 0;

    while (1) {
        int result = select(0, NULL, NULL, NULL, NULL);
        if (result == -1) {
            cout << "main() -- select --> EINTR (" << ++x << ")" << endl;
        }
        else {
            cerr << "main() -- unexpected select" << endl;
            abort();
        }
    }
}

