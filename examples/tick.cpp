// $Id$

#include "coe-session.h"

#include <iostream>

using namespace std;

// -----------------------------------------------------------------------

class MyClock : public Session {
public:
    static SiD spawn (Kernel& kernel)
        {
            return kernel.start_session(new MyClock);
        }
private:
    void _start (EvCtx& ctx)
        {
            ctx.kernel.yield("next");
            ctx.kernel.state("next", handler(*this, &MyClock::next));
        }
    void next (EvCtx& ctx)
        {
            cout << "tick... (from " << ctx.sender
                 << ", state "       << ctx.sender_state << ")" << endl;
            ctx.kernel.delay_set("next", TimeSpec(1, 0));
        }
};

// =======================================================================

int main ()
{
    Kernel& kernel = Kernel::create_new();
    MyClock::spawn(kernel);
    kernel.run_event_loop();
    return 0;
}

