// coe-hsm.cpp

#include "coe-hsm.h"

#include <cstdlib>  // NULL
#include <ostream>
#include <sstream>
#include <cassert>

namespace coe {
namespace hsm {

using namespace std;

// ===========================================================================
// aState

aState::aState (StateType type, Machine& machine)
:   _type(type)
,   _active(0)
,   _target(false)
,   _machine(machine)
,   _parent(NULL)
,   _next(NULL)
{
}

aState::aState (StateType type, OR_State& parent)
:   _type(type)
,   _active(0)
,   _target(false)
,   _machine(parent._machine)
,   _parent(&parent)
,   _next(parent._child_list)
{
    parent._child_list = this;
}

aState::aState (StateType type, AND_State& parent)
:   _type(type)
,   _active(0)
,   _target(false)
,   _machine(parent._machine)
,   _parent(&parent)
,   _next(parent._child_list)
{
    parent._child_list = this;
}

aState::~aState ()
{
}

aComposition& aState::root_ () const
{
    return _machine.root();
}

void aState::on__startup (Kernel& kernel)
{
}

void aState::on__cleanup (Kernel& kernel)
{
}

void aState::on__entry (Kernel& kernel)
{
}

void aState::on__exit (Kernel& kernel)
{
}

// ===========================================================================
// BasicState

BasicState::BasicState (OR_State& parent)
:   aState(BASIC_STATE, parent)
{
}

BasicState::BasicState (AND_State& parent)
:   aState(BASIC_STATE, parent)
{
}

BasicState::~BasicState ()
{
}

// ===========================================================================
// aComposition

aComposition::aComposition (StateType type, Machine& machine)
:   aState(type, machine)
,   _child_list(NULL)
{
}

aComposition::aComposition (StateType type, OR_State& parent)
:   aState(type, parent)
,   _child_list(NULL)
{
}

aComposition::aComposition (StateType type, AND_State& parent)
:   aState(type, parent)
,   _child_list(NULL)
{
}

// ===========================================================================
// OR_State

OR_State::OR_State (Machine& machine, HistoryType histype)
:   aComposition(OR_STATE, machine)
,   _active_child(NULL)
,   _target_child(NULL)
{
    _histype = histype;
}

OR_State::OR_State (OR_State& parent, HistoryType histype)
:   aComposition(OR_STATE, parent)
,   _active_child(NULL)
,   _target_child(NULL)
{
    if ((_histype = histype) == NONE_HISTORY) {
        // from parent which is the xor-state,
        // inherit history type if it's deep
        if (parent._histype & DEEP_HISTORY) {
            _histype = (SHALLOW_HISTORY | DEEP_HISTORY);
        }
    }
}

OR_State::OR_State (AND_State& parent, HistoryType histype)
:   aComposition(OR_STATE, parent)
,   _active_child(NULL)
,   _target_child(NULL)
{
    if ((_histype = histype) == NONE_HISTORY) {
        // find first ancestor xor-state
        for (aComposition* p = parent._parent; NULL != p; p = p->_parent) {
            if (p->_type == OR_STATE) {
                // and inherit its history type if it's deep
                if (static_cast<OR_State*>(p)->_histype & DEEP_HISTORY)
                    _histype = (SHALLOW_HISTORY | DEEP_HISTORY);
                break;
            }
        }
    }
}

OR_State::~OR_State ()
{
}

// ===========================================================================
// AND_State

AND_State::AND_State (Machine& machine)
:   aComposition(AND_STATE, machine)
{
}

AND_State::AND_State (OR_State& parent)
:   aComposition(AND_STATE, parent)
{
}

AND_State::AND_State (AND_State& parent)
:   aComposition(AND_STATE, parent)
{
}

AND_State::~AND_State ()
{
}

// ===========================================================================
// Machine

Machine::Machine (aComposition& root)
:   _root(root)
,   _kernel(NULL)
,   _initialized(false)
,   _started(false)
,   _dirty_status(false)
,   _active_count(0)
,   _target_count(0)
,   _action_level(0)
,   _trace_bits(0)
{
}

Machine::~Machine ()
{
}

void Machine::do_startup (Kernel& kernel)
{
    struct StateStarter : public StateVisitor {
        StateStarter (Machine& m, Kernel& k) : _machine(m), _kernel(k) {}
        void visit (BasicState& state) { startup_state(state); }
        void visit (  OR_State& state) { startup_state(state); }
        void visit ( AND_State& state) { startup_state(state); }
        void startup_state (aState& state)
            {
                assert(_machine._started);
                state.on__startup(_kernel);
            }
        Machine&    _machine;
        Kernel&     _kernel;
    };

    if (! _started) {
        initialize();   // just in case
        _started = true;
        StateStarter    starter(*this, kernel);
        walk_top_down(_root, starter);
    }
}

void Machine::do_cleanup (Kernel& kernel)
{
    struct StateCleaner : public StateVisitor {
        StateCleaner (Machine& m, Kernel& k) : _machine(m), _kernel(k) {}
        void visit (BasicState& state) { cleanup_state(state); }
        void visit (  OR_State& state) { cleanup_state(state); }
        void visit ( AND_State& state) { cleanup_state(state); }
        void cleanup_state (aState& state)
            {
                assert(! _machine._started);
                state.on__cleanup(_kernel);
            }
        Machine&    _machine;
        Kernel&     _kernel;
    };

    if (_started) {
        _started = false;
        StateCleaner    cleaner(*this, kernel);
        walk_bottom_up(_root, cleaner);
    }
}

bool Machine::is_in_stable_state () const
{
    assert(_active_count <= _target_count);
    return _active_count == _target_count;
}

// ===========================================================================
// StateVisitor

StateVisitor::~StateVisitor ()
{
}

// ---------------------------------------------------------------------------

void walk_top_down (aState& state, StateVisitor& visitor)
{
    switch (state.type_()) {
        case AND_STATE:
            {
                AND_State&  ast = static_cast<AND_State&>(state);
                visitor.visit(ast);
                for (aState* child = ast.child_list_();
                     NULL != child;
                     child = child->next_())
                {
                    walk_top_down(*child, visitor);
                }
            }
            break;
        case OR_STATE:
            if (NULL != static_cast<aComposition&>(state).child_list_()) {
                OR_State&   xst = static_cast<OR_State&>(state);
                visitor.visit(xst);
                for (aState* child = xst.child_list_();
                     NULL != child;
                     child = child->next_())
                {
                    walk_top_down(*child, visitor);
                }
                break;
            }
            /* pass */
        case BASIC_STATE:
            visitor.visit(static_cast<BasicState&>(state));
            break;
    }
}

// ------------------------------------

void walk_bottom_up (aState& state, StateVisitor& visitor)
{
    switch (state.type_()) {
        case AND_STATE:
            {
                AND_State&  ast = static_cast<AND_State&>(state);
                for (aState* child = ast.child_list_();
                     NULL != child;
                     child = child->next_())
                {
                    walk_bottom_up(*child, visitor);
                }
                visitor.visit(ast);
            }
            break;
        case OR_STATE:
            if (NULL != static_cast<aComposition&>(state).child_list_()) {
                OR_State&   xst = static_cast<OR_State&>(state);
                for (aState* child = xst.child_list_();
                     NULL != child;
                     child = child->next_())
                {
                    walk_bottom_up(*child, visitor);
                }
                visitor.visit(xst);
                break;
            }
            /* pass */
        case BASIC_STATE:
            visitor.visit(static_cast<BasicState&>(state));
            break;
    }
}

// ---------------------------------------------------------------------------

ostream& full_state_name (ostream& os, aState& state, const char* sep)
{
    // here recursion much more effective than iteration
    if (state.parent_()) {
        full_state_name(os, *state.parent_(), sep);
        os << sep;
    }
    return os << state.state_name();
}

string full_state_name (aState& state, const char* sep)
{
    ostringstream   oss;
    full_state_name(oss, state, sep);
    return oss.str();
}

// ---------------------------------------------------------------------------

namespace {

    class StatePrinter : public StateVisitor {
    public:
        explicit StatePrinter (ostream& os, const string& bl = "", const char* nl = "\n")
            : _os(os), _bl(bl), _nl(nl) {}
        void visit (BasicState& state);
        void visit (  OR_State& state);
        void visit ( AND_State& state);
    private:
        void _state_status (aState& state);
        ostream&    _os;
        string      _bl;
        string      _nl;
    };

}   /// namespace

// ------------------------------------

void StatePrinter::_state_status (aState& state)
{
    _os << (state.is_in()                                              ? '@' : ' ')
        << (! state.machine_().is_in_stable_state() && state.target_() ? '*' : ' ');
}

void StatePrinter::visit (BasicState& state)
{
    _os << _bl;
    _state_status(state);
    _os << "- ";
    full_state_name(_os, state, ".");
    _os << _nl;
}

void StatePrinter::visit (OR_State& state)
{
    _os << _bl;
    _state_status(state);
    switch (state.histype_()) {
        default:                _os << "| ";    break;
        case SHALLOW_HISTORY:   _os << "h ";    break;
        case DEEP_HISTORY:      _os << "H ";    break;
    }
    full_state_name(_os, state, ".");
    _os << _nl;
}

void StatePrinter::visit (AND_State& state)
{
    _os << _bl;
    _state_status(state);
    _os << "& ";
    full_state_name(_os, state, ".");
    _os << _nl;
}

// ------------------------------------

ostream& print_state_machine (ostream& os, Machine& machine, const string& prefix)
{
    StatePrinter    printer(os, prefix);
    walk_top_down(machine.root(), printer);
    return os;
}

// ===========================================================================

} ///// namespace hsm
} ///// namespace coe

