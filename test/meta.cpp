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
    const Meta<ArgI>**  arg;
    size_t              len;
};

template<class List> struct assign_arg_info;
template<>
struct assign_arg_info<Nil> {
    static void apply (const Meta<ArgI>**) {}
};
template<class Head, class Tail>
struct assign_arg_info<Cons<Head, Tail> > {
    static void apply (const Meta<ArgI>** argi)
        {
            *argi = Ctti<Head, ArgI>::meta();
            assign_arg_info<Tail>::apply(++argi);
        }
};

namespace coe {
    template<class List>
    struct init_meta_info<List, ArgListI> {
        void operator() (ArgListI& info) const
            {
                static const Meta<ArgI>*    args[Length<List>::value + 1];
                info.arg = args;
                info.len = Length<List>::value;
                assign_arg_info<List>::apply(args);
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

    return 0;
}

