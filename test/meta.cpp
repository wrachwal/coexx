// meta.cpp

#include "coe-meta.h"

#include "coe-oev.h"    // Cons, ...

#include <string>
#include <iostream>

using namespace std;
using namespace coe;

// ---------------------------------------------------------------------------
// ArgI

struct ArgI {
    TypeInfo    type;
};

namespace coe {
    template<class Type>
    struct init_meta_info<Type, ArgI> {
        void operator() (ArgI& info) const
            {
                info.type = typeid(Type);
            }
    };
}

// ---------------------------------------------------------------------------
// ArgListI

struct ArgListI {
    const ArgI**    arg;
    size_t          len;
};

namespace coe {
    template<class List>
    struct init_meta_info<List, ArgListI> {
        void operator() (ArgListI& info) const
            {
                static const ArgI*  args[Length<List>::value + 1];
                info.arg = args;
                info.len = Length<List>::value;
                // initialize args[...]
            }
    };
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

    typedef List3<int, short, string>::type A_3;
    cout << "length of 3 items list = " << Length<A_3>::value << endl;

    Ctti<A_3, ArgListI>::meta();

    return 0;
}

