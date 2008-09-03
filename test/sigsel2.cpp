// sigsel2.cpp -- posix timers (SIGEV_THREAD), signals, and select

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ***** IMPORTANT NOTE *****
//   On Solaris I am using
//      $ uname -a
//      SunOS XXX 5.9 Generic_118558-35 sun4u sparc SUNW,Sun-Fire-480R Solaris
//  I get the following runtime error:
//      SIGRTMIN = 39
//      SIGRTMAX = 46
//      timer_create: Invalid argument
//      zsh: IOT instruction (core dumped)  ./sigsel2
// Isn't SIGEV_THREAD supported ??!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include <iostream>
#include <cerrno>
#include <csignal>
#include <cassert>

#include <pthread.h>
#include <sys/select.h> // select()

using namespace std;

#define TABLEN(tab)     int(sizeof(tab) / sizeof((tab)[0]))

#if defined(__CYGWIN__)
// Cygwin seems not to have RT signals :( or not many (SIGRTMIN==SIGRTMAX).
const int KickSigNo = SIGRTMIN;
#else
const int KickSigNo = SIGRTMIN;
#endif

static const clockid_t   g_ClockID = CLOCK_REALTIME;
static timer_t           g_TimerID;

// =======================================================================
// worker threads

struct Worker {
    pthread_t   tid;
    int         pipe_fd[2];
} g_Worker[10];

static void set_next_timeout (int secs)
{
    timespec_t  niltime = { 0, 0 };
    timespec_t  nowtime;

    if (clock_gettime(::g_ClockID, &nowtime) != 0) {
        perror("clock_gettime");
        abort();
    }

    nowtime.tv_sec += secs;

    itimerspec  settime;
    settime.it_interval = niltime;  // zero
    settime.it_value    = nowtime;

    if (timer_settime(::g_TimerID, TIMER_ABSTIME, &settime, NULL) != 0) {
        perror("timer_settime");
        abort();
    }
}

static void kick_sig_handler (int sigNo)
{
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
    sigaddset(&sigSet, ::KickSigNo);
    sigAction.sa_handler = &::kick_sig_handler;
    sigAction.sa_mask    = sigSet;
    sigAction.sa_flags   = 0;

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

static void notify_workers ()
{
    static int odd = 0; //UGLY :)

    cout << "\n\n\n" << __FUNCTION__ << " received -- kicking worker "
         << (odd ? "ODD" : "EVEN") << " threads...\n" << endl;

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

static void timer_thread_callback (union sigval sv)
{
    cout << __FUNCTION__ << " (SIGEV_THREAD) called with .sival_ptr "
         << sv.sival_ptr << endl;
    notify_workers();
}

// =======================================================================

int main ()
{
    cout << "SIGRTMIN = " << SIGRTMIN << endl;
    cout << "SIGRTMAX = " << SIGRTMAX << endl;

    // --------------------------------
    // First block Kick signal
    {
        sigset_t    sigSet;
        sigemptyset(&sigSet);
        sigaddset(&sigSet, ::KickSigNo);
        pthread_sigmask(SIG_BLOCK, &sigSet, NULL);
    }

    // --------------------------------
    // create POSIX timer
    //
    {
        sigevent    signalSpec;

        signalSpec.sigev_notify            = SIGEV_THREAD;
        signalSpec.sigev_value.sival_ptr   = &::g_TimerID;
        signalSpec.sigev_notify_function   = &::timer_thread_callback;
        signalSpec.sigev_notify_attributes = NULL;  // will be detached

        if (timer_create(::g_ClockID, &signalSpec, &::g_TimerID) != 0) {
            perror("timer_create");
            abort();
        }

        cout << "### .sigev_value.sival_ptr --> & ::g_TimerID --> "
             << & ::g_TimerID << endl;
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
#if 0
        int result = select(0, NULL, NULL, NULL, NULL);
        if (result == -1) {
            cout << "main() -- select --> EINTR (" << ++x << ")" << endl;
        }
        else {
            cerr << "main() -- unexpected select" << endl;
            abort();
        }
#else
            sleep(1);
            cout << "main() -- sleep(1) [" << ++x << "]" << endl;
#endif
    }
}

