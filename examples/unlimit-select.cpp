// unlimit-select.cpp

#include "coe-kernel.h"
#include "coe-thread.h"

#include <iostream>
#include <cstdio>       // perror
#include <cstdlib>      // drand48
#include <cerrno>

#include <unistd.h>     // pipe

using namespace std;
using namespace coe;

// ===========================================================================

static void reader (Kernel& kernel, IO_Ctx& io)
{
    char    buf[128];
    int res = read(io.fileno(), buf, sizeof(buf));
    if (res >= 0) {
        cout.write(buf, res) << flush;
    }
    else {
        if (res == -1 && errno == EINTR) {
            return; ///
        }
        perror("\nread");
        exit(EXIT_FAILURE);
    }
}

static void writer (Kernel& kernel, IO_Ctx& io)
{
    int res = write(io.fileno(), ".", 1);
    if (res == 1) {
        kernel.select_pause(io.fileno(), IO_write);
        double  after = 0.01 + 10 * drand48();
        kernel.delay_set("wsched", TimeSpec(after), vparam(io.fileno()));
    }
    else {
        if (res == -1 && errno == EINTR) {
            return; ///
        }
        perror("\nwrite");
        exit(EXIT_FAILURE);
    }
}

static void wsched (Kernel& kernel, int& w_fd)
{
    kernel.select_resume(w_fd, IO_write);
}

static void mkpipe (Kernel& kernel)
{
    const int EXTREME = 2 * FD_SETSIZE;
    int fds[2];
    if (pipe(fds) < 0) {
        perror("\npipe");
        exit(EXIT_FAILURE);
    }
    else {
        int r_fd = fds[0];
        int w_fd = fds[1];
        if (r_fd > EXTREME) {
            close(r_fd);
            close(w_fd);
            cout << "\nTime to finish! More than " << EXTREME
                 << " descriptors have been used." << endl;
            exit(EXIT_SUCCESS);
        }
        else {
            cout << "(" << r_fd << ":" << w_fd << ")" << flush;
            kernel.select(r_fd, IO_read,  "reader");
            kernel.select(w_fd, IO_write, "writer");
            kernel.delay_set("mkpipe", TimeSpec(0.1));
        }
    }
}

// ***************************************************************************

int main ()
{
    Kernel& kernel = Kernel::create_new();
    kernel.state("reader", handler(&::reader));
    kernel.state("writer", handler(&::writer));
    kernel.state("wsched", handler(&::wsched));
    kernel.state("mkpipe", handler(&::mkpipe));
    kernel.yield("mkpipe");
    kernel.thread().run_event_loop();
}

