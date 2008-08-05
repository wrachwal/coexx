// dynacall.cpp

#include "coe-session.h"

#include <iostream>
#include <memory>       // auto_ptr

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
            Session* s = new MyHouse(name);
            kernel.start_session(s);
            return s->ID();
        }
    ~MyHouse ()
        { delete _coridor; }

private:
    MyHouse (string name) : _name(name)
        {
            set_heap(this);
            _coridor = new Coridor;
        }

    void _start (EvCtx& ctx)
        {
            ctx.kernel.state("leaving", handler(*this,     &MyHouse::on_leaving_msg));
            ctx.kernel.state("coridor", handler(*_coridor, &Coridor::on_coridor_msg));
            ctx.kernel.state("room2",   handler(*this,     &MyHouse::on_room2));
            ctx.kernel.state("knock",   handler(*this,     &MyHouse::on_knock));
            ctx.kernel.state("wife",    handler(*this,     &MyHouse::on_wife));
            ctx.kernel.state("money",   handler(*this,     &MyHouse::on_money));
        }

    void print_msg (string& msg)
        { cout << _name << " << " << msg << endl; }

    void on_leaving_msg (EvCtx& ctx, string& msg)
        {
            msg += string(" (called from ") + __FUNCTION__ + ")";
            print_msg(msg);
        }

    class Coridor {
    public:
        void on_coridor_msg (TEvCtx<MyHouse>& ctx, string& msg)
            {
                cout << __FUNCTION__ << ": TEvCtx<MyHouse> -> " << ctx.heap << endl;
                msg += string(" (called from ") + __FUNCTION__ + ")";
                ctx->print_msg(msg);    // typed heap :)
            }
    };

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

    void on_wife (EvCtx& ctx, auto_ptr<Flowers>& flowers)
        {
            cout << "wife is given " << *flowers << endl;
            flowers.reset(NULL);
            cout << "you know, she was angry ;)" << endl;
        }

    void on_money (EvCtx& ctx, int& pln)
        {
            cout << "i got " << pln << " zlotych and ";
            int change = pln <= 1 ? +50 : -pln/2;
            cout << (change < 0 ? "took " : "added ") << abs(change) << endl;
            pln += change;
        }

    Coridor* _coridor;

    string  _name;
};

// =======================================================================

void type_info_show ();

void test_coe ()
{
    Kernel& kernel = Kernel::instance();

    SiD tar = MyHouse::spawn(kernel, "waldy");

    kernel.post(tar, "leaving", vparam(string("it's the string argument ;)")));

    //GOOD: I had to add (char*) cast to prevent from below the error.
    // dynacall.cpp: In constructor 'PostArgs1<A1>::PostArgs1(const A1&) [with A1 = char [22]]':
    // dynacall.cpp:192:   instantiated from 'bool Kernel::post(SiD, const std::string&, const A1&) [with A1 = char [22]]'
    // dynacall.cpp:307:   instantiated from here
    // dynacall.cpp:148: error: ISO C++ forbids assignment of arrays
    kernel.post(tar, "leaving", vparam((char*)"raw C-style string :("));

    kernel.post(tar, "leaving", vparam(string("once again with valid string type :)")));

    kernel.run_event_loop();    // ***

    //FIXME: on_coridor_msg: TEvCtx<MyHouse> -> 0
    exit(0);

    kernel.post(tar, "coridor", vparam(string("and to the coridor... :)")));

    kernel.post(tar, "coridor");   // :( mismatch number of params

    kernel.post(tar, "knock");
    kernel.post(tar, "knock", vparam((char*)"excessive argument!"));

    kernel.select(1, Kernel::IO_write, "room2", vparam(string("polling WRITE to stdout :)")));
    kernel.select(2, Kernel::IO_read,  "room2", vparam(string("polling READ for stderr :)")));

    kernel.select(0, Kernel::IO_read,  "room2", vparam(double(666)));   // arg#1 type mismatch

    //XXX   due to auto_ptr's rhs parameter in copy-constructor is not of type *const*
    //      the following doesn't compile:
    //kernel.post(tar, "wife", auto_ptr<Flowers>(new Flowers(42, "rose")));
    //      the solution is to pass auto_ptr in form of auxiliary variable,
    //      but also requires const_cast<>() in the library,
    //      which probably is awful.
    auto_ptr<Flowers>   flowers(new Flowers(42, "rose"));
    kernel.post(tar, "wife", vparam(flowers));

    int cash = 10;
    kernel.call(tar, "money", rparam(cash));
    cout << "and she now have " << cash << "." << endl;

    cash = 1;
    kernel.call(tar, "money", rparam(cash));
    cout << "and she now have " << cash << "." << endl;
}

// ------------------------------------

int main ()
{
    type_info_show();
    cout << string(75, '#') << endl;

    test_coe();
    cout << "# DONE." << endl;

    return 0;
}

