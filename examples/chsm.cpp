// chsm.cpp

#include "coe-kernel.h"
#include "coe-session.h"

#include <string>
#include <iostream>

using namespace std;
using namespace coe;

ostream&    log = cout;

namespace coe { string demangle (const type_info&); }   // coe--util.h

// ===========================================================================
// meta info

enum eSTATE { eBS = 0, eOS = 1, eAS = 2 };

enum History {
    NO_HISTORY          = 0,
    SHALLOW_HISTORY     = 1,
    DEEP_HISTORY        = 2,
    INHERITED_HISTORY   = 3
};

struct mSM; // of machine<>
struct mXS; // of ...state<>
struct mCS; // of "composition"_state<>
struct mOS; // of or_state<>
struct mAS; // of and_state<>
struct mBS; // of "basic"state<>

struct mSM : private _Noncopyable {
    const mXS*          root;
    const type_info*    info;
    string              name;
    ostream& print (ostream& os) const
        {
            return os << this << "=mSM(root=" << root << ")";
        }
};

struct mXS : private _Noncopyable {
    virtual ~mXS () {}
    virtual eSTATE type () const = 0;
    string path () const;
    virtual ostream& print (ostream& os) const = 0;
    ostream& _print (ostream& os) const
        {
            return os << "mXS(sm=" << sm << " par=" << par
                << " size=" << size
                << " put=" << (void*)put
                << " clr=" << (void*)clr
            << ")";
        }
    const mSM*          sm;
    const mCS*          par;
    const mXS*          next;
    int                 level;
    size_t              size;
    const type_info*    info;
    string              name;
    void (*put)(void*);
    void (*clr)(void*);
};

struct mCS : mXS {

    //XXX IMPORTANT: Meta objects are initialized in more or less unpredictable
    //order. So, if a meta object of a child state is initalized first, it's
    //crucial to preserve what that child has setup in 'chld' of its (not yet
    //initialized) parent. Therefore, esp. the 'chld' field cannot be
    //initialized in the constructor (if defined). Fortunately, all data members
    //of static objects are initially zero, so are pointers!

    ostream& _print (ostream& os) const
        {
            mXS::_print(os << "mCS(") << " chld=[";
            for (const mXS* xs = chld; xs; xs = xs->next)
                os << (xs == chld ? "" : " ") << xs;
            return os << "])";
        }
    const mXS*  chld;
};

struct mOS : mCS {
    eSTATE type () const { return eOS; }
    ostream& print (ostream& os) const
        {
            static const char* hs[] = { "no", "H", "H*", "h*" };
            return mCS::_print(os << this << "=mOS(")
                << " init=" << init << " hist=" << hs[hist] << ")";
        }
    const mXS*  init;
    History     hist;
};

struct mAS : mCS {
    eSTATE type () const { return eAS; }
    ostream& print (ostream& os) const
        {
            return mCS::_print(os << this << "=mAS(") << ")";
        }
};

struct mBS : mXS {
    eSTATE type () const { return eBS; }
    ostream& print (ostream& os) const
        {
            return mXS::_print(os << this << "=mBS(") << ")";
        }
};

// ------------------------------------

string mXS::path () const { return par ? par->path() + '.' + name : name; }

// ------------------------------------

ostream& operator<< (ostream& os, const mSM& sm) { return sm.print(os); }
ostream& operator<< (ostream& os, const mXS& xs) { return xs.print(os); }

// ===========================================================================
// state_path<State>

template<class State, int Level = State::LEVEL>
struct state_path {
    typedef Cons<State, typename state_path<typename State::PARENT>::type> type;
};

template<class Root>
struct state_path<Root, 0> {
    typedef Cons<Root, Nil> type;
};

// ===========================================================================
// aComposition_Policy<..>

template<class _Self, class _Parent, class _Meta = typename _Parent::META>
class aComposition_Policy {

    // local metafunctions
    struct this_context_imp {
        template<class Context, class State>
        static Context& apply (State& state) { return state; }
    };
    struct that_context_imp {
        template<class Context, class State>
        static Context& apply (State& state)
#if 0
            { return static_cast<typename State::PARENT*>(state.parent_())->template context<Context>(); }
#else
            ;
#endif
    };

public:
    enum { LEVEL = (_Parent::LEVEL + 1) };
    typedef          _Self              SELF;
    typedef          _Parent            PARENT;
    typedef typename _Parent::ROOT      ROOT;
    typedef typename _Parent::MACHINE   MACHINE;

    //FIXME
#if 0
    ROOT& root () { return static_cast<ROOT&>(static_cast<_Self&>(*this)._machine.root()); }
#endif

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

template<class _Self, class _Parent>
class aComposition_Policy<_Self, _Parent, mSM> {
public:
    enum { LEVEL = 0 };
    typedef _Self   SELF;
    typedef _Self   ROOT;
    typedef _Parent PARENT;
    typedef _Parent MACHINE;

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
// HistoryPolicy<..>


template<class _OrState, History _history = History(_OrState::_HISTORY_TYPE)>
struct InheritHistoryPolicy {
    enum { _HISTORY_TYPE = _history };
};
template<class _OrState>
struct InheritHistoryPolicy<_OrState, SHALLOW_HISTORY> {
    enum { _HISTORY_TYPE = NO_HISTORY };
};
template<class _OrState>
struct InheritHistoryPolicy<_OrState, DEEP_HISTORY> {
    typedef _OrState HISTORY_ROOT;
    enum { _HISTORY_TYPE = INHERITED_HISTORY };
};
template<class _OrState>
struct InheritHistoryPolicy<_OrState, INHERITED_HISTORY> {
    typedef typename _OrState::HISTORY_ROOT HISTORY_ROOT;
    enum { _HISTORY_TYPE = INHERITED_HISTORY };
};

// ---

template<class _Parent, class _Meta = typename _Parent::META>
struct ParentHistoryPolicy : ParentHistoryPolicy<typename _Parent::PARENT> {};
template<class _Parent>
struct ParentHistoryPolicy<_Parent, mOS> : InheritHistoryPolicy<_Parent> {};
template<class _Parent>
struct ParentHistoryPolicy<_Parent, mSM> {
    enum { _HISTORY_TYPE = NO_HISTORY };
};

// ---

template<class _Self, class _Parent, History _history>
struct HistoryPolicy {
    typedef _Self HISTORY_ROOT;
    enum { _HISTORY_TYPE = _history };
};
template<class _Self, class _Parent>
struct HistoryPolicy<_Self, _Parent, NO_HISTORY> : ParentHistoryPolicy<_Parent> {};
template<class _Self, class _Parent>
struct HistoryPolicy<_Self, _Parent, INHERITED_HISTORY> : ParentHistoryPolicy<_Parent> {};

// ===========================================================================
// machine<>

template<class _Self, class _Root>
class machine {
public:
    typedef _Root ROOT;
    machine ()
        {
            log << __FUNCTION__ << endl;
        }
    typedef mSM META;
    const META& meta () const { return Rtti<META, _Self>::meta()->info; }
};

// ------------------------------------
// any_state

class any_state {
public:
    virtual ~any_state () {}
    virtual const mXS& meta () const = 0;
};

// ------------------------------------
// or_state<>

template<class _Self, class _Parent, class _Init, History _history = NO_HISTORY>
class or_state
    :   public any_state
    ,   public aComposition_Policy<_Self, _Parent>
    ,   public HistoryPolicy<_Self, _Parent, _history> {
public:
    typedef _Init INIT;
    or_state ()
        {
            log << __FUNCTION__ << endl;
        }
    typedef mOS META;
    const META& meta () const { return Rtti<META, _Self>::meta()->info; }

private:
    template<class, class> friend struct init_meta_info;
    static void constructor (void* ptr) //TODO: (Kernel&) ???
        {
            new (ptr) _Self;
        }
    static void destructor (void* ptr)
        {
            static_cast<_Self*>(ptr)->~_Self();
        }
};

// ------------------------------------
// and_state<>

template<class _Self, class _Parent, class _InitList>
class and_state
    :   public any_state
    ,   public aComposition_Policy<_Self, _Parent> {
public:
    typedef _InitList INIT_LIST;
    and_state ()
        {
            log << __FUNCTION__ << endl;
        }
    typedef mAS META;
    const META& meta () const { return Rtti<META, _Self>::meta()->info; }

private:
    template<class, class> friend struct init_meta_info;
    static void constructor (void* ptr) //TODO: (Kernel&) ???
        {
            new (ptr) _Self;
        }
    static void destructor (void* ptr)
        {
            static_cast<_Self*>(ptr)->~_Self();
        }
};

// ------------------------------------
// state<>

template<class _Self, class _Parent>
class state
    :   public any_state
    ,   public aComposition_Policy<_Self, _Parent> {
public:
    //XXX _Parent can be either:
    // - composite state (and | or)
    // - state machine (two forms: [a] final, concrete [b] embeddable, for reuse)
    state ()
        {
            log << __FUNCTION__ << endl;
        }
    typedef mBS META;
    const META& meta () const { return Rtti<META, _Self>::meta()->info; }

private:
    template<class, class> friend struct init_meta_info;
    static void constructor (void* ptr) //TODO: (Kernel&) ???
        {
            new (ptr) _Self;
        }
    static void destructor (void* ptr)
        {
            static_cast<_Self*>(ptr)->~_Self();
        }
};

// ---------------------------------------------------------------------------
// coe::init_meta_info<(mSM|mOS|mBS), Type>

template<class _State>
struct init_state_meta_ {

    struct parent_is_state {
        static void apply (mXS& info)
            {
                info.par = & Rtti<typename _State::PARENT::META,
                                  typename _State::PARENT>::meta()->info;
                cout << "### parent " << info.par << endl;
                info.next = info.par->chld;
                const_cast<mCS*>(info.par)->chld = &info;
                info.sm = info.par->sm;
            }
    };
    struct parent_is_machine {
        static void apply (mXS& info)
            {
                assert(! info.par);
                info.sm = & Rtti<mSM, typename _State::MACHINE>::meta()->info;
            }
    };

    static void apply (mXS& info)
        {
            typedef typename meta::If_<meta::IsSame_<_State, typename _State::ROOT>::value,
                                       parent_is_machine,
                                       parent_is_state>::type   imp;
            imp::apply(info);
            assert(NULL != info.info);
        }
};

template<class _List> struct force_init_kids_;
template<>
struct force_init_kids_<Nil> {
    static void apply () {}
};
template<class H, class T>
struct force_init_kids_<Cons<H, T> > {
    static void apply () {
        (void) Rtti<typename H::META, H>::meta();
        force_init_kids_<T>::apply();
    }
};

// ------------------------------------

namespace coe {

    template<class Type>
    struct init_meta_info<mSM, Type> {
        void operator() (mSM& info) const
            {
                assert(! info.root);
                info.root = & Rtti<typename Type::ROOT::META,
                                   typename Type::ROOT>::meta()->info;
                info.info = &typeid(Type);
                info.name = demangle(*info.info);
                log << "@ init_meta_info --> " << info << endl;
            }
    };

    template<class Type>
    struct init_meta_info<mOS, Type> {
        void operator() (mOS& info) const
            {
                cout << ">>> BEG -- " << &info << " " << demangle(typeid(Type)) << endl;
                assert(! info.size);
                info.info = &typeid(Type);
                info.name = demangle(*info.info);
                init_state_meta_<Type>::apply(info);    ///
                info.level = Type::LEVEL;
                info.size = sizeof(Type);
                info.put  = & Type::constructor;
                info.clr  = & Type::destructor;
                info.init = & Rtti<typename Type::INIT::META,
                                   typename Type::INIT>::meta()->info;
                info.hist = History(Type::_HISTORY_TYPE);
                log << "@ init_meta_info --> " << info << endl;
                cout << "<<< END -- " << demangle(typeid(Type)) << endl;
            }
    };

    template<class Type>
    struct init_meta_info<mAS, Type> {
        void operator() (mAS& info) const
            {
                cout << ">>> BEG -- " << &info << " " << demangle(typeid(Type)) << endl;
                assert(! info.size);
                info.info = &typeid(Type);
                info.name = demangle(*info.info);
                init_state_meta_<Type>::apply(info);    ///
                info.level = Type::LEVEL;
                info.size = sizeof(Type);
                info.put  = & Type::constructor;
                info.clr  = & Type::destructor;
                log << "@ init_meta_info --> " << info << endl;
                force_init_kids_<typename Type::INIT_LIST>::apply();
                cout << "<<< END -- " << demangle(typeid(Type)) << endl;
            }
    };

    template<class Type>
    struct init_meta_info<mBS, Type> {
        void operator() (mBS& info) const
            {
                cout << ">>> BEG -- " << &info << " " << demangle(typeid(Type)) << endl;
                assert(! info.size);
                info.info = &typeid(Type);
                info.name = demangle(*info.info);
                init_state_meta_<Type>::apply(info);    ///
                info.level = Type::LEVEL;
                info.size = sizeof(Type);
                info.put  = & Type::constructor;
                info.clr  = & Type::destructor;
                log << "@ init_meta_info --> " << info << endl;
                cout << "<<< END -- " << demangle(typeid(Type)) << endl;
            }
    };
}

// ===========================================================================
// typedefs used to define reactions typedef utilized at compile time

template<class Ev,
         class Dest,
         class Obj,
         typename handler_type<typename Ev::args_type>::template mem_fun<Obj>::type fun>
class transition {
    char tmp[64];
};

// ===========================================================================
// Session_<...>
//
// specializations. policies don't work with Session since they operate on the same level

template<class _Self,
         class _StartArgs = Nil>
class Session_ : public Session {
public:
    SiD start_session (Kernel& kernel,                  ValParam_<_StartArgs> arg);
    SiD start_session (Kernel& kernel, Session& parent, ValParam_<_StartArgs> arg);
    SiD start_session (Kernel& kernel,                  RefParam_<_StartArgs> arg);
    SiD start_session (Kernel& kernel, Session& parent, RefParam_<_StartArgs> arg);
protected:
    typedef Session_ SESSION_;
    Session_ (typename handler_type<_StartArgs>::template mem_fun<_Self>::type fun)
        : Session(handler(static_cast<_Self&>(*this), fun)) {}
};
template<class _Self>
class Session_<_Self, Nil> : public Session {
public:
    SiD start_session (Kernel& kernel);
    SiD start_session (Kernel& kernel, Session& parent);
protected:
    typedef Session_ SESSION_;
    Session_ (typename handler_type<Nil>::template mem_fun<_Self>::type fun)
        : Session(handler(static_cast<_Self&>(*this), fun)) {}
};

// ------------------------------------
// ... examples

class MySes0 : public Session_<MySes0> {
public:
    static SiD spawn (Kernel& kernel)
        {
            //return (new MySes0)->start_session(kernel, vparam(int(1)));
            //return (new MySes0)->start_session(kernel, vparam(short(1)));
            //return (new MySes0)->start_session(kernel, (EventArg*)0);
            return (new MySes0)->start_session(kernel);                         //XXX only that works!
        }
private:
    MySes0 () : SESSION_(&MySes0::_start) {}
    void _start (Kernel&);
};

// ----------------

class MySes1 : public Session_<MySes1, List1<int>::type> {
public:
    static SiD spawn (Kernel& kernel)
        {
            return (new MySes1)->start_session(kernel, vparam(int(1)));         //XXX only that works!
            //return (new MySes1)->start_session(kernel, vparam(short(1)));
            //return (new MySes1)->start_session(kernel, (EventArg*)0);
            //return (new MySes1)->start_session(kernel);
        }
private:
    MySes1 () : SESSION_(&MySes1::_start) {}
    void _start (Kernel&, int&);
};

// ===========================================================================

ostream& operator<< (ostream& os, const ArgI& arg)
{
    const type_info*    ti = arg.type.ptr();
    if (ti)
        os << demangle(*ti);
    else
        os << '?';
    return os;
}

ostream& operator<< (ostream& os, const ArgListI& arglist)
{
    os << '[';
    for (size_t i = 0; i < arglist.len; ++i)
        os << (i ? ", " : "") << *arglist.arg[i];
    return os << ']';
}

// ------------------------------------

static char state_symb (const mXS& xs)
{
    switch (xs.type()) {
        default:    assert(0);
        case eBS:   return '-';
        case eAS:   return '&';
        case eOS:
            switch (static_cast<const mOS&>(xs).hist) {
                default:                assert(0);
                case NO_HISTORY:        return '|';
                case SHALLOW_HISTORY:   return 'h';
                case DEEP_HISTORY:      return 'H';
                case INHERITED_HISTORY: return '/';
            }
    }
}

static void print_xs (ostream& os, const mXS& xs, bool init)
{
#if 0
    os << state_symb(xs) << " " << xs.path() << (init ? " =" : "") << "\n";
#else
    os << state_symb(xs) << " " << string(xs.level * 2, ' ')
       << xs.name << (init ? " @" : "") << "\n";
#endif
    eSTATE  type = xs.type();
    if (eBS != type) {
        const mCS&  comp = static_cast<const mCS&>(xs);
        const mXS*  init_child = (eOS == type) ? static_cast<const mOS&>(xs).init : NULL;
        for (const mXS* ch = comp.chld; ch; ch = ch->next) {
            print_xs(os, *ch, (ch == init_child));
        }
    }
}

ostream& print_meta_machine (ostream& os, const mSM& sm)
{
    assert(NULL != sm.root);
    print_xs(os, *sm.root, false);
    return os;
}

// ------------------------------------

template<class Info>
ostream& print_meta_info (ostream& os)
{
    for (const Meta<Info>* meta = Meta<Info>::registry(); meta; meta = meta->next)
        os << "  #" << meta->indx << " " << meta->info << endl;
    return os;
}

void print_meta_info ()
{
#if 1   /* check integrity of meta-info types which duplicated their identities */
    for (const Meta<ArgI>* meta = Meta<ArgI>::registry(); meta; meta = meta->next)
        assert(meta->indx == meta->info.iid);
    for (const Meta<ArgListI>* meta = Meta<ArgListI>::registry(); meta; meta = meta->next)
        assert(meta->indx == meta->info.iid);
#endif
    print_meta_info<ArgI>(log << "@ ArgI" << endl);
    print_meta_info<ArgListI>(log << "@ ArgListI" << endl);
    print_meta_info<mSM>(log << "@ mSM" << endl);
    print_meta_info<mOS>(log << "@ mOS" << endl);
    print_meta_info<mAS>(log << "@ mAS" << endl);
    print_meta_info<mBS>(log << "@ mBS" << endl);
}

// ***************************************************************************
// 'SM2' revisited

namespace sm2 {

struct OUT;
struct A;
struct C;
struct C1;
struct D1;
struct D3_1;
struct E;
struct E1;
struct F;
struct F1;

struct SM2 : machine<SM2, OUT> { SM2(); };
#if 1
struct OUT : or_state<OUT, SM2, A> {};
#else
struct OUT : and_state<OUT, SM2, List1<A>::type> {};
#endif
struct A : or_state<A, OUT, C> {};
struct C : or_state<C, A, C1> {};
struct C1 : state<C1, C> {};
struct C2 : state<C2, C> {};
struct D : or_state<D, A, D1, DEEP_HISTORY> {};
struct D1 : state<D1, D> {};
struct D2 : state<D2, D> {};
struct D3 : or_state<D3, D, D3_1> {};
struct D3_1 : state<D3_1, D3> {};
struct D3_2 : state<D3_2, D3> {};
struct B : and_state<B, OUT, List2<E, F>::type> {};
struct E : or_state<E, B, E1> {};
struct E1 : state<E1, E> {};
struct E2 : state<E2, E> {};
struct F : or_state<F, B, F1, SHALLOW_HISTORY> {};
struct F1 : state<F1, F> {};
struct F2 : state<F2, F> {};

} // namespace sm2

/// -- force all states to be present
sm2::SM2::SM2() { B b; F2 f2; E2 e2; D3_2 d3_2; D2 d2; C2 c2; }

// ---------------------------------------------------------------------------

struct ev1 : event<short> {};
struct Dest {};
struct MySes {
    void on_ev1 (Kernel&, short&);
};

// ===========================================================================

#define EVAL_(expr) do { cout << #expr << " |--> " << (expr) << endl; } while(0)
#define PRINT_LIST_(list) do { cout << #list << " |--> " << Rtti<ArgListI, list >::meta()->info << endl; } while(0)

struct A_ {};
struct B_ {};
struct C_ {};

int main ()
{
    log << string(65, '=') << endl;
    print_meta_info();
    log << "*** START " << string(50, '-') << " " << __FUNCTION__ << endl;
    {
        sm2::SM2    sm2;
    }
    log << "**** STOP " << string(50, '-') << " " << __FUNCTION__ << endl;
    print_meta_info();

    transition<ev1, Dest, MySes, &MySes::on_ev1>    trans1;
    cout << "transition<>:))))) = " << sizeof(trans1) << endl;

    EVAL_((Rtti<ArgListI, Reverse<state_path<sm2::F1>::type>::type>::meta()->info));
    ///
    PRINT_LIST_(Reverse<state_path<sm2::F1>::type>::type);
    PRINT_LIST_(Nil);
    PRINT_LIST_(Reverse<Nil>::type);
    PRINT_LIST_(Reverse<state_path<sm2::D1>::type>::type);
    PRINT_LIST_(Reverse<state_path<sm2::D3_2>::type>::type);

    typedef Reverse<state_path<sm2::D1>::type>::type    to__D1;
    typedef Reverse<state_path<sm2::D3_2>::type>::type  to__D3_2;

    PRINT_LIST_(to__D1);
    PRINT_LIST_(to__D3_2);

    EVAL_((Rtti<ArgListI, Common<to__D1, to__D3_2>::type>::meta()->info));

    //XXX RESTRICTION: One cannot use classes defined locally, i.e. main()::A_ :(
    EVAL_((Rtti<ArgListI, Common<List3<A_,B_,C_>::type, List3<C_,B_,A_>::type>::type>::meta()->info));
    EVAL_((Rtti<ArgListI, Common<List3<A_,B_,C_>::type, List3<A_,C_,B_>::type>::type>::meta()->info));
    EVAL_((Rtti<ArgListI, Common<List3<A_,B_,C_>::type, List3<A_,B_,C_>::type>::type>::meta()->info));
    EVAL_((Rtti<ArgListI, Common<List3<A_,B_,C_>::type, List4<A_,B_,C_,A_>::type>::type>::meta()->info));

    for (const Meta<mSM>* meta = Meta<mSM>::registry(); meta; meta = meta->next) {
        print_meta_machine(cout << "\n### ===== " << meta->info.name << "\n", meta->info) << endl;
    }
}

