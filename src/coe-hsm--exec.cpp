// coe-hsm--exec.cpp

#include "coe-hsm.h"

#include "coe-kernel.h"
#include "coe-sys-sync.h"   // Mutex

#include <cstdlib>          // abs

#include <iostream>
#include <cassert>

namespace coe {
namespace hsm {

using namespace std;

// ---------------------------------------------------------------------------

static unsigned state_level (aState* state)
{
    unsigned    level = 0;
    while (NULL != state->parent_()) {
        ++level;
        state = state->parent_();
    }
    return level;
}

static aState* ancestor_state (aState* state, unsigned advance)
{
    while (advance--)
        state = state->parent_();
    return state;
}

static aState* common_ancestor (aState* s1, aState* s2)
{
    if (s1 == s2)
        return s1;
    unsigned    n1 = state_level(s1);
    unsigned    n2 = state_level(s2);
    if      (n1 > n2)   s1 = ancestor_state(s1, n1 - n2);
    else if (n1 < n2)   s2 = ancestor_state(s2, n2 = n1);
    while (s1 != s2) {
        s1 = s1->parent_();
        s2 = s2->parent_();
    }
    return s1;
}

// ===========================================================================
// StateTrace (global state & its API)

static Mutex            s_Trace_Mutex;
static StateTrace       s_Trace;
static StateTrace::Ctrl s_Trace_Ctrl;

void StateTrace::set_trace (const StateTrace& cbs)
{
    // --@@--
    Mutex::Guard    lock(s_Trace_Mutex);
    s_Trace = cbs;
}

StateTrace::Ctrl StateTrace::get_trace_ctrl ()
{
    // --@@--
    Mutex::Guard    lock(s_Trace_Mutex);
    Ctrl    ctrl = s_Trace_Ctrl;
    return ctrl;
}

void StateTrace::set_trace_ctrl (const Ctrl& ctrl)
{
    // --@@--
    Mutex::Guard    lock(s_Trace_Mutex);
    s_Trace_Ctrl = ctrl;
}

// ===========================================================================
// _MachineExecutor

struct _MachineExecutor {

    static void initialize_state (aState& state);

    static Kernel* set_kernel_context (Machine& sm, Kernel* kernel);

    static void debug_trace_setup (Machine& sm);
    static void debug_trace_action (Machine& sm, StateTrace::Action action,
                                    aState* to, aState* ex);
    static void debug_trace_status (Machine& sm);

    static void set_target_flag (aState& state, bool flag);
    static void set_active_flag (aState& state, signed char flag);

    static void clear_children_deep_history (aComposition& parent);
    static void clear_deep_history (aState& state);

    static void state_transition (Machine& sm, aState* to, aState* ex);
    static void clear_target_tree (aState& state);
    static void prepare_target_path (aState* state);
    static void exit_state (aState& state);

    static void clear_exited_state (aState& state);
    static void enter_target_state (aState& state);
};

// ---------------------------------------------------------------------------

void _MachineExecutor::initialize_state (aState& state)
{
    if (state._type != BASIC_STATE) {
        aComposition&   parent = static_cast<aComposition&>(state);
        if (NULL == parent._child_list) {
            state._type = BASIC_STATE;      // hardwire
        }
        else {
            // reverse child list
            aState* prev = NULL;
            aState* node = parent._child_list;
            while (node != NULL) {
                initialize_state(*node);    //--
                aState* next = node->_next;
                node->_next = prev;
                prev = node;
                node = next;
            }
            parent._child_list = prev;
        }
    }
}

inline Kernel* _MachineExecutor::set_kernel_context (Machine& sm, Kernel* kernel)
{
    Kernel* tmp = sm._kernel;
    sm._kernel = kernel;
    return tmp;
}

void _MachineExecutor::debug_trace_setup (Machine& sm)
{
    StateTrace::Ctrl    global;

    // --@@--
    {
        Mutex::Guard    lock(s_Trace_Mutex);
        sm._trace = s_Trace;
        global    = s_Trace_Ctrl;
    }

    // determine what to trace taking global & machine settings

    sm._trace_bits = (global.flag & sm._trace_next.mask)
                   | (global.mask & sm._trace_next.flag)
                   ;

    // clear bits for unset callbacks

    if (NULL == sm._trace.action) {
        sm._trace_bits &= ~StateTrace::ACTION;
    }
    if (NULL == sm._trace.status) {
        sm._trace_bits &= ~StateTrace::STATUS;
    }
}

void _MachineExecutor::debug_trace_action (Machine& sm, StateTrace::Action action,
                                           aState* to, aState* ex)
{
    if (sm._trace_bits & StateTrace::ACTION) {
        assert(NULL != sm._kernel);
        sm._trace.action(*sm._kernel, sm, action, to, ex);
    }

}

void _MachineExecutor::debug_trace_status (Machine& sm)
{
    if (sm._dirty_status) {
        sm._dirty_status = false;
        if (sm._trace_bits & StateTrace::STATUS) {
            assert(NULL != sm._kernel);
            sm._trace.status(*sm._kernel, sm);
        }
    }
}

void _MachineExecutor::set_target_flag (aState& state, bool flag)
{
    assert(state._target != flag);

    state._machine._target_count += (flag ? +1 : -1);
    state._machine._dirty_status = true;
    state._target = flag;
}

void _MachineExecutor::set_active_flag (aState& state, signed char flag)
{
    assert(flag >= -2 && flag <= 1);
    assert(state._active != flag);

    if ((state._active > 0) != (flag > 0)) {
        state._machine._active_count += (flag > 0 ? +1 : -1);
        state._machine._dirty_status = true;
    }
    state._active = flag;
}

// ---------------------------------------------------------------------------

void _MachineExecutor::state_transition (Machine& sm, aState* to, aState* ex)
{
    int level = sm._action_level;
    sm._action_level = (level <= 0 ? level - 1 : level + 1);

    debug_trace_action(sm, StateTrace::OnTr, to, ex);

    if (NULL != to) {
        if (NULL != ex) {
            assert(NULL != to);
            assert(&to->_machine == &ex->_machine);
            aState*  a_to = to->_active > 0 ? to : NULL;
            aState*  a_ex = ex->_active > 0 ? ex : NULL;
            ex = a_to && a_ex ? common_ancestor(a_to, a_ex)
                              : a_to ? a_to
                                     : a_ex;
        }
        else if (to->_active > 0) {
            ex = to;
        }
        if (NULL != ex) {
            assert(ex->_target);
            clear_target_tree(*ex);
            set_target_flag(*ex, true);
            exit_state(*ex);
        }
        prepare_target_path(to);
    }
    else {
        clear_target_tree(sm.root());
        exit_state(sm.root());
    }

    sm._action_level = level;
}

void _MachineExecutor::clear_target_tree (aState& state)
{
    if (state._target) {

        set_target_flag(state, false);

        if (state._type == OR_STATE) {
            OR_State&   xst = static_cast<OR_State&>(state);
            if (NULL != xst._target_child) {
                clear_target_tree(*xst._target_child);
                xst._target_child = NULL;
            }
        }
        else if (state._type == AND_STATE) {
            AND_State&  ast = static_cast<AND_State&>(state);
            for (aState* child = ast._child_list; NULL != child; child = child->_next) {
                clear_target_tree(*child);
            }
        }
    }
}

void _MachineExecutor::prepare_target_path (aState* state)
{
    if (state->_target) {

        exit_state(*state);
    }
    else {

        set_target_flag(*state, true);

        for (aState* parent = state->_parent;
            NULL != parent;
            state = parent, parent = parent->_parent)
        {
            if (parent->_target) {

                if (parent->_type == OR_STATE) {
                    OR_State*   xst = static_cast<OR_State*>(parent);

                    if (NULL != xst->_target_child) {

                        assert(xst->_target_child != state);
                        assert(xst->_target_child->_target);

                        clear_target_tree(*xst->_target_child);
                        xst->_target_child = state;

                        if (NULL != xst->_active_child) {
                            exit_state(*xst->_active_child);
                        }
                    }
                    else {
                        xst->_target_child = state;
                    }
                }
                break;  ////
            }
            else {
                set_target_flag(*parent,  true);

                if (parent->_type == OR_STATE) {
                    OR_State*   xst = static_cast<OR_State*>(parent);
                    assert(NULL == xst->_target_child);
                    xst->_target_child = state;
                }
            }
        }
    }
}

void _MachineExecutor::exit_state (aState& state)
{
    switch (state._active) {

        case 1:
            set_active_flag(state, -1);

        case -1:
            if (state._type == OR_STATE) {
                OR_State&   xst = static_cast<OR_State&>(state);
                if (NULL != xst._active_child) {
                    exit_state(*xst._active_child);     //--
                }
            }
            else if (state._type == AND_STATE) {
                AND_State&  ast = static_cast<AND_State&>(state);
                for (aState* child = ast._child_list; child != NULL; child = child->_next) {
                    exit_state(*child);                 //--
                }
            }

            if (state._active == -1) {

                set_active_flag(state, -2);

                Machine&    sm = state._machine;
                debug_trace_action(sm, StateTrace::OnEx, &state, NULL);

                assert(NULL != sm._kernel);
                state.on_ex(*sm._kernel);               //**
            }

            if (state._active == -2) {
        case -2:
                if (NULL == state._parent || state._parent->_active < 0) {
                    set_active_flag(state, 0);
                }
            }

        default:    // 0
            return;
    }
}

// ---------------------------------------------------------------------------

void _MachineExecutor::clear_exited_state (aState& state)
{
    switch (state._active) {

        case 1:
            if (state._type == OR_STATE) {
                OR_State&   xst = static_cast<OR_State&>(state);
                if (NULL != xst._active_child) {
                    clear_exited_state(*xst._active_child);     //--
                }
            }
            else if (state._type == AND_STATE) {
                AND_State&  ast = static_cast<AND_State&>(state);
                for (aState* child = ast._child_list;
                     child != NULL;
                     child = child->_next)
                {
                    clear_exited_state(*child);                 //--
                }
            }
            break;

        case -2:
            set_active_flag(state, 0);
            break;

        default:
            assert(state._active != -1);
            break;
    }
}

void _MachineExecutor::enter_target_state (aState& state)
{
    assert(state._target);

    if (state._active != 1) {

        set_active_flag(state, 1);

        Machine&    sm = state._machine;
        int level = sm._action_level;
        assert(level >= 0);
        sm._action_level = level + 1;

        debug_trace_action(sm, StateTrace::OnEn, &state, NULL);

        assert(NULL != sm._kernel);
        state.on_en(*sm._kernel);                   //**

        sm._action_level = level;

        if (state._active != 1) {
            return;
        }
    }

    if (state._type == OR_STATE) {

        OR_State&   xst = static_cast<OR_State&>(state);

        if (NULL == xst._target_child) {

            xst._target_child =
                (xst._histype == NONE_HISTORY || NULL == xst._active_child)
                    ? xst._child_list
                    : xst._active_child
                    ;

            assert(! xst._target_child->_target);

            set_target_flag(*xst._target_child, true);
        }
        else {
            assert(xst._target_child->_target);
        }

        xst._active_child = xst._target_child;

        enter_target_state(*xst._active_child);     //--
    }
    else if (state._type == AND_STATE) {

        AND_State&  ast = static_cast<AND_State&>(state);

        for (aState* child = ast._child_list; child != NULL; child = child->_next) {

            if (! child->_target) {
                set_target_flag(*child, true);
            }

            enter_target_state(*child);             //--

            if (state._active != 1) {
                return;
            }
        }
    }
}

// ---------------------------------------------------------------------------

void _MachineExecutor::clear_children_deep_history (aComposition& parent)
{
    for (aState* child = parent._child_list; NULL != child; child = child->_next) {
        _MachineExecutor::clear_deep_history(*child);                   //--
    }
}

void _MachineExecutor::clear_deep_history (aState& state)
{
    assert(0 == state._active);

    //walk recursively up to leaves or nested xor-states with their own history
    //specified and clear _active_child in all xor-states visited.

    if (state._type == OR_STATE) {
        OR_State&   xst = static_cast<OR_State&>(state);
        if (xst._histype == (SHALLOW_HISTORY | DEEP_HISTORY)) {
            if (NULL != xst._active_child) {
                clear_deep_history(*xst._active_child);                 //--
                xst._active_child = NULL;
            }
        }
    }
    else if (state._type == AND_STATE) {
        clear_children_deep_history(static_cast<AND_State&>(state));    //--
    }
}

// ===========================================================================
// Machine

void Machine::initialize ()
{
    if (! _initialized) {
        _initialized = true;
        _MachineExecutor::initialize_state(_root);
    }
}

void Machine::terminate (Kernel& kernel)
{
    if (is_in_stable_state()) {
        _MachineExecutor::debug_trace_setup(*this);
    }

    Kernel* tmp = _MachineExecutor::set_kernel_context(*this, &kernel);

    _MachineExecutor::state_transition(*this, NULL, NULL);

    bool    terminated = false;

    if (is_in_stable_state()) {
        assert(! _root.is_in());
        terminated = true;
    }
    else {
        _MachineExecutor::debug_trace_status(*this);
        kernel.continuation(handler(*this, &Machine::_complete_transition));
    }
    _MachineExecutor::set_kernel_context(*this, tmp);

    if (terminated) {
        do_cleanup(kernel);
    }
}

bool Machine::enter_stable_state (Kernel& kernel)
{
    if (! is_in_stable_state() && 0 == _action_level) {
        Kernel* tmp = _MachineExecutor::set_kernel_context(*this, &kernel);
        _MachineExecutor::clear_exited_state(_root);
        _MachineExecutor::enter_target_state(_root);
        _MachineExecutor::set_kernel_context(*this, tmp);
    }
    return is_in_stable_state();
}

void Machine::_complete_transition (Kernel& kernel)
{
    assert(0 == _action_level);

    Kernel* tmp = _MachineExecutor::set_kernel_context(*this, &kernel);
    _MachineExecutor::debug_trace_status(*this);

    bool    terminated = false;

    if (enter_stable_state(kernel)) {
        _MachineExecutor::debug_trace_status(*this);
        terminated = ! _root.is_in();
    }
    else {
        kernel.continuation(handler(*this, &Machine::_complete_transition));
    }
    _MachineExecutor::set_kernel_context(*this, tmp);

    if (terminated) {
        do_cleanup(kernel);
    }
}

// ===========================================================================
// aState

void aState::transition_to (Kernel& kernel)
{
    if (_machine.is_in_stable_state()) {
        _MachineExecutor::debug_trace_setup(_machine);
    }

    _machine.do_startup(kernel);

    Kernel* tmp = _MachineExecutor::set_kernel_context(_machine, &kernel);

    _MachineExecutor::state_transition(_machine, this, NULL);

    if (! _machine.is_in_stable_state()) {
        _MachineExecutor::debug_trace_status(_machine);
        kernel.continuation(handler(_machine, &Machine::_complete_transition));
    }
    _MachineExecutor::set_kernel_context(_machine, tmp);
}

void aState::transition_to_ex (Kernel& kernel, aState& ex)
{
    if (_machine.is_in_stable_state()) {
        _MachineExecutor::debug_trace_setup(_machine);
    }

    _machine.do_startup(kernel);

    Kernel* tmp = _MachineExecutor::set_kernel_context(_machine, &kernel);

    _MachineExecutor::state_transition(_machine, this, &ex);

    if (! _machine.is_in_stable_state()) {
        _MachineExecutor::debug_trace_status(_machine);
        kernel.continuation(handler(_machine, &Machine::_complete_transition));
    }
    _MachineExecutor::set_kernel_context(_machine, tmp);
}

// ===========================================================================
// OR_State

bool OR_State::_clear_history ()
{
    if (0 == _active) {

        switch (_histype) {

            case SHALLOW_HISTORY:
                _active_child = NULL;
                return true;

            case DEEP_HISTORY:
                _MachineExecutor::clear_children_deep_history(*this);
                _active_child = NULL;
                return true;

            case (SHALLOW_HISTORY | DEEP_HISTORY):
                _MachineExecutor::clear_children_deep_history(*this);
                return true;
        }
    }

    return false;
}

// ===========================================================================

} ///// namespace hsm
} ///// namespace coe

