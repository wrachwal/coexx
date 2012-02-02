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
    mSM () : init(0) {}
    const mXS*  init;
};

struct mXS {
    mXS () : sm(0), par(0), size(0) {}
    virtual ~mXS () {}
    virtual eSTATE type () const = 0;
    const mSM*  sm;
    const mXS*  par;
    size_t      size;
};

struct mOS : mXS {
    mOS () : init(0), chld(0) {}
    eSTATE type () const { return eOS; }
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
};

// ===========================================================================

template<class Self, class Init>
class machine {
public:
    machine ()
        {
            log << __FUNCTION__ << endl;
            Init    state;
            log << "&__burn --> " << &__burn << endl;
        }
private:
    struct Burn {
        Burn ()
            {
#if 0
                //XXX RTTI assertion, equality of types
                //      typename Init::Parent =:= Self
                const_cast<mSM&>(Ctti<Self, mSM>::meta()->info).init =
                    Ctti<Init, typename Init::MetaInfo>::meta();
#endif
                log << __FUNCTION__ << endl;
            }
    };
    static Burn __burn;
};

template<class Self, class Init>
typename machine<Self, Init>::Burn machine<Self, Init>::__burn;

template<class Self, class Parent>
class state {
public:
    //XXX Parent can be either:
    // - composite state (and | or)
    // - state machine (two forms: [a] final, concrete [b] embeddable, for reuse)
    state ()
        {
            log << __FUNCTION__ << endl;
        }
private:
};

// ***************************************************************************

struct A;   // FORWARD: SM's init state
struct SM : machine<SM, /*init*/A> { SM(){} ~SM(){} };

struct A : state<A, SM> { A(){} ~A(){} };

// ===========================================================================

int main ()
{
    SM  sm;
    log << "### " << __FUNCTION__ << endl;
}

