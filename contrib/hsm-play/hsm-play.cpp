// hsm-play.cpp

#include "hsm-play.h"

#include "coe-hsm.h"
#include "coe-hsm-misc.h"   // build_abbrev_to_state_map

#include "coe-session.h"
#include "coe-thread.h"

#include <iostream>
#include <sstream>
#include <vector>

#include <cstdlib>          // exit
#include <cerrno>
#include <cassert>

using namespace std;
using namespace coe;
using namespace coe::hsm;

#define TABLEN(tab)     (sizeof(tab) / sizeof((tab)[0]))

// ===========================================================================
// RegisterExampleStateMachine

typedef aComposition* (*SMCreator)();
typedef map<string, SMCreator> SMMap;

static SMMap& g_menu ()
{
    static SMMap    menu;
    return menu;
}

RegisterExampleStateMachine::
RegisterExampleStateMachine (const char* type, SMCreator creator)
    { ::g_menu()[type] = creator; }

ostream& example_machines (ostream& os)
{
    for (map<string, SMCreator>::iterator i = g_menu().begin(); i != g_menu().end(); ++i)
        os << (i == g_menu().begin() ? "- " : "\n- ") << (*i).first;
    return os;
}

// ---------------------------------------------------------------------------
// SMSession -- session hosting single state machine to play with

class SMSession : public Session {
public:
    static SiD machine_session (string name);   // query registered by name
    static void spawn (Kernel& kernel, string name, SMCreator creator);

private:
    SMSession (aComposition* root);
    ~SMSession ();

    typedef SMSession Self;
    void _start (Kernel& kernel, string& name);
    void command_select_machine (Kernel& kernel);
    void command_transitions_set (Kernel& kernel, vector<pair<string, string> >& trans);
    void command_stop_session (Kernel& kernel);
    aState* find_state (string state) const;

private:    // data
    aComposition*           _root;
    map<string, aState*>    _abbr;
};

// ===========================================================================

static inline ostream& echo_line (bool echo, string line)
{
    if (echo)   // self-document in batch mode
        cout << line << endl;
    return cout;
}

static bool interp_command_line (string line, bool echo, Kernel& kernel, string& selsm)
{
    istringstream   iss(line);

    if ((iss >> ws).eof())
        return false;

    string  cmd;
    if (! (iss >> cmd >> ws))
        return false;
    assert(cmd.length() > 0);

    if (*cmd.begin() == ':') {
        echo_line(echo, line) << endl;
        if (cmd == ":h") {
            cout << "\n\
:h                  ## this screen\n\
:m [<machine>]      ## list all or select state machine\n\
$ <to>              ## simple transition to <to> state\n\
$ <to1> <to2> ...   ## compound transition\n\
$ <to1\\ex1> ...     ## transition via (ex)iting state\n\
$$                  ## transition to terminate state\n\
~<ev> [<arg1> ...]  ## post <ev>ent with args (integers)\n\
@                   ## stop current machine session\n\
" << endl;
            return false;
        }
        else
        if (cmd == ":m") {
            if (iss.eof()) {    // list all
                cout << "state machines:\n" << example_machines << endl;
            }
            else {              // select one
                iss >> selsm;
                if (! ::g_menu().count(selsm)) {
                    cout << "!!! no such machine `" << selsm
                         << "' -- enter :m command to list all" << endl;
                    selsm.clear();
                    return false;
                }

                SiD smsid = SMSession::machine_session(selsm);
                if (! smsid) {
                    SMCreator   creator = ::g_menu()[selsm];
                    assert(NULL != creator);
                    SMSession::spawn(kernel, selsm, creator);
                    smsid = SMSession::machine_session(selsm);
                }

                kernel.post(smsid, "command_select_machine");
                return true;
            }
        }
        else {
            cout << "! bad command `" << cmd << "' -- enter :h command to get help" << endl;
        }
        return false;
    }
    else
    if (cmd == "$") {
        echo_line(echo, line) << endl;
        vector<pair<string, string> >   trans;
        string  to;
        while (iss >> ws >> to) {
            if (to == "$$") {
                trans.push_back(pair<string, string>());
            }
            else {
                string  ex;
                string::size_type   pos;
                if ((pos = to.find_first_of('\\')) != string::npos) {
                    ex = to.substr(pos + 1);
                    to = to.erase(pos);
                }
                trans.push_back(make_pair(to, ex));
            }
        }
        kernel.post(SMSession::machine_session(selsm), "command_transitions_set",
                    vparam(trans));
        return true;
    }
    else
    if (cmd == "$$") {
        echo_line(echo, line) << endl;
        vector<pair<string, string> >   trans;
        trans.push_back(pair<string, string>());
        kernel.post(SMSession::machine_session(selsm), "command_transitions_set",
                    vparam(trans));
        return true;
    }
    else
    if (*cmd.begin() == '~') {
        echo_line(echo, line) << endl;
        string  event(cmd, 1);
        int arg[3] = { 0 };
        size_t  count = 0;
        while (! (iss >> ws).eof()) {
            if (count == TABLEN(arg)) {
                cout << "! allowed up to " << TABLEN(arg) << " arguments." << endl;
                return false;
            }
            if ((iss >> arg[++count]).fail()) {
                cout << "! argument #" << count << " is not an integer." << endl;
                return false;
            }
        }
        SiD smsid = SMSession::machine_session(selsm);
        switch (count) {
            case 0:
                kernel.post(smsid, event);
                break;
            case 1:
                kernel.post(smsid, event, vparam(arg[0]));
                break;
            case 2:
                kernel.post(smsid, event, vparam(arg[0], arg[1]));
                break;
            case 3:
                kernel.post(smsid, event, vparam(arg[0], arg[1], arg[2]));
                break;
            default:
                assert(0);
                break;
        }
        return true;
    }
    else
    if (cmd == "@") {
        echo_line(echo, line) << endl;
        kernel.post(SMSession::machine_session(selsm), "command_stop_session");
        return true;
    }
    else
    if (*cmd.begin() == '#') {
        echo_line(echo, line);
        return false;
    }
    else {
        return false;
    }
}

// ---------------------------------------------------------------------------

static void suicide (Kernel& kernel)
{
    cout << "\nbye!" << endl;
    exit(0);
}

static bool single_async_command (string& buffer, bool echo, Kernel& kernel, string& selsm)
{
    string::size_type   pos;
    while ((pos = buffer.find_first_of('\n')) != string::npos) {
        string  line = buffer.substr(0, pos);   // without newline
        buffer.erase(0, pos + 1);
        if (interp_command_line(line, echo, kernel, selsm)) {
            return true;
        }
    }
    return false;
}

static void on_command (Kernel& kernel, IO_Ctx& io, string& buffer, string& selsm)
{
    bool    echo = ! ::isatty(io.fileno());

    if (single_async_command(buffer, echo, kernel, selsm)) {
        return;
    }

    char    tmp[512];
    ssize_t nbytes = read(io.fileno(), tmp, sizeof(tmp));

    if (nbytes < 0) {
        if (errno == EINTR || errno == EAGAIN)
            return;
        cout << "! fatal: broken input (exiting...)" << endl;
        exit(EXIT_FAILURE);
    }

    if (nbytes > 0) {
        buffer.append(tmp, nbytes);
    }

    if (single_async_command(buffer, echo, kernel, selsm)) {
        return;
    }

    if (0 == nbytes) {

        kernel.select(io.fileno(), io.mode());  // cancel

        if (! buffer.empty()) {
            interp_command_line(buffer, echo, kernel, selsm);
        }

        kernel.state("suicide", handler(&::suicide));
        kernel.yield("suicide");
    }
}

// ---------------------------------------------------------------------------
// SMRegistry

static map<string, SiD> s_sm2sid;   // registry plain map (string --> SiD)
static map<SiD, string> s_sid2sm;   // reverse map (for fast deregistration)

static void _unregister_machine_session (SiD smsid)     // callback function
{
    map<SiD, string>::iterator  i = s_sid2sm.find(smsid);
    assert(i != s_sid2sm.end());
    if (i != s_sid2sm.end()) {
        s_sm2sid.erase((*i).second);
        s_sid2sm.erase(i);
    }
}

static bool register_machine_session (string name, SMSession& smses)    // self-register
{
    if (s_sm2sid.count(name))
        return false;
    if (! smses.unregistrar_set(&::_unregister_machine_session))
        return false;   // must have been registered (but under different name)
    SiD smsid = smses.ID();
    assert(smsid);
    s_sm2sid[name] = smsid;
    s_sid2sm[smsid] = name;
    return true;
}

// ---------------------------------------------------------------------------

static void get_trace_ctrl (Kernel& kernel,
                            Machine& machine,
                            StateTrace::Ctrl& /*output*/ctrl)
{
    ctrl.flag = StateTrace::ALL;    // full trace
}

static void print_action (Kernel& kernel,
                          Machine& machine,
                          StateTrace::Action action,
                          aState* st,
                          aState* ex)
{
    int level = machine.action_level();
    cout << string((level ? abs(level) - 1 : 0) * 3, ' ');  // indent
    switch (action) {
        case StateTrace::OnTr:  cout << "tr @@ ";   break;
        case StateTrace::OnEx:  cout << "ex << ";   break;
        case StateTrace::OnEn:  cout << "en >> ";   break;
    }
    if (st) {
        full_state_name(cout, *st);
        if (ex)
            full_state_name(cout << "(via ", *ex) << ")";
    }
    else {
        cout << "[terminate]";
    }
    cout << endl;
}

static void print_machine (Kernel& kernel, Machine& machine)
{
    string  prefix;
    prefix += machine.is_in_stable_state() ? "|= " : "|? ";
    print_state_machine(cout << "\n", machine, prefix) << endl;
}

// ---------------------------------------------------------------------------

static ostream& operator<< (ostream& os, const pair<string, string>& toex)
{
    if (toex.first.empty()) {
        assert(toex.second.empty());
        return os << "$$";
    }
    else {
        os << toex.first;
        if (! toex.second.empty())
            os << '\\' << toex.second;
        return os;
    }
}

static ostream& operator<< (ostream& os, const pair<aState*, aState*>& toex)
{
    if (NULL == toex.first) {
        assert(NULL == toex.second);
        return os << "$$";
    }
    else {
        full_state_name(os, *toex.first);
        if (NULL != toex.second)
            full_state_name(os << '\\', *toex.second);
        return os;
    }
}

// ---------------------------------------------------------------------------
// SMSession

SiD SMSession::machine_session (string name)
{
    map<string, SiD>::iterator  i = s_sm2sid.find(name);
    return i != s_sm2sid.end() ? (*i).second : SiD::NONE();
}

void SMSession::spawn (Kernel& kernel, string name, SMCreator creator)
{
    (new Self(creator()))->start_session(kernel, vparam(name));
}

SMSession::SMSession (aComposition* root)
:   Session(handler(*this, &Self::_start))
,   _root(root)
{
    assert(NULL != _root);
    ::build_abbrev_to_state_map(_abbr, *_root);
}

SMSession::~SMSession ()
{
    delete _root;
    _root = NULL;   // just in case
}

aState* SMSession::find_state (string state) const
{
    map<string, aState*>::const_iterator    i = _abbr.find(state);
    return i != _abbr.end() ? (*i).second : NULL;
}

void SMSession::_start (Kernel& kernel, string& name)
{
    if (register_machine_session(name, *this)) {

        kernel.state("command_select_machine",
                     handler(*this, &Self::command_select_machine));
        kernel.state("command_transitions_set",
                     handler(*this, &Self::command_transitions_set));
        kernel.state("command_stop_session",
                     handler(*this, &Self::command_stop_session));
    }
    else {
        stop_session();
    }
}

void SMSession::command_select_machine (Kernel& kernel)
{
    ::print_machine(kernel, _root->machine_());
}

void SMSession::command_transitions_set (Kernel& kernel,
                                         vector<pair<string, string> >& trans)
{
    if (trans.empty())
        return;

    vector<pair<aState*, aState*> > toex;
    bool    error = false;

    // translation to aState*s
    cout << "|\n|#\n";
    for (vector<pair<string, string> >::iterator i = trans.begin();
         i != trans.end();
         ++i)
    {
        if ((*i).first.empty()) {
            cout << "|# " << *i << endl;
            toex.push_back(pair<aState*, aState*>(NULL, NULL));
        }
        else {
            aState* to = find_state((*i).first);
            if (NULL == to) {
                cout << "!# " << *i << " (syntax: unknown target state)" << endl;
                error = true;
                continue;
            }
            aState* ex = NULL;
            if (! (*i).second.empty()) {
                ex = find_state((*i).second);
                if (NULL == ex) {
                    cout << "!# " << *i << " (syntax: unknown exit state)" << endl;
                    error = true;
                    continue;
                }
            }
            cout << "|# ";
            full_state_name(cout, *to);
            if (ex)
                full_state_name(cout << '\\', *ex);
            cout << endl;
            toex.push_back(pair<aState*, aState*>(to, ex));
        }
    }
    cout << "|#\n|\n";

    if (error) {
        return;
    }

    Machine&    sm = _root->machine_();

    for (vector<pair<aState*, aState*> >::iterator i = toex.begin();
         i != toex.end();
         ++i)
    {
        aState* to = (*i).first;
        aState* ex = (*i).second;

        cout << "\n||| " << (to ? "$" : "") << *i << "\n" << endl;

        // execute transition (exiting phase)
        if      (ex)    to->transit_ex(kernel, *ex);
        else if (to)    to->transit(kernel);
        else            sm.terminate(kernel);
    }
}

void SMSession::command_stop_session (Kernel& kernel)
{
    cout << "\n|-- Session hosting `" << _root->state_name()
         << "' state-machine has stopped." << endl;
    stop_session();
}

// ***************************************************************************

struct XXX { virtual ~XXX (); };

#define PRINT_SIZEOF_(Type) \
    do { cout << "# sizeof(" #Type ") = " << sizeof(Type) << endl; } while(0)

// ---------------------------------------------------------------------------

int main ()
{
#if 0
    PRINT_SIZEOF_(XXX);
    PRINT_SIZEOF_(StateType);
    PRINT_SIZEOF_(bool);
    PRINT_SIZEOF_(coe::hsm::aState);
    PRINT_SIZEOF_(coe::hsm::BasicState);
    PRINT_SIZEOF_(coe::hsm::aComposition);
    PRINT_SIZEOF_(coe::hsm::OR_State);
    PRINT_SIZEOF_(coe::hsm::AND_State);
#endif

    if (::isatty(STDIN_FILENO))
        cout << "enter :h to get help." << endl;

    StateTrace          trace_funs =
        { &::get_trace_ctrl, &::print_action, &::print_machine };
    StateTrace::set_trace(trace_funs);

    Kernel& kernel = Kernel::create_new();

    kernel.select(STDIN_FILENO, IO_read, "on_command", vparam(string(), string()));
    kernel.state("on_command", handler(&::on_command));

    kernel.thread().run_event_loop();

    return 0;
}

