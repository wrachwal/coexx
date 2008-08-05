// coe-kernel--dcl.cpp

#include "coe-kernel.h"

#include <iostream> // debug

#include <cxxabi.h>

using namespace std;

// -----------------------------------------------------------------------

//TODO:
// `demangle' currently is not thread-safe
// intoduce critical section to make it so.
string demangle (const type_info* ti)
{
    static char* buf = NULL;
    static size_t n = 0;
    if (0 == n) {
        if (NULL != (buf = (char*)malloc(256)))
            n = 256;
    }
    int status;
    const char* demangled = abi::__cxa_demangle(ti->name(), buf, &n, &status);
    if (NULL == demangled) {
        return string("bad-mangled(\"") + ti->name() + "\")";
    }
    return demangled;
}

// =======================================================================
// EventArg

EventArg::~EventArg ()
{
}

// =======================================================================
// StateCmd

StateCmd::~StateCmd ()
{
}

static void
report_arg_mismatch (ostream& os,
                     int xN,
                     int cj,
                     const type_info* at,
                     const type_info* ct
                    )
{
    os << "argument #" << (cj + 1) << " type mismatch"
       << " (handler: `" << demangle(ct)
       << "', event: `"  << demangle(at) << "')" << endl;
}

bool StateCmd::execute (EvCtx& ctx, const ArgTV* xA, int xN, EventArg* arg)
{
    int eN = 0;
    const ArgTV*    eA = arg ? arg->arg_list(eN) : NULL;

    int cN;
    ArgTV*          cA =      this->arg_list(cN);

    if (xN + eN != cN) {
        cout << "handler has " << cN << " args, while event ("
                               << xN << "+" << eN << ") args" << endl;
        return false;
    }

    int cj = 0;

    //
    // cA <-- xA
    //
    for (int xj = 0; xj < xN; ++xj, ++cj) {
        if (*xA[xj]._ti != *cA[cj]._ti) {
            report_arg_mismatch(cout, xN, cj, xA[xj]._ti, cA[cj]._ti);
            return false;
        }
        cA[cj]._pv = xA[xj]._pv;
    }

    //
    // cA <-- eA
    //
    for (int ej = 0; ej < eN; ++ej, ++cj) {
        if (*eA[ej]._ti != *cA[cj]._ti) {
            report_arg_mismatch(cout, xN, cj, eA[ej]._ti, cA[cj]._ti);
            return false;
        }
        cA[cj]._pv = eA[ej]._pv;
    }

    //
    // `_execute()' will use (_arg[*]._pv) pointers to values
    //              just set in the previous step.
    //
    _execute(ctx);

    return true;
}

