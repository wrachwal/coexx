// chsm.cpp

#include "coe-kernel.h"

#include <string>
#include <iostream>

using namespace std;
using namespace coe;

ostream&    log = cout;

namespace coe { string demangle (const type_info&); }   // coe--util.h

// ===========================================================================
// meta info

enum eSTATE { e_S = 0, eOS = 1, eAS = 2 };

struct mSM; // of machine<>
struct mXS; // of ...state<>
struct mCS; // of "composition"_state<>
struct mOS; // of or_state<>
struct mAS; // of and_state<>
struct mBS; // of "basic"state<>

struct mSM {
    mSM () : root(0) {}
    const mXS*  root;
    ostream& print (ostream& os) const
        {
            return os << this << "=mSM(root=" << root << ")";
        }
};

struct mXS {
    mXS () : sm(0), par(0), next(0), size(0), put(0), clr(0) {}
    virtual ~mXS () {}
    virtual eSTATE type () const = 0;
    virtual ostream& print (ostream& os) const = 0;
    ostream& _print (ostream& os) const
        {
            return os << "mXS(sm=" << sm << " par=" << par
                << " size=" << size
                << " put=" << (void*)put
                << " clr=" << (void*)clr
            << ")";
        }
    const mSM*  sm;
    const mCS*  par;
    const mXS*  next;
    size_t      size;
    void (*put)(void*);
    void (*clr)(void*);
};

struct mCS : mXS {
    mCS () : chld(0) {}
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
    mOS () : init(0) {}
    eSTATE type () const { return eOS; }
    ostream& print (ostream& os) const
        {
            return mCS::_print(os << this << "=mOS(") << " init=" << init << ")";
        }
    const mXS*  init;
};

struct mAS : mCS {
    mAS () {}
    eSTATE type () const { return eAS; }
    ostream& print (ostream& os) const
        {
            return mCS::_print(os << this << "=mAS(") << ")";
        }
};

struct mBS : mXS {
    eSTATE type () const { return e_S; }
    ostream& print (ostream& os) const
        {
            return mXS::_print(os << this << "=mBS(") << ")";
        }
};

// ------------------------------------

ostream& operator<< (ostream& os, const mSM& sm) { return sm.print(os); }
ostream& operator<< (ostream& os, const mXS& xs) { return xs.print(os); }

// ===========================================================================

template<class _Root> struct RootStatePolicy { typedef _Root ROOT; };
template<>            struct RootStatePolicy<void> {};

template<class _Init> struct InitStatePolicy { typedef _Init INIT; };
template<>            struct InitStatePolicy<void> {};

template<class _InitList> struct InitListStatesPolicy { typedef _InitList INIT_LIST; };
template<>                struct InitListStatesPolicy<void> {};

// ---------------------------------------------------------------------------

template<class _Self,
         class _Parent,
         bool _isRoot = meta::IsSame_<typename _Parent::META, mSM>::value>
class aComposition_Policy;

template<class _Self, class _Parent>
class aComposition_Policy<_Self, _Parent, false> {

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
class aComposition_Policy<_Self, _Parent, true> {
public:
    enum { LEVEL = 0 };
    typedef _Self   SELF;
    typedef _Self   ROOT;
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

// ===========================================================================
// machine<>

template<class _Self, class _Root = void>
class machine
    :   public RootStatePolicy<_Root> {
public:
    machine ()
        :   _meta(Ctti<_Self, mSM>::meta()->info)
        {
            log << __FUNCTION__ << endl;
        }
    typedef mSM META;

private:
    const META& _meta;
public: /// DEBUG
    enum { _EMPTY_SIZE = sizeof(META*) };
};

// ------------------------------------
// or_state<>

template<class _Self, class _Parent, class _Init = void>
class or_state
    :   public InitStatePolicy<_Init>
    ,   public aComposition_Policy<_Self, _Parent> {
public:
    or_state ()
        {
            log << __FUNCTION__ << endl;
        }
    typedef mOS META;

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
public: /// DEBUG
    enum { _EMPTY_SIZE = 0 };
};

// ------------------------------------
// and_state<>

template<class _Self, class _Parent, class _InitList = void>
class and_state
    :   public InitListStatesPolicy<_InitList>
    ,   public aComposition_Policy<_Self, _Parent> {
public:
    and_state ()
        {
            log << __FUNCTION__ << endl;
        }
    typedef mAS META;

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
public: /// DEBUG
    enum { _EMPTY_SIZE = 0 };
};

// ------------------------------------
// state<>

template<class _Self, class _Parent>
class state
    :   public aComposition_Policy<_Self, _Parent> {
public:
    //XXX _Parent can be either:
    // - composite state (and | or)
    // - state machine (two forms: [a] final, concrete [b] embeddable, for reuse)
    state ()
        {
            log << __FUNCTION__ << endl;
        }
    typedef mBS META;

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
public: /// DEBUG
    enum { _EMPTY_SIZE = 0 };
};

// ---------------------------------------------------------------------------
// coe::init_meta_info<Type, (mSM|mOS|mBS)>

template<class _State>
struct init_state_meta_ {

    struct parent_is_state {
        static void apply (mXS& info)
            {
                info.par = & Ctti<typename _State::PARENT,
                                  typename _State::PARENT::META>::meta()->info;
                info.next = info.par->chld;
                const_cast<mCS*>(info.par)->chld = &info;
                info.sm = info.par->sm;
            }
    };
    struct parent_is_machine {
        static void apply (mXS& info)
            {
                assert(! info.par);
                info.sm = & Ctti<typename _State::MACHINE, mSM>::meta()->info;
            }
    };

    static void apply (mXS& info)
        {
            typedef typename meta::If_<meta::IsSame_<_State, typename _State::ROOT>::value,
                                       parent_is_machine,
                                       parent_is_state>::type   imp;
            imp::apply(info);
        }
};

// ------------------------------------

namespace coe {

    template<class Type>
    struct init_meta_info<Type, mSM> {
        void operator() (mSM& info) const
            {
                assert(! info.root);
                info.root = & Ctti<typename Type::ROOT, typename Type::ROOT::META>::meta()->info;
                log << "@ init_meta_info --> " << info << endl;
            }
    };

    template<class Type>
    struct init_meta_info<Type, mOS> {
        void operator() (mOS& info) const
            {
                assert(! info.size);
                init_state_meta_<Type>::apply(info);    ///
                info.size = sizeof(Type);
                info.put  = & Type::constructor;
                info.clr  = & Type::destructor;
                info.init = & Ctti<typename Type::INIT, typename Type::INIT::META>::meta()->info;
                log << "@ init_meta_info --> " << info << endl;
            }
    };

    template<class Type>
    struct init_meta_info<Type, mAS> {
        void operator() (mAS& info) const
            {
                assert(! info.size);
                init_state_meta_<Type>::apply(info);    ///
                info.size = sizeof(Type);
                info.put  = & Type::constructor;
                info.clr  = & Type::destructor;
                log << "@ init_meta_info --> " << info << endl;
            }
    };

    template<class Type>
    struct init_meta_info<Type, mBS> {
        void operator() (mBS& info) const
            {
                assert(! info.size);
                init_state_meta_<Type>::apply(info);    ///
                info.size = sizeof(Type);
                info.put  = & Type::constructor;
                info.clr  = & Type::destructor;
                log << "@ init_meta_info --> " << info << endl;
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

// ------------------------------------
// state_path<State>

template<class State, int Level = State::LEVEL>
struct state_path {
    typedef Cons<State, typename state_path<typename State::PARENT>::type> type;
};

template<class Root>
struct state_path<Root, 0> {
    typedef Cons<Root, Nil> type;
};

// ------------------------------------
// Reverse<List>

template<class, class> struct Reverse_Acc;
template<class Acc>
struct Reverse_Acc<Nil, Acc> {
    typedef Acc type;
};
template<class Head, class Tail, class Acc>
struct Reverse_Acc<Cons<Head, Tail>, Acc> {
    typedef typename Reverse_Acc<Tail, Cons<Head, Acc> >::type type;
};
// ------
template<class List>
struct Reverse {
    typedef typename Reverse_Acc<List, Nil>::type type;
};

// ------------------------------------
// Common<Lst1, Lst2>

template<class Lst1, class Lst2>
struct Common;
template<class H1, class T1, class H2, class T2>
struct Common<Cons<H1, T1>, Cons<H2, T2> > {
    typedef Nil type;
};
template<class H, class T1, class T2>
struct Common<Cons<H, T1>, Cons<H, T2> > {
    typedef Cons<H, typename Common<T1, T2>::type> type;
};
template<class Lst>
struct Common<Lst, Nil> {
    typedef Nil type;
};
template<class Lst>
struct Common<Nil, Lst> {
    typedef Nil type;
};
template<>
struct Common<Nil, Nil> {
    typedef Nil type;
};

// ===========================================================================

template<class Info>
ostream& print_meta_info (ostream& os)
{
    for (const Meta<Info>* meta = Meta<Info>::registry(); meta; meta = meta->next)
        os << "  #" << meta->indx << " " << meta->info << endl;
    return os;
}

void print_meta_info ()
{
    print_meta_info<mSM>(log << "@ mSM" << endl);
    print_meta_info<mOS>(log << "@ mOS" << endl);
    print_meta_info<mAS>(log << "@ mAS" << endl);
    print_meta_info<mBS>(log << "@ mBS" << endl);
}

// ------------------------------------

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
        os << (i ? ", " : "") << arglist.arg[i]->info;
    return os << ']';
}

// ***************************************************************************
// 'SM2' revisited

namespace sm2a {

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

struct SM2a : machine<SM2a, OUT> {};
struct OUT : or_state<OUT, SM2a, A> {};
struct A : or_state<A, OUT, C> {};
struct C : or_state<C, A, C1> {};
struct C1 : state<C1, C> {};
struct C2 : state<C2, C> {};
struct D : or_state<D, A, D1> {};   // deep-history
struct D1 : state<D1, D> {};
struct D2 : state<D2, D> {};
struct D3 : or_state<D3, D, D3_1> {};
struct D3_1 : state<D3_1, D3> {};
struct D3_2 : state<D3_2, D3> {};
struct B : and_state<B, OUT, List2<E, F>::type> {};
struct E : or_state<E, B, E1> {};
struct E1 : state<E1, E> {};
struct E2 : state<E2, E> {};
struct F : or_state<F, B, F1> {};   // shallow-history
struct F1 : state<F1, F> {};
struct F2 : state<F2, F> {};

} // namespace sm2a

// ---------------------------------------------------------------------------
// other way to structure a state machine

namespace sm2b {

struct SM2b : machine<SM2b> {
    typedef struct OUT : or_state<OUT, SM2b> {
        struct B;   ///XXX forwarding will be necessary
        typedef struct A : or_state<A, OUT> {
            typedef struct C : or_state<C, A> {
                typedef struct C1 : state<C1, C> {} INIT;
                struct C2 : state<C2, C> {};
            } INIT;
            struct D : or_state<D, A> {         // deep-history
                typedef struct D1 : state<D1, D> {} INIT;
                struct D2 : state<D2, D> {};
                struct D3 : or_state<D3, D> {
                    typedef struct D3_1 : state<D3_1, D3> {} INIT;
                    struct D3_2 : state<D3_2, D3> {};
                    void access (SM2b& sm, OUT& out, A& a)
                        {
                            sm.xxx  = 1;
                            out.qqq = 2;
                            a.aaa   = 3;
                        }
                    //XXX if we want use types defined later :(
                    typedef B elsewhere_B;
                };
            };
        private:
            int aaa;
        } INIT;
        struct B : and_state<B, OUT> {
            typedef struct E : or_state<E, B> {
                typedef struct E1 : state<E1, E> {} INIT;
                struct E2 : state<E2, E> {};
            } INIT;
            struct F : or_state<F, B> {         // shallow-history
                typedef struct F1 : state<F1, F> {} INIT;
                struct F2 : state<F2, F> {};
            };
            typedef List2<E, F>::type INIT_LIST;
        };
    private:
        int qqq;
    } ROOT;
private:
    int xxx;
};

} // namespace sm2b

// ---------------------------------------------------------------------------

struct ev1 : event<short> {};
struct Dest {};
struct MySes {
    void on_ev1 (Kernel&, short&);
};

// ===========================================================================

#define EVAL_(expr) do { cout << #expr << " |--> " << (expr) << endl; } while(0)
#define PRINT_LIST_(list) do { cout << #list << " |--> " << Ctti<list, ArgListI>::meta()->info << endl; } while(0)

struct A_ {};
struct B_ {};
struct C_ {};

int main ()
{
    log << string(65, '=') << endl;
    print_meta_info();
    log << "*** START " << string(50, '-') << " " << __FUNCTION__ << endl;
    {
        sm2a::SM2a  sm2a;
    }
    {
        sm2b::SM2b  sm2b;
    }
    log << "**** STOP " << string(50, '-') << " " << __FUNCTION__ << endl;
    print_meta_info();

    transition<ev1, Dest, MySes, &MySes::on_ev1>    trans1;
    cout << "transition<>:))))) = " << sizeof(trans1) << endl;

    EVAL_((Ctti<Reverse<state_path<sm2a::F1>::type>::type, ArgListI>::meta()->info));
    ///
    PRINT_LIST_(Reverse<state_path<sm2a::F1>::type>::type);
    PRINT_LIST_(Nil);
    PRINT_LIST_(Reverse<Nil>::type);
    PRINT_LIST_(Reverse<state_path<sm2b::SM2b::OUT::B::F::F1>::type>::type);
    PRINT_LIST_(Reverse<state_path<sm2a::D1>::type>::type);
    PRINT_LIST_(Reverse<state_path<sm2a::D3_2>::type>::type);

    typedef Reverse<state_path<sm2a::D1>::type>::type   to__D1;
    typedef Reverse<state_path<sm2a::D3_2>::type>::type to__D3_2;

    PRINT_LIST_(to__D1);
    PRINT_LIST_(to__D3_2);

    EVAL_((Ctti<Common<to__D1, to__D3_2>::type, ArgListI>::meta()->info));

    //XXX RESTRICTION: One cannot use classes defined locally, i.e. main()::A_ :(
    EVAL_((Ctti<Common<List3<A_,B_,C_>::type, List3<C_,B_,A_>::type>::type, ArgListI>::meta()->info));
    EVAL_((Ctti<Common<List3<A_,B_,C_>::type, List3<A_,C_,B_>::type>::type, ArgListI>::meta()->info));
    EVAL_((Ctti<Common<List3<A_,B_,C_>::type, List3<A_,B_,C_>::type>::type, ArgListI>::meta()->info));
    EVAL_((Ctti<Common<List3<A_,B_,C_>::type, List4<A_,B_,C_,A_>::type>::type, ArgListI>::meta()->info));
}
