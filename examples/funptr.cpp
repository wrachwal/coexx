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
    typedef R (_Obj::*M11)(COE_X(11, _A&));
    typedef R (_Obj::*M12)(COE_X(12, _A&));
    typedef R (_Obj::*M13)(COE_X(13, _A&));
    typedef R (_Obj::*M14)(COE_X(14, _A&));
    typedef R (_Obj::*M15)(COE_X(15, _A&));

    switch (arity) {
        default:
            throw std::length_error("more than 15 args");
        case 0:
            return (((_Obj*)obj) ->* (M00(fun)))(
            );
        case 1:
            return (((_Obj*)obj) ->* (M01(fun)))(
                *(_A*)arg[ 0]
            );
        case 2:
            return (((_Obj*)obj) ->* (M02(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1]
            );
        case 3:
            return (((_Obj*)obj) ->* (M03(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2]
            );
        case 4:
            return (((_Obj*)obj) ->* (M04(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2], *(_A*)arg[ 3]
            );
        case 5:
            return (((_Obj*)obj) ->* (M05(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2], *(_A*)arg[ 3], *(_A*)arg[ 4]
            );
        case 6:
            return (((_Obj*)obj) ->* (M06(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2], *(_A*)arg[ 3], *(_A*)arg[ 4],
                *(_A*)arg[ 5]
            );
        case 7:
            return (((_Obj*)obj) ->* (M07(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2], *(_A*)arg[ 3], *(_A*)arg[ 4],
                *(_A*)arg[ 5], *(_A*)arg[ 6]
            );
        case 8:
            return (((_Obj*)obj) ->* (M08(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2], *(_A*)arg[ 3], *(_A*)arg[ 4],
                *(_A*)arg[ 5], *(_A*)arg[ 6], *(_A*)arg[ 7]
            );
        case 9:
            return (((_Obj*)obj) ->* (M09(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2], *(_A*)arg[ 3], *(_A*)arg[ 4],
                *(_A*)arg[ 5], *(_A*)arg[ 6], *(_A*)arg[ 7], *(_A*)arg[ 8]
            );
        case 10:
            return (((_Obj*)obj) ->* (M10(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2], *(_A*)arg[ 3], *(_A*)arg[ 4],
                *(_A*)arg[ 5], *(_A*)arg[ 6], *(_A*)arg[ 7], *(_A*)arg[ 8], *(_A*)arg[ 9]
            );
        case 11:
            return (((_Obj*)obj) ->* (M11(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2], *(_A*)arg[ 3], *(_A*)arg[ 4],
                *(_A*)arg[ 5], *(_A*)arg[ 6], *(_A*)arg[ 7], *(_A*)arg[ 8], *(_A*)arg[ 9],
                *(_A*)arg[10]
            );
        case 12:
            return (((_Obj*)obj) ->* (M12(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2], *(_A*)arg[ 3], *(_A*)arg[ 4],
                *(_A*)arg[ 5], *(_A*)arg[ 6], *(_A*)arg[ 7], *(_A*)arg[ 8], *(_A*)arg[ 9],
                *(_A*)arg[10], *(_A*)arg[11]
            );
        case 13:
            return (((_Obj*)obj) ->* (M13(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2], *(_A*)arg[ 3], *(_A*)arg[ 4],
                *(_A*)arg[ 5], *(_A*)arg[ 6], *(_A*)arg[ 7], *(_A*)arg[ 8], *(_A*)arg[ 9],
                *(_A*)arg[10], *(_A*)arg[11], *(_A*)arg[12]
            );
        case 14:
            return (((_Obj*)obj) ->* (M14(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2], *(_A*)arg[ 3], *(_A*)arg[ 4],
                *(_A*)arg[ 5], *(_A*)arg[ 6], *(_A*)arg[ 7], *(_A*)arg[ 8], *(_A*)arg[ 9],
                *(_A*)arg[10], *(_A*)arg[11], *(_A*)arg[12], *(_A*)arg[13]
            );
        case 15:
            return (((_Obj*)obj) ->* (M15(fun)))(
                *(_A*)arg[ 0], *(_A*)arg[ 1], *(_A*)arg[ 2], *(_A*)arg[ 3], *(_A*)arg[ 4],
                *(_A*)arg[ 5], *(_A*)arg[ 6], *(_A*)arg[ 7], *(_A*)arg[ 8], *(_A*)arg[ 9],
                *(_A*)arg[10], *(_A*)arg[11], *(_A*)arg[12], *(_A*)arg[13], *(_A*)arg[14]
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
// OnEv<EV> [::react<Obj, &Obj::fun> | ...] ::type

#if 0
template<class _EV>
struct OnEv {
    typedef _EV EV;
    enum { has_target = false };
    enum { has_react  = false };
    enum { has_guard  = false };
    template<class Obj,
             typename handler_type<typename _EV::args_type>::template mem_fun<Obj>::type fun>
    struct react {
        typedef typename handler_type<typename _EV::args_type>::template mem_fun<Obj>::type react_fun_type;
        static react_fun_type react_fun_ptr () { return fun; }
        typedef _EV EV;
        enum { has_target = false };
        enum { has_react  = true  };
        enum { has_guard  = false };
    };
};
#else
template<class _EV>
struct OnEv {
    struct type {   // result
        typedef _EV EV;
        enum { has_target = false };
        enum { has_react  = false };
        enum { has_guard  = false };
    };
    typedef type _type; // alias

    template<class Obj,
             typename handler_type<typename _EV::args_type>::template mem_fun<Obj>::type fun>
    struct react : protected _type {    // protected to force client to put final ::type
        struct type {   // result
            typedef typename _type::EV EV;
            enum { has_target = _type::has_target };
            typedef typename handler_type<typename EV::args_type>::template mem_fun<Obj>::type react_fun_type;
            enum { has_react = true  };
            static react_fun_type react_fun_ptr () { return fun; }
            enum { has_guard = _type::has_guard };
        };
        typedef type _type; // alias

        ///XXX nested api(s) for OnEv/react ...
    };
};
#endif

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
            cout << "/*** calling " << __FUNCTION__
                                  << "(f=" << f << ", i=" << i << ") ***/" << endl;
            return f + i;
        }
};
void g2a (A1&, A2&) {}

// ***************************************************************************

#define EXPR_(expr) do { cout << #expr << " ==> " << (expr) << endl; } while(0)

typedef Cons<Kernel, Nil> CoePfx;

int main ()
{
    EXPR_((void*)Gfun::type(&g2a));
    EXPR_(sizeof(Gfun::type(&g2a)));
    // converting from 'Mfun::_R (Mfun::_Obj::*)()' to 'void*'
    // warning with gcc 4.4.3; error with gcc 4.1.1
    EXPR_(Mfun::type(&Cls::m2a));
    EXPR_(sizeof(Mfun::type(&Cls::m2a)));

    // === check/get degenerated pointer

    EXPR_((MFUN<void>        ::ptr(&Cls::m0a)));
    EXPR_((MFUN<void, CoePfx>::ptr(&Cls::h2a)));
    EXPR_((MFUN<void, CoePfx>::ptr(&Cls::h2A)));

    // === and then execute it

    float   val_f = 7;
    int     val_i = 6;
    void*   argfi[] = { &val_f, &val_i };

    Cls obj;

    EXPR_(execute<double>(MFUN<double>::ptr(&Cls::plus_fi),
                          2,
                          &obj,
                          argfi));

    EXPR_((Length<List2<A1, A2>::type>::value));

    // === explore alternative interface for transitions

    EXPR_((Length<OnEv<ev12>::react<Cls, &Cls::h2a>::type::EV::args_type>::value));
    EXPR_((OnEv<ev12>::react<Cls, &Cls::h2a>::type::has_target));
    EXPR_((OnEv<ev12>::react<Cls, &Cls::h2a>::type::has_react));
    EXPR_((OnEv<ev12>::react<Cls, &Cls::h2a>::type::react_fun_ptr()));
    EXPR_((OnEv<ev12>::react<Cls, &Cls::h2a>::type::has_guard));
}

