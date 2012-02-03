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
struct mOS; // of or_state<>
struct mAS; // of and_state<>
struct m_S; // of state<>

struct mSM {
    mSM () : root(0) {}
    const mXS*  root;
    ostream& print (ostream& os) const
        {
            return os << this << "=mSM(root=" << root << ")";
        }
};

struct mXS {
    mXS () : sm(0), par(0), size(0), put(0), clr(0) {}
    virtual ~mXS () {}
    virtual eSTATE type () const = 0;
    virtual ostream& print (ostream& os) const ///=0 XXX???
        {
            return os << "mXS(sm=" << sm << " par=" << par
                << " size=" << size
                << " put=" << (void*)put
                << " clr=" << (void*)clr
            << ")";
        }
    const mSM*  sm;
    const mXS*  par;
    size_t      size;
    void (*put)(void*);
    void (*clr)(void*);
};

struct mOS : mXS {
    mOS () : init(0), chld(0) {}
    eSTATE type () const { return eOS; }
    ostream& print (ostream& os) const
        {
            return mXS::print(os << this << "=mOS(") << " init=" << init << " chld=?)";
        }
    const mXS*  init;
    const mXS*  chld;
};

struct mAS : mXS {
    mAS () : chld(0) {}
    eSTATE type () const { return eAS; }
    const mXS*  chld;
};

struct m_S : mXS {
    eSTATE type () const { return e_S; }
    ostream& print (ostream& os) const
        {
            return mXS::print(os << this << "=m_S(") << ")";
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
    typedef m_S META;

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

template<class _State>
struct init_state_meta_ {

    struct parent_is_state {
        static void apply (mXS& info)
            {
                info.par = & Ctti<typename _State::PARENT,
                                  typename _State::PARENT::META>::meta()->info;
                info.sm  = info.par->sm;
            }
    };
    struct parent_is_machine {
        static void apply (mXS& info)
            {
                assert(! info.par);
                info.sm  = & Ctti<typename _State::MACHINE, mSM>::meta()->info;
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
    struct init_meta_info<Type, m_S> {
        void operator() (m_S& info) const
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
    print_meta_info<m_S>(log << "@ m_S" << endl);
}

// ***************************************************************************

#define SIZE_(n)    char __reserve[(n) - _EMPTY_SIZE];

struct A;   // FORWARD: SM's init state
struct SM : machine<SM, /*init*/A> { SM(){} ~SM(){} };

struct C;   // FORWARD: A's init state
struct A : or_state<A, SM, C> { A(){} ~A(){} SIZE_(20) };

struct C : state<C, A> { C(){} ~C(){} SIZE_(12); };

// ===========================================================================

int main ()
{
    log << string(65, '=') << endl;
    print_meta_info();
    log << "*** START " << string(50, '-') << " " << __FUNCTION__ << endl;
    {
        //SM  sm;
    }
    log << "**** STOP " << string(50, '-') << " " << __FUNCTION__ << endl;
    print_meta_info();
}

