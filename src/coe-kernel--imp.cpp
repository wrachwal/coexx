// coe-kernel--imp.cpp

#include "coe-kernel.h"

using namespace std;

// =======================================================================
// MFunCmd0 ... MFunCmd5

ArgTV* MFunCmd0::arg_list (int& len)
{
    len = 0;
    return NULL;
}

void MFunCmd0::_execute (EvCtx& ctx) const
{
    (_obj->*_memfun)(ctx);
}

void MFunCmd1::_execute (EvCtx& ctx) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)_arg[0]._pv
    );
}

void MFunCmd2::_execute (EvCtx& ctx) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv
    );
}

void MFunCmd3::_execute (EvCtx& ctx) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv,
        *(_Arg*)_arg[2]._pv
    );
}

void MFunCmd4::_execute (EvCtx& ctx) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv,
        *(_Arg*)_arg[2]._pv,
        *(_Arg*)_arg[3]._pv
    );
}

void MFunCmd5::_execute (EvCtx& ctx) const
{
    (_obj->*_memfun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv,
        *(_Arg*)_arg[2]._pv,
        *(_Arg*)_arg[3]._pv,
        *(_Arg*)_arg[4]._pv
    );
}

// =======================================================================
// GFunCmd0 ... GFunCmd5

ArgTV* GFunCmd0::arg_list (int& len)
{
    len = 0;
    return NULL;
}

void GFunCmd0::_execute (EvCtx& ctx) const
{
    (*_fun)(ctx);
}

void GFunCmd1::_execute (EvCtx& ctx) const
{
    (*_fun)(
        ctx,
        *(_Arg*)_arg[0]._pv
    );
}

void GFunCmd2::_execute (EvCtx& ctx) const
{
    (*_fun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv
    );
}

void GFunCmd3::_execute (EvCtx& ctx) const
{
    (*_fun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv,
        *(_Arg*)_arg[2]._pv
    );
}

void GFunCmd4::_execute (EvCtx& ctx) const
{
    (*_fun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv,
        *(_Arg*)_arg[2]._pv,
        *(_Arg*)_arg[3]._pv
    );
}

void GFunCmd5::_execute (EvCtx& ctx) const
{
    (*_fun)(
        ctx,
        *(_Arg*)_arg[0]._pv,
        *(_Arg*)_arg[1]._pv,
        *(_Arg*)_arg[2]._pv,
        *(_Arg*)_arg[3]._pv,
        *(_Arg*)_arg[4]._pv
    );
}

// -----------------------------------------------------------------------

StateCmd* handler (void (*fun)(EvCtx&))
{
    return new GFunCmd0(fun);
}

