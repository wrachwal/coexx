// myhouse.cpp -- hodgepodge, my first and ever growing example ;)

#include "coe-session.h"
#include "coe-thread.h" // tls<T>(), run_event_loop()
#include "coe-misc.h"   // owned_ptr

#include <cstdlib>      // abs, exit
#include <cstring>      // strncmp
#include <unistd.h>     // read() on Linux
#include <iostream>
#include <cassert>

using namespace std;
using namespace coe;

#define EVAL_(expr)     #expr << " --> " << (expr)

// ---------------------------------------------------------------------------

struct Ev_0 : event<> {};
struct Ev_1 : event<int> {};
struct Ev_2 : event<string, int> {};
struct Ev_2a : Ev_2 {}; // events inheritance

// ===========================================================================

class Flowers {
public:
    Flowers (unsigned count, string name)
        : _name(name), _count(count)
        {
            cout << "+ creating " << *this << "..." << endl;
        }
    virtual ~Flowers ()
        {
            cout << "- deleting " << *this << "..." << endl;
            _name = "wilted";
        }
    friend ostream& operator<< (ostream& os, const Flowers& f)
        { return os << f._count << '-' << f._name << "(s)"; }
private:
    string      _name;
    unsigned    _count;
};

// ------------------------------------

class MyHouse : public Session {
public:
    static SiD spawn (Kernel& kernel, string name)
        {
            return (new MyHouse(name))->start_session(kernel);
        }
private:
    MyHouse (string name)
        :   Session(handler(*this, &MyHouse::_start)),
            _coridor(*this),
            _name(name)
        {
        }
    ~MyHouse ()
        {}

    static void idle (Kernel&) {}

    void _start (Kernel& kernel)
        {

            cout << "\n### Comparing objects of type Handler0, HandlerX..." << endl;
            // To effectively compare Handler0 with HandlerX, first we have to
            // convert specific type Handler0 to more general HandlerX:
            cout << EVAL_(HandlerX(handler(*this, &MyHouse::_start))
                                == handler(*this, &MyHouse::on_leaving_msg))    << endl;

            cout << EVAL_(handler(*this, &MyHouse::_start)
                       == handler(&MyHouse::idle))                              << endl;
            cout << EVAL_(handler(*this, &MyHouse::_start)
                       == handler(*this, &MyHouse::_start))                     << endl;
            cout << EVAL_(handler(*this, &MyHouse::_start)
                       == handler(*this, &MyHouse::_stop))                      << endl;
            cout << EVAL_(handler(*this, &MyHouse::_stop)
                       == handler(*this, &MyHouse::_stop))                      << endl;

            cout << EVAL_(handler(*this, &MyHouse::_start)
                       != handler(*this, &MyHouse::_stop))                      << endl;
            cout << EVAL_(handler(*this, &MyHouse::_stop)
                       != handler(*this, &MyHouse::_stop))                      << endl;

            cout << "\n### Safe [to-bool] conversion operators..." << endl;
            cout << EVAL_(Handler0() ? "YES" : "NO")                                << endl;
            cout << EVAL_(HandlerX() ? "YES" : "NO")                                << endl;
            cout << EVAL_(handler(*this, &MyHouse::_start) ? "YES" : "NO")          << endl;
            cout << EVAL_(handler(*this, &MyHouse::on_leaving_msg) ? "YES" : "NO")  << endl;
            cout << EVAL_(handler(&MyHouse::idle) ? "YES" : "NO")                   << endl;

            cout << __PRETTY_FUNCTION__ << " in thread " << kernel.thread().ID()
                 << ": sid=" << kernel.session().ID() << " state=" << kernel.context().state() << endl;

            kernel.state("leaving", handler(*this,    &MyHouse::on_leaving_msg));
            kernel.state("coridor", handler(_coridor, &Coridor::on_coridor_msg));
            kernel.state("room2",   handler(*this,    &MyHouse::on_room2));
            kernel.state("knock",   handler(*this,    &MyHouse::on_knock));
            kernel.state("wife",    handler(*this,    &MyHouse::on_wife));
            kernel.state("money",   handler(*this,    &MyHouse::on_money));

            kernel.state("command", handler(*this,    &MyHouse::on_command));
            kernel.select(0, IO_read, "command", vparam(0));

            stop_handler(handler(*this, &MyHouse::_stop));

            kernel         .kls<string>() = string("** string set in ") + kernel.context().state().c_str() + " **";
            kernel.thread().tls<string>() = string("** string set in ") + kernel.context().state().c_str() + " **";

            kernel.state("on_expiry", handler(*this, &MyHouse::on_expiry));
            AiD alarm = kernel.delay_set("on_expiry", TimeSpec(10.0));
            cout << "Alarm " << alarm << " expires in 10 seconds." << endl;
        }
    void _stop (Kernel& kernel)
        {
            cout << __PRETTY_FUNCTION__ << " in thread " << kernel.thread().ID()
                    << ": sid=" << kernel.session().ID() << " state=" << kernel.context().state()
                 << "\n# kls<string>() -> " << kernel         .kls<string>()
                 << "\n# tls<string>() -> " << kernel.thread().tls<string>() << endl;
        }

    void print_msg (string& msg)
        { cout << _name << " << " << msg << endl; }

    void on_command (Kernel& kernel, IO_Ctx& io, int& count)
        {
            char    buf[256];

            ssize_t nbytes = read(io.fileno(), buf, sizeof(buf)-1);

            if (nbytes <= 0) {
                kernel.select(io.fileno(), io.mode());
            }
            else {
                buf[nbytes] = 0;
                count += 1;
                cout << count << "> " << buf;

                if (count == 20) {
                    cout << __FUNCTION__ << ": hit 20th cmd --> stop_session().." << endl;
                    stop_session();
                }
                else
                if (0 == strncmp(buf, "close", 5)) {
                    cout << "### leaving out command prompt" << endl;
                    kernel.select(io.fileno(), io.mode());
                }
            }
        }

    void on_leaving_msg (Kernel& kernel, string& msg)
        {
            msg += string(" (called from ") + __FUNCTION__ + ")";
            print_msg(msg);
        }

    class Coridor {
        MyHouse&    _house;
    public:
        Coridor (MyHouse& house) : _house(house) {}
        void on_coridor_msg (Kernel& kernel, string& msg)
            {
                cout << __FUNCTION__ << ": Session::current_session() -> "
                     << SiD(Session::current_session())
                     << endl;
                msg += string(" (called from ") + __FUNCTION__ + ")";
                // NOTE: nested class has access to outer's private!
                _house.print_msg(msg);
            }
    } _coridor;

    void on_room2 (Kernel& kernel, const IO_Ctx& io, string& msg)
        {
            cout << __FUNCTION__
                 << " <IO: fd=" << io.fileno() << " mode=" << io.mode() << ">"
                 << " a1=\"" << msg << '"'
                 << endl;
        }

    void on_knock (Kernel& kernel)
        {
            cout << "someone knocks to the door..." << endl;
        }

    void on_wife (Kernel& kernel, owned_ptr<Flowers>& flowers)
        {
            cout << "wife is given " << *flowers << endl;
            delete flowers.release();
            cout << "you know, she was angry ;)" << endl;

            // ------------------------

            int cash = 10;
            kernel.call(ID(), "money", rparam(cash));
            cout << "and she now have " << cash << "." << endl;

            cash = 1;
            kernel.call(ID(), "money", rparam(cash));
            cout << "and she now have " << cash << "." << endl;
        }

    void on_money (Kernel& kernel, int& pln)
        {
            cout << "i got " << pln << " zlotych and ";
            int change = pln <= 1 ? +50 : -pln/2;
            cout << (change < 0 ? "took " : "added ") << abs(change) << endl;
            pln += change;
        }

    void on_expiry (Kernel& kernel)
        {
            cout << "alarm " << kernel.context().alarm_id()
                 << " expired -- calling stop_session()..." << endl;
            stop_session();
        }

    string  _name;
};

// ===========================================================================

#define TABLEN(tab)     int(sizeof(tab) / sizeof((tab)[0]))

static void print_after_house_expiry (Kernel& kernel)
{
    cout << __PRETTY_FUNCTION__ << " in thread " << kernel.thread().ID()
         << "\n# kls<string>() --> " << kernel         .kls<string>()
         << "\n# tls<string>() --> " << kernel.thread().tls<string>() << endl;
}

static void exit_app (Kernel& kernel)
{
    cout << "### the app is about to exit(0)." << endl;
    exit(0);
}

static bool unloop (Thread& thread)
{
    return true;
}

void test_my_house ()
{
    Kernel& kernel1 = Kernel::create_new();     // will attach to current thread

    // ================================
    TiD tid[10];

    for (int i = 0; i < 10; ++i) {
        tid[i] = Thread::spawn_new();
        cout << "created " << tid[i] << " thread." << endl;
    }
    // --------------------------------

    SiD tar = MyHouse::spawn(kernel1, "waldy");

    kernel1.post(tar, "leaving", vparam(string("it's the string argument ;)")));

    kernel1.post<Ev_0>(tar);
    kernel1.post<Ev_1>(tar, vparam(0));
    kernel1.post<Ev_2>(tar, vparam(string("ala"), 0));
    kernel1.post<Ev_2a>(tar, vparam(string(), 77));

    delete vparam(666);

    ValParam_<Ev_1::args_type>  vp1_a = vparam(44);
    assert(vp1_a != NULL);
    delete vp1_a;
    assert(vp1_a != NULL);

    ValParam_<Ev_1::args_type>  vp1_b = vparam(33);
    assert(vp1_b != NULL);
    delete vp1_b.clear();
    assert(vp1_b == NULL);

    //GOOD: I had to add (char*) cast to prevent from below the error.
    // dynacall.cpp: In constructor 'PostArgs1<A1>::PostArgs1(const A1&) [with A1 = char [22]]':
    // dynacall.cpp:192:   instantiated from 'bool Kernel::post(SiD, const std::string&, const A1&) [with A1 = char [22]]'
    // dynacall.cpp:307:   instantiated from here
    // dynacall.cpp:148: error: ISO C++ forbids assignment of arrays
    kernel1.post(tar, "leaving", vparam((char*)"raw C-style string :("));

    kernel1.post(tar, "leaving", vparam(string("once again with valid string type :)")));

    kernel1.post(tar, "coridor", vparam(string("and to the coridor... :)")));

    char    coridor[] = "coridor";
    cout << "before posting on-stack c-string (COEXX_COESTR=" << COEXX_COESTR << ") ..." << endl;
    kernel1.post(tar, coridor);     // :( mismatch number of params
    cout << "after posted on-stack c-string :)" << endl;

    kernel1.post(tar, "knock");
    kernel1.post(tar, "knock", vparam((char*)"excessive argument!"));

    kernel1.post(tar, "wife", vparam(owned_ptr<Flowers>(new Flowers(42, "rose"))));

    kernel1.call(SiD(kernel1.ID(), 1/*root*/), "wrong-path");

    kernel1.move_to_thread(tid[0]);

    // --------------------------------

    Kernel& kernel2 = Kernel::create_new();
    kernel2.delay_set("print_after_house_expiry", TimeSpec(11.0));
    kernel2.state("print_after_house_expiry", handler(&::print_after_house_expiry));

    kernel2.move_to_thread(tid[1]);

    // --------------------------------

    Thread& t_main = kernel1.thread();  // could be kernel2 as well

    // Block main thread until both kernels reach their target threads.
    // BEWARE: After run_event_loop() kernel1/kernel2 references
    // must *not* be used, and for this reason we kept the reference
    // to the main thread earlier.
    t_main.run_event_loop(&::unloop);

    Kernel& kernel3 = Kernel::create_new();
    kernel3.delay_set("last_print", TimeSpec(12.0));
    kernel3.state("last_print", handler(&::print_after_house_expiry));

    kernel3.delay_set("exit_app", TimeSpec(15.0));
    kernel3.state("exit_app", handler(&::exit_app));

    t_main.run_event_loop();    // block infinitely
}

// ***************************************************************************

int main ()
{
    test_my_house();
    cout << "# DONE." << endl;
    return 0;
}

