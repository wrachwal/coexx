// funptr.cpp

#include <iostream>
#include "coe--local.h" // namespace meta
#include "coe-oev.h"

#include <stdexcept>

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

template<class R>
R execute (Mfun::type fun, int arity, void* obj, void* arg[]) throw (std::length_error)
{
    class _Obj;
    class _A;

    typedef R (_Obj::*M00)();
    typedef R (_Obj::*M01)(COE_X( 1, _A&));
    typedef R (_Obj::*M02)(COE_X( 2, _A&));
    typedef R (_Obj::*M03)(COE_X( 3, _A&));
    typedef R (_Obj::*M04)(COE_X( 4, _A&));
    typedef R (_Obj::*M05)(COE_X( 5, _A&));
    typedef R (_Obj::*M06)(COE_X( 6, _A&));
    typedef R (_Obj::*M07)(COE_X( 7, _A&));
    typedef R (_Obj::*M08)(COE_X( 8, _A&));
    typedef R (_Obj::*M09)(COE_X( 9, _A&));
    typedef R (_Obj::*M10)(COE_X(10, _A&));

    switch (arity) {
        default:
            throw std::length_error("more than 10 args");
        case 0:
            return (((_Obj*)obj) ->* (M00(fun)))(
            );
        case 1:
            return (((_Obj*)obj) ->* (M01(fun)))(
                *(_A*)arg[0]
            );
        case 2:
            return (((_Obj*)obj) ->* (M02(fun)))(
                *(_A*)arg[0],
                *(_A*)arg[1]
            );
        case 3:
            return (((_Obj*)obj) ->* (M03(fun)))(
                *(_A*)arg[0],
                *(_A*)arg[1],
                *(_A*)arg[2]
            );
        case 4:
            return (((_Obj*)obj) ->* (M04(fun)))(
                *(_A*)arg[0],
                *(_A*)arg[1],
                *(_A*)arg[2],
                *(_A*)arg[3]
            );
        case 5:
            return (((_Obj*)obj) ->* (M05(fun)))(
                *(_A*)arg[0],
                *(_A*)arg[1],
                *(_A*)arg[2],
                *(_A*)arg[3],
                *(_A*)arg[4]
            );
        case 6:
            return (((_Obj*)obj) ->* (M06(fun)))(
                *(_A*)arg[0],
                *(_A*)arg[1],
                *(_A*)arg[2],
                *(_A*)arg[3],
                *(_A*)arg[4],
                *(_A*)arg[5]
            );
        case 7:
            return (((_Obj*)obj) ->* (M07(fun)))(
                *(_A*)arg[0],
                *(_A*)arg[1],
                *(_A*)arg[2],
                *(_A*)arg[3],
                *(_A*)arg[4],
                *(_A*)arg[5],
                *(_A*)arg[6]
            );
        case 8:
            return (((_Obj*)obj) ->* (M08(fun)))(
                *(_A*)arg[0],
                *(_A*)arg[1],
                *(_A*)arg[2],
                *(_A*)arg[3],
                *(_A*)arg[4],
                *(_A*)arg[5],
                *(_A*)arg[6],
                *(_A*)arg[7]
            );
        case 9:
            return (((_Obj*)obj) ->* (M09(fun)))(
                *(_A*)arg[0],
                *(_A*)arg[1],
                *(_A*)arg[2],
                *(_A*)arg[3],
                *(_A*)arg[4],
                *(_A*)arg[5],
                *(_A*)arg[6],
                *(_A*)arg[7],
                *(_A*)arg[8]
            );
        case 10:
            return (((_Obj*)obj) ->* (M10(fun)))(
                *(_A*)arg[0],
                *(_A*)arg[1],
                *(_A*)arg[2],
                *(_A*)arg[3],
                *(_A*)arg[4],
                *(_A*)arg[5],
                *(_A*)arg[6],
                *(_A*)arg[7],
                *(_A*)arg[8],
                *(_A*)arg[9]
            );
    }
}

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
    double plus_fi (float& f, int& i)
        {
            cout << "calling " << __FUNCTION__ << "(f=" << f << ", i=" << i << ")" << endl;
            return f + i;
        }
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

    // === getting degenerated pointer

    cout << MFUN<void>        ::ptr(&Cls::m0a) << endl;

    cout << MFUN<void>        ::ptr(&Cls::m2a) << endl;
    cout << MFUN<void, CoePfx>::ptr(&Cls::h2a) << endl;

    cout << MFUN<void, CoePfx>::ptr(&Cls::h2A) << endl;

    // === and then execute it

    float   val_f = 5;
    int     val_i = 3;
    void*   argfi[] = { &val_f, &val_i };

    Cls obj;

    double  ret = execute<double>(MFUN<double>::ptr(&Cls::plus_fi),
                                  2,
                                  &obj,
                                  argfi);
    cout << "# ret = " << ret << endl;
}

