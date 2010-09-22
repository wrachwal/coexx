// notify.cpp

#include "notify.h"

#include "coe-thread.h"

#include <iostream>

using namespace std;
using namespace coe;

// ===========================================================================

enum Event {
    Event_A,
    Event_B,
    Event_C
};

void broadcast ()
{
    EventNotifier<Event>    notifier;

    EventReceptor           _going_idle;
    notifier.subscribe(Event_A, _going_idle);
}

// ===========================================================================

void echo (Kernel& kernel, string& ctx, string& arg)
{
    cout << ctx << " -- " << arg << endl;
}

// ***************************************************************************

#define EXEC_(statement)    do { cout << #statement << endl; statement; } while(0)

int main ()
{
    EventNotifier<Event>    notifier;

    Kernel& kernel = Kernel::create_new();

    kernel.state("echo", handler(&::echo));

    EventReceptor   echo_1;
    EventReceptor   echo_2;

    EXEC_(notifier.subscribe(Event_A, echo_1.callback(kernel, "echo", vparam(string("A/1")))));
    EXEC_(notifier.subscribe(Event_A, echo_2.callback(kernel, "echo", vparam(string("A/2")))));
    EXEC_(notifier.subscribe(Event_A, echo_2));
    EXEC_(notifier.subscribe(Event_B, echo_2));
    EXEC_(notifier.notify_event(Event_A, kernel, vparam(string("Event_A"))));
    EXEC_(notifier.notify_event(Event_A, kernel, vparam(string("Event_A"))));
    EXEC_(notifier.notify_event(Event_B, kernel, vparam(string("Event_B"))));
    EXEC_(echo_2.unsubscribe());
    EXEC_(notifier.notify_event(Event_A, kernel, vparam(string("Event_A"))));
    EXEC_(notifier.notify_event(Event_B, kernel, vparam(string("Event_B"))));
    cout << "# STOP." << endl;
}

