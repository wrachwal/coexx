// meta.cpp

#include "coe-kernel.h"

#include <string>
#include <iostream>

using namespace std;
using namespace coe;

ostream&    log = cout;

// ===========================================================================

template<class Self, class Init>
struct machine {
    machine ()
        {
            log << "machine<"
                << Ctti<Self, ArgI>::meta()->info.type.name() << ","
                << Ctti<Init, ArgI>::meta()->info.type.name() << ">"
                << endl;
            Init    init;
        }
};

template<class Self, class Parent, class Init>
struct or_state {
    or_state ()
        {
            log << "or_state<"
                << Ctti<Self,   ArgI>::meta()->info.type.name() << ","
                << Ctti<Parent, ArgI>::meta()->info.type.name() << ","
                << Ctti<Init,   ArgI>::meta()->info.type.name() << ">"
                << endl;
            Init    init;
        }
};

template<class Self, class Parent, class InitList>
struct and_state {
    and_state ()
        {
            log << "and_state<"
                << Ctti<Self,   ArgI>::meta()->info.type.name() << ","
                << Ctti<Parent, ArgI>::meta()->info.type.name() << ","
                << Ctti<InitList, ArgListI>::meta()->info.arg[0]->info.type.name() << "...>"
                << endl;
            typename InitList::car_type init;   // instantiating all impossible this way
        }
};

template<class Self, class Parent>
struct state {
    state ()
        {
            log << "or_state<"
                << Ctti<Self,   ArgI>::meta()->info.type.name() << ","
                << Ctti<Parent, ArgI>::meta()->info.type.name() << ">"
                << endl;
        }
};

// ---------------------------------------------------------------------------
// SM2 revisited

struct A;
struct SM : machine<SM, A> { SM(){} ~SM(){} };
struct C;
struct A : or_state<A, SM, C> { A(){} ~A(){} };
struct E;
struct F;
struct B : and_state<B, SM, List2<E,F>::type> { B(){} ~B(){} };

struct C1;
struct C2;
struct C : or_state<C, A, C1> { C(){} ~C(){} };
struct C1 : state<C1, C> {};
struct C2 : state<C2, C> {};

struct E : state<E, B> {};  // or_state
struct F : state<F, B> {};  // or_state

// ------------------------------------

struct Fwd;

void test_sm ()
{
    //XXX cannot compile -- Ctti<Fwd, ArgI>::meta();

    log << "### SM" << endl;
    SM  sm;
    log << "### B" << endl;
    B   b;  // non-initial state of SM
}

// ***************************************************************************

int main ()
{
    TypeInfo    ti_int = TypeInfo(typeid(int));

    Ctti<int, ArgI>::meta();
    Ctti<long, ArgI>::meta();
    Ctti<string, ArgI>::meta();

    for (const Meta<ArgI>* meta = Meta<ArgI>::registry(); meta; meta = meta->next)
        cout << "ArgI#" << meta->indx << " -- " << meta->info.type.name() << endl;

    typedef List3<int, short, string>::type A_3a;
    typedef List3<int, long, double>::type A_3b;

    cout << "length of 3 items list = " << Length<A_3a>::value << endl;

    Ctti<A_3a, ArgListI>::meta();
    Ctti<A_3b, ArgListI>::meta();

    for (const Meta<ArgListI>* ml = Meta<ArgListI>::registry(); ml; ml = ml->next) {
        cout << "ArgListI#" << ml->indx << " -- args (" << ml->info.len << ")" << endl;
        for (size_t i = 0; i < ml->info.len; ++i) {
            const Meta<ArgI>    *ma = ml->info.arg[i];
            cout << "  - [" << i << "] -- " << ma->info.type.name() << " #" << ma->indx << endl;
        }
    }

    // -----------
    test_sm();

    return 0;
}

