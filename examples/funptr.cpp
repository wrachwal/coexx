// funptr.cpp

#include <iostream>
#include "coe--local.h" // namespace meta
#include "coe-oev.h"

using namespace std;
using namespace coe;

// ---------------------------------------------------------------------------

struct Gfun {
    class _R;
    typedef _R (*type)();
};

struct Mfun {
    class _Obj; class _R;
    typedef _R (_Obj::*type)();
};

// ---------------------------------------------------------------------------

#define PFX_CHECK_NOARGS_(PFX)  \
        COE_STATIC_CHECK_(0 == Length<PFX>::value, Missing_Prefix)

#define PFX_CHECK_(N, PFX)                                                          \
        COE_STATIC_CHECK_(                                                          \
            (meta::IsSame_<PFX, typename Common<PFX,                                \
                                                typename List##N<COE_T(N, A)>::type \
                                               >::type                              \
                          >::value),                                                \
            Wrong_Prefix)

// ------------------------------------

template<class R, class PFX=Nil>
struct MFUN {

    // -- 0

    template<class Obj>
    static Mfun::type ptr (R (Obj::*fun)())
        { PFX_CHECK_NOARGS_(PFX); return Mfun::type(fun); }

    template<class Obj>
    static Mfun::type ptr (R (Obj::*fun)() const)
        { PFX_CHECK_NOARGS_(PFX); return Mfun::type(fun); }

    // -- 1

    template<class Obj, COE_T(1, class A)>
    static Mfun::type ptr (R (Obj::*fun)(COE_TA(1, A, &a)))
        { PFX_CHECK_(1, PFX); return Mfun::type(fun); }

    template<class Obj, COE_T(1, class A)>
    static Mfun::type ptr (R (Obj::*fun)(COE_TA(1, A, &a)) const)
        { PFX_CHECK_(1, PFX); return Mfun::type(fun); }

    // -- 2

    template<class Obj, COE_T(2, class A)>
    static Mfun::type ptr (R (Obj::*fun)(COE_TA(2, A, &a)))
        { PFX_CHECK_(2, PFX); return Mfun::type(fun); }

    template<class Obj, COE_T(2, class A)>
    static Mfun::type ptr (R (Obj::*fun)(COE_TA(2, A, &a)) const)
        { PFX_CHECK_(2, PFX); return Mfun::type(fun); }

    // -- 3

    template<class Obj, COE_T(3, class A)>
    static Mfun::type ptr (R (Obj::*fun)(COE_TA(3, A, &a)))
        { PFX_CHECK_(3, PFX); return Mfun::type(fun); }

    template<class Obj, COE_T(3, class A)>
    static Mfun::type ptr (R (Obj::*fun)(COE_TA(3, A, &a)) const)
        { PFX_CHECK_(3, PFX); return Mfun::type(fun); }
};

// ---------------------------------------------------------------------------

struct A1 {};
struct A2 {};

struct ev12 : event<A1, A2> {};

struct Cls {
    void m0a () {}
    void m2a (A1&, A2&) {}
    void h2a (Kernel&, A1&, A2&) {}
    void h2A (Kernel&, A1&, A2&) const {}
};
void g2a (A1&, A2&) {}

// ***************************************************************************

typedef Cons<Kernel, Nil> CoePfx;

int main ()
{
    Mfun::type m2 = Mfun::type(&Cls::m2a);
    // for (void*)m2 we get the message:
    // converting from 'Mfun::_R (Mfun::_Obj::*)()' to 'void*'
    // warning with gcc 4.4.3; error with gcc 4.1.1
    cout << m2 << " sizeof(m2)=" << sizeof(m2) << endl;
    Gfun::type g2 = Gfun::type(&g2a);
    cout << (void*)g2 << " sizeof(g2)=" << sizeof(g2) << endl;

    // ===

    cout << MFUN<void>        ::ptr(&Cls::m0a) << endl;

    cout << MFUN<void>        ::ptr(&Cls::m2a) << endl;
    cout << MFUN<void, CoePfx>::ptr(&Cls::h2a) << endl;

    cout << MFUN<void, CoePfx>::ptr(&Cls::h2A) << endl;
}

