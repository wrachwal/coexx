// stats.cpp

#include "coe-session.h"
#include "coe-thread.h"     // run_event_loop()
#include "coe-stats.h"

#include <iostream>

#include <cstdlib>          // exit
#include <cstdio>           // perror

#include <unistd.h>         // write

using namespace std;
using namespace coe;

// ===========================================================================
// MySession

class MySession : public Session {
public:
    typedef MySession Self;

    static SiD spawn (Kernel& kernel, int rfd, Callback* reply)
        {
            return (new Self(reply))->start_session(kernel, vparam(rfd));
        }
private:
    MySession (Callback* reply) : Session(handler(*this, &Self::_start))
        ,   _reply(reply)
        {
        }
    ~MySession ()
        {
            delete _reply;
            _reply = NULL;
        }
    void _start (Kernel& kernel, int& rfd)
        {
            stop_handler(handler(*this, &Self::_stop));

            kernel.state("on_tick", handler(*this, &Self::on_tick));
            kernel.delay_set("on_tick", TimeSpec(0.3), vparam(1, false));
            kernel.delay_set("on_tick", TimeSpec(2.0), vparam(0, true));
            kernel.delay_set("on_tick", TimeSpec(1e2), vparam(0, false));

            kernel.state("on_post", handler(*this, &Self::on_post));

            kernel.state("on_read", handler(*this, &Self::on_read));
            kernel.select(rfd, IO_read, "on_read");
        }
    void _stop (Kernel& kernel)
        {
            _reply->anon_post();
        }
    void on_tick (Kernel& kernel, int& next, bool& stop)
        {
            cout << "on_tick" << (stop ? " (stop!)" : "") << endl;
            if (next)
                kernel.delay_set("on_tick", TimeSpec(next, 0), vparam(next, stop));
            if (stop)
                stop_session();
        }
    void on_post (Kernel& kernel)
        {
            cout << "on_post" << endl;
            stop_session();
        }
    void on_read (Kernel& kernel, IO_Ctx& io)
        {
            char    buf[256];
            ssize_t nr = read(io.fileno(), buf, sizeof(buf) - 1);
            if (nr > 0) {
                buf[nr] = 0;
                cout << "on_read ==> " << buf << endl;
                kernel.select_pause(io.fileno(), io.mode());
            }
            stop_session();
        }
private:    // data
    Callback*   _reply;
};

// ===========================================================================

static struct {
    int r_pipe;
    int w_pipe;
    int n_loop;
} gState;

// ---------------------------------------------------------------------------

#define STAT_(pfx, fld) cout << "    " << pfx << #fld << ": " << stat.curr.fld << "\n"

static void print_stats (Kernel& kernel, int line)
{
    cout << "\n### print_stats (state [" << kernel.context().state()
         << "], file " << __FILE__ << ", line " << line << ")" << "\n";
    cout << "* Kernel::Stats" << "\n";
    {
        Kernel::Stats   stat;
        kernel.get_stats(stat);
        STAT_("", sessions);
        STAT_("", handlers);
        STAT_("  ", alarms);
    }
    cout << "* Thread::Stats" << "\n";
    {
        Thread::Stats   stat;
        kernel.thread().get_stats(stat);
        STAT_("        ", kernels);
        STAT_("         ", alarms);
        STAT_("   ", watchers_all);
        STAT_("", watchers_active);
        STAT_("    ", lqueue_size);
        STAT_("    ", pqueue_size);
    }
    cout << flush;
}

// ---------------------------------------------------------------------------

static void on_alarmed (Kernel& kernel)
{
    print_stats(kernel, __LINE__);
    SiD sid = MySession::spawn(kernel, gState.r_pipe, kernel.callback("on_posted"));
    kernel.delay_set("posting", TimeSpec(1.5), vparam(sid));
}

static void posting (Kernel& kernel, SiD& sid)
{
    for (int i = 0; i < 3; ++i)
        kernel.post(sid, "on_post");
}

static void on_posted (Kernel& kernel)
{
    print_stats(kernel, __LINE__);
    MySession::spawn(kernel, gState.r_pipe, kernel.callback("on_written"));
    kernel.delay_set("writing", TimeSpec(1.5));
}

static void writing (Kernel& kernel)
{
    const char* msg = "stop, buddy!";
    (void) write(gState.w_pipe, msg, strlen(msg));  // blocking
}

static void on_written (Kernel& kernel)
{
    print_stats(kernel, __LINE__);
    if (++gState.n_loop < 3) {
        cout << "### " << string(75, '-') << endl;
        MySession::spawn(kernel, gState.r_pipe, kernel.callback("on_alarmed"));
    }
    else {
        cout << "### DONE." << endl;
        exit(EXIT_SUCCESS);
    }
}

// ***************************************************************************

int main ()
{
    Kernel& kernel = Kernel::create_new();

    int fds[2];
    if (pipe(fds) == -1) {
        perror("main: pipe");
        exit(EXIT_FAILURE);
    }
    gState.r_pipe = fds[0];
    gState.w_pipe = fds[1];

    kernel.state("on_alarmed", handler(&::on_alarmed));
    kernel.state("posting",    handler(&::posting));
    kernel.state("on_posted",  handler(&::on_posted));
    kernel.state("writing",    handler(&::writing));
    kernel.state("on_written", handler(&::on_written));

    print_stats(kernel, __LINE__);
    MySession::spawn(kernel, gState.r_pipe, kernel.callback("on_alarmed"));

    kernel.thread().run_event_loop();
}

