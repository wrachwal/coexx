// chsm.cpp

#include "coe-kernel.h"

#include <string>
#include <iostream>

using namespace std;
using namespace coe;

ostream&    log = cout;

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

template<class _Self, class _Root>
class machine {
public:
    typedef _Root   ROOT;
    machine ()
        :   _meta(Ctti<_Self, mSM>::meta()->info)
        {
            log << __FUNCTION__ << endl;
        }
    typedef mSM META;

private:
    const META& _meta;
public: /// DEBUG
    enum { _EMPTY_SIZE = sizeof(&_meta) };
};

// ------------------------------------
// or_state<>

template<class _Self, class _Parent, class _Init>
class or_state
    :   public aComposition_Policy<_Self, _Parent> {
public:
    typedef _Init   INIT;

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

template<class _Self, class _Parent, class _InitList>
class and_state
    :   public aComposition_Policy<_Self, _Parent> {
public:
    typedef _InitList   INIT_LIST;

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

// ***************************************************************************
// 'SM2' revisited

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

struct SM2 : machine<SM2, OUT> {};
struct OUT : or_state<OUT, SM2, A> {};
struct A : or_state<A, OUT, C> {};
struct C : or_state<C, A, C1> {};
struct C1 : state<C1, C> {};
struct C2 : state<C2, C> {};
struct D : or_state<D, A, D1> {};   // deep-history
struct D1 : state<D1, D> {};
struct D2 : state<D2, D> {};
struct D3 : or_state<D3, D, D3_1> {};
struct D3_1 : state<D3_1, D> {};
struct D3_2 : state<D3_2, D> {};
struct B : and_state<B, OUT, List2<E, F> > {};
struct E : or_state<E, B, E1> {};
struct E1 : state<E1, E> {};
struct E2 : state<E2, E> {};
struct F : or_state<F, B, F1> {};   // shallow-history
struct F1 : state<F1, F> {};
struct F2 : state<F2, F> {};

// ---------------------------------------------------------------------------

struct ev1 : event<short> {};
struct Dest {};
struct MySes {
    void on_ev1 (Kernel&, short&);
};

// ===========================================================================

int main ()
{
    log << string(65, '=') << endl;
    print_meta_info();
    log << "*** START " << string(50, '-') << " " << __FUNCTION__ << endl;
    {
        SM2 sm;
    }
    log << "**** STOP " << string(50, '-') << " " << __FUNCTION__ << endl;
    print_meta_info();

    transition<ev1, Dest, MySes, &MySes::on_ev1>    trans1;
    cout << "transition<>:))))) = " << sizeof(trans1) << endl;
}

