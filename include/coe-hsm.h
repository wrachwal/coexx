// coe-hsm.h

#ifndef __COE_HSM_H
#define __COE_HSM_H

#include "coe--local.h"

#include <string>
#include <iosfwd>   // ostream
#include <vector>

namespace coe {

class Kernel;

namespace hsm {

// ---------------------------------------------------------------------------

class Machine;
class aState;
    class BasicState;
    class aComposition;
        class  OR_State;
        class AND_State;

class StateVisitor;
struct StateTrace;

enum StateType {
    BASIC_STATE,
       OR_STATE,
      AND_STATE
};

enum HistoryType {
       NONE_HISTORY = 0,
    SHALLOW_HISTORY = 1,
       DEEP_HISTORY = 2
};

struct _MachineExecutor;

// ===========================================================================
// aState

template<class, class> class aComposition_Policy;

class aState {
public:
    virtual ~aState ();
    virtual const char* state_name () const = 0;

    // startup/cleanup phase
    virtual void on__startup (Kernel& kernel);
    virtual void on__cleanup (Kernel& kernel);

    // on entry/exit actions
    virtual void on__entry (Kernel& kernel);
    virtual void on__exit  (Kernel& kernel);

    // predicate
    bool is_in () const { return 0 != _active; }

    // transitions
    void transit    (Kernel& kernel);
    void transit_ex (Kernel& kernel, aState& ex);

    // (internals)
    StateType       type_ () const { return static_cast<StateType>(_type); }
    aComposition&   root_ () const;
    aComposition* parent_ () const { return _parent; }
    aState*         next_ () const { return _next; }
    bool          target_ () const { return _target; }
    Machine&     machine_ () const { return _machine; }

protected:
    aState (StateType type,  Machine& machine);
    aState (StateType type,  OR_State& parent);
    aState (StateType type, AND_State& parent);

private:
    friend struct _MachineExecutor;
    friend class OR_State;
    template<class, class> friend class aComposition_Policy;
    unsigned char       _type;
    unsigned char       _histype;
    signed char         _active;    // (1)active (-1)mark-exit (-2)exec-exit (0)inactive
    bool                _target;
    Machine&            _machine;
    aComposition* const _parent;
    aState*             _next;
};

// ------------------------------------
// BasicState

class BasicState : public aState {
protected:
    friend struct _MachineExecutor;
    BasicState ( OR_State& parent);
    BasicState (AND_State& parent);
    ~BasicState ();
};

// ------------------------------------
// aComposition

class aComposition : public aState {
public:
    // (internals)
    aState* child_list_ () const { return _child_list; }

protected:
    aComposition (StateType type,  Machine& machine);
    aComposition (StateType type,  OR_State& parent);
    aComposition (StateType type, AND_State& parent);

private:
    friend struct _MachineExecutor;
    friend class aState;
    aState* _child_list;
};

// ------------------------------------
// OR_State

template<class, HistoryType> class OR_State_Policy;

class OR_State : public aComposition {
public:
    // (internals)
    HistoryType histype_ () const
        { return _histype <= 2 ? static_cast<HistoryType>(_histype) : NONE_HISTORY; }
    aState* history_child_ () const { return _active ? 0 : _active_child; }
    bool    history_child_ (aState* child);

protected:
    OR_State ( Machine& machine, HistoryType histype);
    OR_State ( OR_State& parent, HistoryType histype);
    OR_State (AND_State& parent, HistoryType histype);
    ~OR_State ();

private:
    friend class aState;
    friend struct _MachineExecutor;
    template<class, HistoryType> friend class OR_State_Policy;
    aState* _active_child;  // active or history state
    aState* _target_child;
};

// ------------------------------------
// AND_State

class AND_State : public aComposition {
protected:
    friend struct _MachineExecutor;
    AND_State ( Machine& machine);
    AND_State ( OR_State& parent);
    AND_State (AND_State& parent);
    ~AND_State ();
};

// ---------------------------------------------------------------------------
// StateTrace

struct StateTrace {

    enum What {
        ACTION = (1 << 0),
        STATUS = (1 << 1),
        ALL    = (ACTION | STATUS)
    };
    struct Ctrl {
        Ctrl () : flag(0), mask(ALL) {}
        int flag;
        int mask;
    };

    static void set_trace (const StateTrace& cbs);

    // ---

    enum Action { OnTr, OnEx, OnEn };
    typedef void (*Fun_Control) (Kernel& kernel,
                                 Machine& machine,
                                 Ctrl& /*output*/ctrl);
    typedef void (*Fun_Action)  (Kernel& kernel,
                                 Machine& machine,
                                 Action action,
                                 aState* st,
                                 aState* ex);
    typedef void (*Fun_Status)  (Kernel& kernel,
                                 Machine& machine);
    Fun_Control control;
    Fun_Action  action;
    Fun_Status  status;
};

// ===========================================================================
// Machine

class Machine {
public:
    Machine (aComposition& root);
    ~Machine ();

    void initialize ();                 // to be called in root state constructor
    void do_startup (Kernel& kernel);   // implicitly on first aState::transit...()
    void do_cleanup (Kernel& kernel);   // implicitly after terminate()

    // transition to termination state
    void terminate (Kernel& kernel);

    aComposition& root () { return _root; }
    bool is_in_stable_state () const;
    bool enter_stable_state (Kernel& kernel);
    int action_level () const { return _action_level; }

    // debug tracing (local)
    StateTrace::Ctrl get_trace_ctrl () const;
    void             set_trace_ctrl (const StateTrace::Ctrl& ctrl);

private:
    friend class aState;
    friend struct _MachineExecutor;
    void _complete_transition (Kernel&);
    aComposition&   _root;
    Kernel* _kernel;
    bool    _initialized;
    bool    _started;
    bool    _dirty_status;
    int     _active_count;
    int     _target_count;
    int     _action_level;  // exiting(<0), entering(>0)
    StateTrace          _trace;
    int                 _trace_bits;
    StateTrace::Ctrl    _trace_next;
};

// ===========================================================================
// StateVisitor

class StateVisitor {
public:
    virtual ~StateVisitor ();
    virtual void visit (BasicState& state) = 0;
    virtual void visit (  OR_State& state) = 0;
    virtual void visit ( AND_State& state) = 0;
};

// ------------------------------------
// walking states tree

void walk_top_down  (aState& state, StateVisitor& visitor);
void walk_bottom_up (aState& state, StateVisitor& visitor);

// ------------------------------------
// naming and printing

std::ostream& full_state_name (std::ostream& os, aState& state, const char* sep = ".");
std::string   full_state_name (                  aState& state, const char* sep = ".");

std::ostream& print_state_machine (std::ostream& os, Machine& machine,
                                   const std::string& prefix = std::string());

// ------------------------------------
// serialization

void save_machine_state (Machine& machine,
                         // to restore, transit to states first,
                         std::vector<aState*>& state_transit,
                         // then restore retained child_history_
                         std::vector<aState*>& child_history);

// ---------------------------------------------------------------------------
// aComposition_Policy<_Self, _Parent>

template<class _Self, class _Parent = _Self>
class aComposition_Policy {

    // local metafunctions
    struct this_context_imp {
        template<class Context, class State>
        static Context& apply (State& state) { return state; }
    };
    struct that_context_imp {
        template<class Context, class State>
        static Context& apply (State& state)
            { return static_cast<typename State::PARENT*>(state.parent_())->template context<Context>(); }
    };

public:
    typedef          _Self         SELF;
    typedef          _Parent       PARENT;
    typedef typename _Parent::ROOT ROOT;

    ROOT& root () { return static_cast<ROOT&>(static_cast<_Self&>(*this)._machine.root()); }

    template<class Context>
    Context& context ()
        {
            typedef typename meta::If_<meta::IsSame_<Context, SELF>::value,
                                       this_context_imp,
                                       that_context_imp>::type imp;
            return imp::template apply<Context>(static_cast<SELF&>(*this));
        }

protected:
    ~aComposition_Policy () {}
};

template<class _Self>
class aComposition_Policy<_Self, _Self> {
public:
    typedef _Self SELF;
    typedef _Self ROOT;

    template<class Context>
    Context& context ()
        {
            // Context can only be SELF; checked by double casts
            return static_cast<Context&>((static_cast<SELF&>(*this)));
        }

protected:
    ~aComposition_Policy () {}
};

// ---------------------------------------------------------------------------
// OR_State_Policy<_Self, histype>

template<class _Self, HistoryType histype>
class OR_State_Policy {
public:
    bool clear_history () { return static_cast<_Self&>(*this).history_child_(0); }
protected:
    ~OR_State_Policy () {}
};

template<class _Self>
class OR_State_Policy<_Self, NONE_HISTORY> {
protected:
    ~OR_State_Policy () {}
};

// ===========================================================================
// OR_State_<_Self, _Parent[, histype]>

template<class _Self, class _Parent = _Self, HistoryType histype = NONE_HISTORY>
class OR_State_
  : public OR_State_Policy<_Self, histype>,
    public  aComposition_Policy<_Self, _Parent> {
public:
    static const HistoryType HISTORY = histype;
protected:
    ~OR_State_ () {}
};

template<class _Self, HistoryType histype>
class OR_State_<_Self, _Self, histype>
  : public     OR_State_Policy<_Self, histype>,
    public aComposition_Policy<_Self, _Self> {
public:
    static const HistoryType HISTORY = histype;
    Machine& machine () { return _machine; }
protected:
    OR_State_ () : _machine(static_cast<_Self&>(*this)) {}
    ~OR_State_ () {}
private:
    Machine _machine;
};

// ------------------------------------

#define BEG__MACHINE_OR_STATE_(S, L_args_R)                                 \
        struct S : public coe::hsm::OR_State,                               \
                   public coe::hsm::OR_State_<S> {                          \
            S L_args_R;                                                     \
            const char* state_name () const;
#define BEG__MACHINE_OR_STATE_H_(S, L_args_R)                               \
        struct S : public coe::hsm::OR_State,                               \
                   public coe::hsm::OR_State_<S> {                          \
            S L_args_R;                                                     \
            const char* state_name () const;
#define END__MACHINE_OR_STATE_(s)                                           \
        };

#define BEG__OR_STATE_(S)                                                   \
        struct S : public coe::hsm::OR_State,                               \
                   public coe::hsm::OR_State_<S, SELF> {                    \
            S (PARENT& _p);                                                 \
            const char* state_name () const;
#define BEG__OR_STATE_H_(S, H)                                              \
        struct S : public coe::hsm::OR_State,                               \
                   public coe::hsm::OR_State_<S, SELF, coe::hsm::H> {       \
            S (PARENT& _p);                                                 \
            const char* state_name () const;
#define END__OR_STATE_(s)                                                   \
        } s;

// ------------------------------------

#define DEF__MACHINE_OR_STATE_(Pp, S, L_args_R)                             \
        const char* Pp S::state_name () const { return #S; }                \
        Pp S::S L_args_R : coe::hsm::OR_State(machine(), HISTORY)

#define DEF__OR_STATE_(Pp, S)                                               \
        const char* Pp S::state_name () const { return #S; }                \
        Pp S::S (PARENT& _p) : coe::hsm::OR_State(_p, HISTORY)

// ---------------------------------------------------------------------------
// AND_State_<_Self, _Parent>

template<class _Self, class _Parent = _Self>
class AND_State_
  : public aComposition_Policy<_Self, _Parent> {
protected:
    ~AND_State_ () {}
};

template<class _Self>
class AND_State_<_Self, _Self>
  : public aComposition_Policy<_Self, _Self> {
public:
    Machine& machine () { return _machine; }
protected:
    AND_State_ () : _machine(static_cast<_Self&>(*this)) {}
    ~AND_State_ () {}
private:
    Machine _machine;
};

// ------------------------------------

#define BEG__AND_STATE_(S)                                                  \
        struct S : public coe::hsm::AND_State,                              \
                   public coe::hsm::AND_State_<S, SELF> {                   \
            S (PARENT& _p);                                                 \
            const char* state_name () const;
#define END__AND_STATE_(s)                                                  \
        } s;

// ------------------------------------

#define DEF__AND_STATE_(Pp, S)                                              \
        const char* Pp S::state_name () const { return #S; }                \
        Pp S::S (PARENT& _p) : coe::hsm::AND_State(_p)

// ---------------------------------------------------------------------------
// BasicState_<_Self, _Parent>

template<class _Self, class _Parent>
class BasicState_
  : public aComposition_Policy<_Self, _Parent> {
protected:
    ~BasicState_ () {}
};

// ------------------------------------

#define BEG__STATE_(S)                                                      \
        struct S : public coe::hsm::BasicState,                             \
                   public coe::hsm::BasicState_<S, SELF> {                  \
            S (PARENT& _p);                                                 \
            const char* state_name () const;
#define END__STATE_(s)                                                      \
        } s;

// ------------------------------------

#define DEF__STATE_(Pp, S)                                                  \
        const char* Pp S::state_name () const { return #S; }                \
        Pp S::S (PARENT& _p) : coe::hsm::BasicState(_p)

// ---------------------------------------------------------------------------

#define ON_EN       void on__entry (coe::Kernel& kernel);
#define ON_EX       void on__exit  (coe::Kernel& kernel);
#define ON_ENEX     ON_EN ON_EX

// ===========================================================================

} ///// namespace hsm
} ///// namespace coe

#endif

