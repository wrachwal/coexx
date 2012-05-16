// funptr.cpp

#include <iostream>
#include "coe--local.h" // namespace meta
#include "coe-oev.h"

#include <typeinfo>
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

namespace detail {

template<class Result, int ApiSet> struct onev_apiset;

enum {
    api_EVENT = (1 << 0),
    api_GUARD = (1 << 1),
    api_DEFER = (1 << 2),
    api_TO    = (1 << 3),
    api_REACT = (1 << 4)
};

const int api_ONEV =
    api_GUARD
  | api_DEFER
  | api_TO
  | api_REACT
  ;

const int api_NOEV =
    api_GUARD
  | api_TO
  | api_REACT
  ;

// ------------------------------------
// guard<Obj, &Obj::fun>

template<class Result, int ApiSet, bool _Define = (0 != (ApiSet & api_GUARD))>
struct onev_api_guard {

    template<class Obj,
             typename /*FIXME*/handler_type<typename Result::EV::args_type>::template mem_fun<Obj>::type fun>
    struct _result : Result {
        enum { optset = Result::optset | api_GUARD };
        typedef typename /*FIXME*/handler_type<typename Result::EV::args_type>::template mem_fun<Obj>::type guard_fun_type;
        static guard_fun_type guard_fun_ptr () { return fun; }
    };
    enum { _apiset = ApiSet & ~api_GUARD };

    template<class Obj,
             typename /*FIXME*/handler_type<typename Result::EV::args_type>::template mem_fun<Obj>::type fun>
    struct guard : onev_apiset<_result<Obj, fun>, _apiset> {
        typedef _result<Obj, fun> type;
    };
};
template<class Result, int ApiSet>
struct onev_api_guard<Result, ApiSet, false> {};

// ------------------------------------
// defer

template<class Result, int ApiSet, bool _Define = (0 != (ApiSet & api_DEFER))>
struct onev_api_defer {

    struct _result : Result {
        enum { optset = Result::optset | api_DEFER };
    };
    enum { _apiset = ApiSet & ~(api_DEFER|api_TO|api_REACT) };

    struct defer : onev_apiset<_result, _apiset> {
        typedef _result type;
    };
};

// ------------------------------------
// to<State | List>

template<class Result, int ApiSet, bool _Define = (0 != (ApiSet & api_TO))>
struct onev_api_to {

    template<class To>
    struct _result : Result {
        enum { optset = Result::optset | api_TO };
        //TODO static check: single state | list of states. doable?
        typedef To target;
    };
    enum { _apiset = ApiSet & ~(api_TO|api_DEFER) };

    template<class To>
    struct to : onev_apiset<_result<To>, _apiset> {
        typedef _result<To> type;
    };
};
template<class Result, int ApiSet>
struct onev_api_to<Result, ApiSet, false> {};

// ------------------------------------
// react<Obj, &Obj::fun>

template<class Result, int ApiSet, bool _Define = (0 != (ApiSet & api_REACT))>
struct onev_api_react {

    template<class Obj,
             typename handler_type<typename Result::EV::args_type>::template mem_fun<Obj>::type fun>
    struct _result : Result {
        enum { optset = Result::optset | api_REACT };
        typedef typename handler_type<typename Result::EV::args_type>::template mem_fun<Obj>::type react_fun_type;
        static react_fun_type react_fun_ptr () { return fun; }
    };
    enum { _apiset = ApiSet & ~(api_REACT|api_DEFER) };

    template<class Obj,
             typename handler_type<typename Result::EV::args_type>::template mem_fun<Obj>::type fun>
    struct react : onev_apiset<_result<Obj, fun>, _apiset> {
        typedef _result<Obj, fun> type;
    };
};
template<class Result, int ApiSet>
struct onev_api_react<Result, ApiSet, false> {};

// ------------------------------------

template<class Result, int ApiSet>
struct onev_apiset
    :  onev_api_guard<Result, ApiSet>
    ,  onev_api_defer<Result, ApiSet>
    ,  onev_api_to   <Result, ApiSet>
    ,  onev_api_react<Result, ApiSet>
{
    typedef Result type;
};

// ---

struct _noev : event<> {};  // fictitious, allows to reuse onev_api_.. in (on|no)ev contexts

template<class _EV>
struct result_onev {
    typedef _EV EV;
    enum { optset = (meta::IsSame_<EV, _noev>::value ? 0 : api_EVENT) };
};

}   // namespace detail

// ------------------------------------
// OnEv<EV> [::onev-detail<..> ...] ::type
// OnEv<>   [::noev-detail<..> ...] ::type

template<class _EV = detail::_noev>
struct OnEv : detail::onev_apiset<detail::result_onev<_EV>, detail::api_ONEV> {
    typedef detail::result_onev<_EV> type;
};
template<>
struct OnEv<detail::_noev> : detail::onev_apiset<detail::result_onev<detail::_noev>, detail::api_NOEV> {
    typedef detail::result_onev<detail::_noev> type;
};

// ---------------------------------------------------------------------------

template<typename Pmf>
struct retval;
template<class R, class Obj, class A1, class A2>
struct retval<R (Obj::*)(A1&, A2&)> {
    typedef R type;
    typedef Obj obj_type;
};
template<class R, class Obj, class A1, class A2>
struct retval<R (Obj::*)(A1&, A2&) const> {
    typedef R type;
    typedef Obj obj_type;
};

// ---------------------------------------------------------------------------

struct A1 { virtual ~A1(){} char xx[10]; };
struct A2 {};

struct ev12 : event<A1, A2> {};

struct Cls {
    void m0a () {}
    void m2a (A1&, A2&) {}
    void m2A (A1&, A2&) const {}
    void h0a (Kernel&) {}
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

// ---------------------------------------------------------------------------

static void _strapp (string& str, const char* app, bool cond)
{
    if (cond) {
        if (! str.empty())
            str += ' ';
        str += app;
    }
}

static string ONEV (int optset)
{
    string  str;
    #define ONEV_API_(name) #name, (optset & detail::api_##name)
    _strapp(str, ONEV_API_(EVENT));
    _strapp(str, ONEV_API_(GUARD));
    _strapp(str, ONEV_API_(DEFER));
    _strapp(str, ONEV_API_(TO));
    _strapp(str, ONEV_API_(REACT));
    return '(' + str + ')';
}

// ===========================================================================

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
    cout << "@@@ OnEv<> -- transitions interface" << endl;

    EXPR_((Length<OnEv<>::type::EV::args_type>::value));
    EXPR_(ONEV(OnEv<>::type::optset));
    EXPR_((Length<OnEv<ev12>::type::EV::args_type>::value));
    EXPR_(ONEV(OnEv<ev12>::type::optset));
    EXPR_(ONEV(OnEv<ev12>::guard<Cls, &Cls::h2a>::type::optset));
    EXPR_(ONEV(OnEv<ev12>::react<Cls, &Cls::h2a>::type::optset));
    EXPR_(ONEV(OnEv<ev12>::guard<Cls, &Cls::h2a>::react<Cls, &Cls::h2a>::type::optset));
    EXPR_(ONEV(OnEv<ev12>::react<Cls, &Cls::h2a>::guard<Cls, &Cls::h2a>::type::optset));
    EXPR_((OnEv<ev12>::react<Cls, &Cls::h2a>::guard<Cls, &Cls::h2a>::type::react_fun_ptr()));
    EXPR_(ONEV(OnEv<ev12>::defer::type::optset));

    //XXX error: ‘react’ is not a member of ‘detail::onev_api_defer<detail::result_onev<ev12>, 14, true>::defer’
    //EXPR_(ONEV(OnEv<ev12>::defer::react<Cls, &Cls::h2a>::type::optset));

    EXPR_(ONEV(OnEv<>::to<A2>::type::optset));

    //XXX error: could not convert template argument ‘&Cls::h2a’ to ‘void (Cls::*)(coe::Kernel&)’
    //EXPR_(ONEV(OnEv<>::guard<Cls, &Cls::h2a>::to<A2>::type::optset));

    EXPR_(ONEV(OnEv<>::guard<Cls, &Cls::h0a>::to<A2>::type::optset));
    EXPR_(ONEV(OnEv<ev12>::to<A2>::type::optset));

    typedef retval<typeof(&Cls::m2a)>::type M2a;
    typedef retval<typeof(&Cls::m2A)>::type M2A;

    EXPR_(typeid(retval<typeof(&Cls::m2a)>::obj_type).name());
    EXPR_(typeid(retval<typeof(&Cls::m2A)>::obj_type).name());
    EXPR_(typeid(int).name());
    EXPR_(typeid(const int).name());
    EXPR_(typeid(int&).name());
    EXPR_(typeid(const int&).name());
    EXPR_(typeid(A1).name());
    EXPR_(typeid(const A1).name());
    EXPR_(typeid(A1&).name());
    EXPR_(typeid(const A1&).name());
    EXPR_(&typeid(A1));
    EXPR_(&typeid(const A1));
    EXPR_(&typeid(A1&));
    EXPR_(&typeid(const A1&));
    EXPR_(typeid(int).name());
    EXPR_(typeid(const int).name());
    EXPR_(typeid(int&).name());
    EXPR_(typeid(const int&).name());
    EXPR_(typeid(int*).name());
    EXPR_(typeid(const int*).name());
    EXPR_(typeid(int*&).name());
    EXPR_(typeid(const int*&).name());
}

