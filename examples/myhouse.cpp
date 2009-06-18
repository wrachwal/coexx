// myhouse.cpp -- hodgepodge, my first and ever growing example ;)

#include "coe-session.h"
#include "coe-thread.h" // tls<T>(), run_event_loop()
#include "coe-misc.h"   // owned_ptr

#include <cstdlib>      // abs
#include <cstring>      // strncmp
#include <unistd.h>     // read() on Linux
#include <iostream>
#include <cassert>

using namespace std;
using namespace coe;

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

    void _start (EvCtx& ctx)
        {
            cout << __PRETTY_FUNCTION__ << " in thread " << ctx.kernel.thread().ID()
                 << ": sid=" << ctx.session.ID() << " state=" << ctx.state << endl;

            ctx.kernel.state("leaving", handler(*this,    &MyHouse::on_leaving_msg));
            ctx.kernel.state("coridor", handler(_coridor, &Coridor::on_coridor_msg));
            ctx.kernel.state("room2",   handler(*this,    &MyHouse::on_room2));
            ctx.kernel.state("knock",   handler(*this,    &MyHouse::on_knock));
            ctx.kernel.state("wife",    handler(*this,    &MyHouse::on_wife));
            ctx.kernel.state("money",   handler(*this,    &MyHouse::on_money));

            ctx.kernel.state("command", handler(*this,    &MyHouse::on_command));
            ctx.kernel.select(0, IO_read, "command", vparam(0));

            stop_handler(handler(*this, &MyHouse::_stop));

            ctx.kernel         .kls<string>() = "** string set in " + ctx.state + " **";
            ctx.kernel.thread().tls<string>() = "** string set in " + ctx.state + " **";

            ctx.kernel.state("on_expiry", handler(*this, &MyHouse::on_expiry));
            AiD alarm = ctx.kernel.delay_set("on_expiry", TimeSpec(10.0));
            cout << "Alarm " << alarm << " expires in 10 seconds." << endl;
        }
    void _stop (EvCtx& ctx)
        {
            cout << __PRETTY_FUNCTION__ << " in thread " << ctx.kernel.thread().ID()
                    << ": sid=" << ctx.session.ID() << " state=" << ctx.state
                 << "\n# kls<string>() -> " << ctx.kernel         .kls<string>()
                 << "\n# tls<string>() -> " << ctx.kernel.thread().tls<string>() << endl;
        }

    void print_msg (string& msg)
        { cout << _name << " << " << msg << endl; }

    void on_command (EvCtx& ctx, DatIO& io, int& count)
        {
            char    buf[256];

            ssize_t nbytes = read(io.filedes, buf, sizeof(buf)-1);

            if (nbytes <= 0) {
                ctx.kernel.select(io.filedes, io.mode);
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
                    ctx.kernel.select(io.filedes, io.mode);
                }
            }
        }

    void on_leaving_msg (EvCtx& ctx, string& msg)
        {
            msg += string(" (called from ") + __FUNCTION__ + ")";
            print_msg(msg);
        }

    class Coridor {
        MyHouse&    _house;
    public:
        Coridor (MyHouse& house) : _house(house) {}
        void on_coridor_msg (EvCtx& ctx, string& msg)
            {
                cout << __FUNCTION__ << ": Kernel::current_session() -> "
                     << Kernel::current_session()
                     << endl;
                msg += string(" (called from ") + __FUNCTION__ + ")";
                // NOTE: nested class has access to outer's private!
                _house.print_msg(msg);
            }
    } _coridor;

    void on_room2 (EvCtx& ctx, const DatIO& io, string& msg)
        {
            cout << __FUNCTION__
                 << " <IO: fd=" << io.filedes << " mode=" << io.mode << ">"
                 << " a1=\"" << msg << '"'
                 << endl;
        }

    void on_knock (EvCtx& ctx)
        {
            cout << "someone knocks to the door..." << endl;
        }

    void on_wife (EvCtx& ctx, owned_ptr<Flowers>& flowers)
        {
            cout << "wife is given " << *flowers << endl;
            delete flowers.release();
            cout << "you know, she was angry ;)" << endl;

            // ------------------------

            int cash = 10;
            ctx.kernel.call(ID(), "money", rparam(cash));
            cout << "and she now have " << cash << "." << endl;

            cash = 1;
            ctx.kernel.call(ID(), "money", rparam(cash));
            cout << "and she now have " << cash << "." << endl;
        }

    void on_money (EvCtx& ctx, int& pln)
        {
            cout << "i got " << pln << " zlotych and ";
            int change = pln <= 1 ? +50 : -pln/2;
            cout << (change < 0 ? "took " : "added ") << abs(change) << endl;
            pln += change;
        }

    void on_expiry (EvCtx& ctx)
        {
            cout << "alarm " << ctx.alarm_id
                 << " expired -- calling stop_session()..." << endl;
            stop_session();
        }

    string  _name;
};

// ===========================================================================

#define TABLEN(tab)     int(sizeof(tab) / sizeof((tab)[0]))

static void print_after_house_expiry (EvCtx& ctx)
{
    cout << __PRETTY_FUNCTION__ << " in thread " << ctx.kernel.thread().ID()
         << "\n# kls<string>() --> " << ctx.kernel         .kls<string>()
         << "\n# tls<string>() --> " << ctx.kernel.thread().tls<string>() << endl;
}

void test_my_house ()
{
    Kernel& kernel = Kernel::create_new();  // will attach to current thread

    // ================================
    TiD tid[10];

    for (int i = 0; i < 10; ++i) {
        tid[i] = Thread::spawn_new();
        cout << "created " << tid[i] << " thread." << endl;
    }
    // --------------------------------

    SiD tar = MyHouse::spawn(kernel, "waldy");

    kernel.post(tar, "leaving", vparam(string("it's the string argument ;)")));

    //GOOD: I had to add (char*) cast to prevent from below the error.
    // dynacall.cpp: In constructor 'PostArgs1<A1>::PostArgs1(const A1&) [with A1 = char [22]]':
    // dynacall.cpp:192:   instantiated from 'bool Kernel::post(SiD, const std::string&, const A1&) [with A1 = char [22]]'
    // dynacall.cpp:307:   instantiated from here
    // dynacall.cpp:148: error: ISO C++ forbids assignment of arrays
    kernel.post(tar, "leaving", vparam((char*)"raw C-style string :("));

    kernel.post(tar, "leaving", vparam(string("once again with valid string type :)")));

    kernel.post(tar, "coridor", vparam(string("and to the coridor... :)")));

    kernel.post(tar, "coridor");   // :( mismatch number of params

    kernel.post(tar, "knock");
    kernel.post(tar, "knock", vparam((char*)"excessive argument!"));

    kernel.post(tar, "wife", vparam(owned_ptr<Flowers>(new Flowers(42, "rose"))));

#if 1
    bool trans = kernel.move_to_thread(tid[0]);
         trans = trans;
    assert(trans);
#endif

    // --------------------------------

    Kernel& other_kernel = Kernel::create_new();
    other_kernel.delay_set("print_after_house_expiry", TimeSpec(11.0));
    other_kernel.state("print_after_house_expiry", handler(&::print_after_house_expiry));

    // --------------------------------

    kernel.thread().run_event_loop();       // *** block in main thread
                                            // kernel will go to tid[0]
}

// ***************************************************************************

int main ()
{
    test_my_house();
    cout << "# DONE." << endl;
    return 0;
}

