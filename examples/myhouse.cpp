// $Id$

#include "coe-session.h"
#include "coe-thread.h"
#include "coe-misc.h"   // owned_ptr

#include <iostream>
#include <unistd.h>     // read() on Linux

using namespace std;

// =======================================================================

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
            return kernel.start_session(new MyHouse(name));
        }
    ~MyHouse ()
        {}

private:
    MyHouse (string name) : _name(name)
        {
            set_heap(this);
        }

    void _start (EvCtx& ctx)
        {
            ctx.kernel.state("leaving", handler(*this,    &MyHouse::on_leaving_msg));
            ctx.kernel.state("coridor", handler(_coridor, &Coridor::on_coridor_msg));
            ctx.kernel.state("room2",   handler(*this,    &MyHouse::on_room2));
            ctx.kernel.state("knock",   handler(*this,    &MyHouse::on_knock));
            ctx.kernel.state("wife",    handler(*this,    &MyHouse::on_wife));
            ctx.kernel.state("money",   handler(*this,    &MyHouse::on_money));

            ctx.kernel.state("command", handler(*this,    &MyHouse::on_command));
            ctx.kernel.select(0, IO_read, "command", pparam(0));
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
    public:
        void on_coridor_msg (TEvCtx<MyHouse>& ctx, string& msg)
            {
                cout << __FUNCTION__ << ": TEvCtx<MyHouse> -> " << ctx->_name << endl;
                msg += string(" (called from ") + __FUNCTION__ + ")";
                ctx->print_msg(msg);    // typed heap :)
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
            ctx.kernel.call(ID(), "money", cparam(cash));
            cout << "and she now have " << cash << "." << endl;

            cash = 1;
            ctx.kernel.call(ID(), "money", cparam(cash));
            cout << "and she now have " << cash << "." << endl;
        }

    void on_money (EvCtx& ctx, int& pln)
        {
            cout << "i got " << pln << " zlotych and ";
            int change = pln <= 1 ? +50 : -pln/2;
            cout << (change < 0 ? "took " : "added ") << abs(change) << endl;
            pln += change;
        }

    string  _name;
};

// =======================================================================

#define TABLEN(tab)     int(sizeof(tab) / sizeof((tab)[0]))

void test_my_house ()
{
    TiD tid[10];

    for (int i = 0; i < 10; ++i) {
        tid[i] = Thread::spawn_new();
        cout << "created " << tid[i] << " thread." << endl;
    }

    // ================================

    Kernel& kernel = Kernel::create_new();

    SiD tar = MyHouse::spawn(kernel, "waldy");

    kernel.post(tar, "leaving", pparam(string("it's the string argument ;)")));

    //GOOD: I had to add (char*) cast to prevent from below the error.
    // dynacall.cpp: In constructor 'PostArgs1<A1>::PostArgs1(const A1&) [with A1 = char [22]]':
    // dynacall.cpp:192:   instantiated from 'bool Kernel::post(SiD, const std::string&, const A1&) [with A1 = char [22]]'
    // dynacall.cpp:307:   instantiated from here
    // dynacall.cpp:148: error: ISO C++ forbids assignment of arrays
    kernel.post(tar, "leaving", pparam((char*)"raw C-style string :("));

    kernel.post(tar, "leaving", pparam(string("once again with valid string type :)")));

    kernel.post(tar, "coridor", pparam(string("and to the coridor... :)")));

    kernel.post(tar, "coridor");   // :( mismatch number of params

    kernel.post(tar, "knock");
    kernel.post(tar, "knock", pparam((char*)"excessive argument!"));

#if 0
    owned_ptr<Flowers>  flowers(new Flowers(42, "rose"));
    kernel.post(tar, "wife", pparam(flowers));
#else
    kernel.post(tar, "wife", pparam(owned_ptr<Flowers>(new Flowers(42, "rose"))));
#endif

    kernel.run_event_loop();    // *** block in main thread
}

// =======================================================================

int main ()
{
    test_my_house();
    cout << "# DONE." << endl;
    return 0;
}

