// safe-cb.cpp -- callbacks and their arguments' validity guarantee

#include "coe-session.h"
#include "coe-thread.h"     // run_event_loop

#include <iostream>
#include <cstdlib>          // exit

using namespace std;
using namespace coe;

// ===========================================================================

class Fragile {
protected:
    virtual ~Fragile () {}
    virtual int val () const = 0;
    virtual int inc () = 0;
};

// ------------------------------------

class Sensitive : public Fragile {
public:
    Sensitive () : _counter(0)
        {
            cout << "  + Sensitive [objects="
                 << ++_objects << ']' << endl;
        }
    Sensitive (const Sensitive& rhs) : _counter(rhs._counter)
        {
            cout << "  + Sensitive :counter=" << _counter << " {copy} [objects="
                 << ++_objects << ']' << endl;
        }
    ~Sensitive ()
        {
            cout << "  ~ Sensitive :counter=" << _counter << " [objects="
                 << --_objects << ']' << endl;
            _counter = /*garbage*/-1000;
        }
    int val () const { return _counter; }
    int inc () { return ++_counter; }
private:
    void operator= (const Sensitive&);  // not-implemented
    int _counter;
    static int _objects;    // here for debug purposes (usage not thread safe)
};

// ------------------------------------

int Sensitive::_objects = 0;

// ===========================================================================

class MySession : public Session {
public:
    static void spawn (Kernel& kernel)
        {
            cout << "@ " << __FUNCTION__ << endl;
            (new MySession)->start_session(kernel);
        }
private:
    MySession ()
        :   Session(handler(*this, &MySession::_start)),
            _cb(NULL)
        {
            cout << "@ " << __FUNCTION__ << endl;
        }
    ~MySession ()
        {
            cout << "@ " << __FUNCTION__ << endl;
            // because callbacks are free objects, never owned by the framework,
            // they must be explicitly deleted, and destructors are a good place
            // of last resort cleanup.
            delete _cb;
            _cb = NULL;
        }
    void _start (Kernel& kernel)
        {
            cout << "@ " << __FUNCTION__ << endl;
            stop_handler(handler(*this, &::MySession::_stop));

            cout << "# creating callback..." << endl;
            _cb = kernel.callback("kick", vparam(Sensitive()));
            kernel.state("kick", handler(*this, &MySession::kick));

            cout << "# POST'ing callback..." << endl;
            _cb->post(kernel, vparam(10));
        }
    void kick (Kernel& kernel, Sensitive& counter, int& limit)
        {
            cout << "@ " << __FUNCTION__ << ": counter=" << counter.val() << endl;

            if (counter.val() < limit) {

                counter.inc();

                cout << "# CALL'ing callback..." << endl;
                _cb->call(kernel, vparam(limit));   // recursion via callback
            }
            else {

                cout << "# delete callback... (limit reached)" << endl;
                delete _cb;
                _cb = NULL; // needed because of cleanup in destructor

                cout << "# stop_session()..." << endl;
                stop_session();
            }

            // guarantee: the `counter' argument which comes from the callback
            // will still be valid even after `delete _cb' has been done.
            cout << "$ " << __FUNCTION__ << ": counter=" << counter.val() << endl;
        }
    void _stop (Kernel& kernel)
        {
            cout << "@ " << __FUNCTION__ << endl;
        }

    Callback*   _cb;
};

// ***************************************************************************

void terminate (Kernel& kernel)
{
    cout << "@ " << __FUNCTION__ << endl;
    exit(0);
}

// ------------------------------------

int main ()
{
    cout << "@ " << __FUNCTION__ << endl;

    Kernel& kernel = Kernel::create_new();

    MySession::spawn(kernel);

    kernel.delay_set("terminate", TimeSpec(0.2));
    kernel.state("terminate", handler(&::terminate));

    cout << "[entering] Thread::run_event_loop()" << endl;

    kernel.thread().run_event_loop();
}

