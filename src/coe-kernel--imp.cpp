// coe-kernel--imp.cpp

/*****************************************************************************
Copyright (c) 2008-2019 Waldemar Rachwał <waldemar.rachwal@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*****************************************************************************/

#include "coe-kernel.h"
#include "coe--util.h"

#include <iostream> // debug
#include <cassert>

using namespace std;
using namespace coe;

#if 0
// ===========================================================================
// _TypeD

const _TypeD* _TypeD::_register (_TypeD* type)
{
    static _TypeD*  head = NULL;

    if (NULL != type) {
        assert(NULL == type->next);
        type->pos  = head ? head->pos + 1 : 0;
        type->next = head;
        head = type;
    }

    return head;
}

// ===========================================================================
// _TypeDN

const _TypeDN* _TypeDN::_register (_TypeDN* type)
{
    static _TypeDN* head = NULL;

    if (NULL != type) {
        assert(NULL == type->next);
        type->pos  = head ? head->pos + 1 : 0;
        type->next = head;
        head = type;
    }

    return head;
}

// ------------------------------------
#endif

bool coe::syntax_check (const ArgListI* hT,
                        const ArgListI* xT,
                        const ArgListI* aT)
{
    if (NULL == xT) {
        return (hT == aT);
    }
    if (NULL == aT) {
        return (hT == xT);
    }
    if (NULL == hT || hT->len != xT->len + aT->len) {
        return false;
    }

    size_t i = 0;
    while (i < xT->len) {
        if (hT->arg[i]->type != xT->arg[i]->type) {
            return false;
        }
        ++i;
    }
    for (size_t j = 0; j < aT->len; ++j, ++i) {
        if (hT->arg[i]->type != aT->arg[j]->type) {
            return false;
        }
    }
    return true;
}

// ===========================================================================
// EventArg

EventArg::~EventArg ()
{
}

// ===========================================================================
// ValParam

ValParam::~ValParam ()
{
    assert(! _locked);
}

void ValParam::destroy ()
{
    if (! _locked) {
        delete this;
    }
    else {
        assert(_locked > 0);
        _locked = - _locked;
    }
}

// ===========================================================================
// HandlerX

void HandlerX::execute (Kernel& kernel, void* arg[]) const
{
    if (_obj) {
        if (NULL == _tdn) {
            (_obj->*_fun.m0)(kernel);
        }
        else {
            switch (_tdn->len) {
                case 1:
                    (_obj->*_fun.m1)(
                        kernel,
                        *(A*)arg[0]
                    );
                    break;
                case 2:
                    (_obj->*_fun.m2)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1]
                    );
                    break;
                case 3:
                    (_obj->*_fun.m3)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2]
                    );
                    break;
                case 4:
                    (_obj->*_fun.m4)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2],
                        *(A*)arg[3]
                    );
                    break;
                case 5:
                    (_obj->*_fun.m5)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2],
                        *(A*)arg[3],
                        *(A*)arg[4]
                    );
                    break;
                case 6:
                    (_obj->*_fun.m6)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2],
                        *(A*)arg[3],
                        *(A*)arg[4],
                        *(A*)arg[5]
                    );
                    break;
                case 7:
                    (_obj->*_fun.m7)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2],
                        *(A*)arg[3],
                        *(A*)arg[4],
                        *(A*)arg[5],
                        *(A*)arg[6]
                    );
                    break;
                case 8:
                    (_obj->*_fun.m8)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2],
                        *(A*)arg[3],
                        *(A*)arg[4],
                        *(A*)arg[5],
                        *(A*)arg[6],
                        *(A*)arg[7]
                    );
                    break;
                case 9:
                    (_obj->*_fun.m9)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2],
                        *(A*)arg[3],
                        *(A*)arg[4],
                        *(A*)arg[5],
                        *(A*)arg[6],
                        *(A*)arg[7],
                        *(A*)arg[8]
                    );
                    break;
            }
        }
    }
    else {
        if (NULL == _tdn) {
            (*_fun.g0)(kernel);
        }
        else {
            switch (_tdn->len) {
                case 1:
                    (*_fun.g1)(
                        kernel,
                        *(A*)arg[0]
                    );
                    break;
                case 2:
                    (*_fun.g2)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1]
                    );
                    break;
                case 3:
                    (*_fun.g3)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2]
                    );
                    break;
                case 4:
                    (*_fun.g4)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2],
                        *(A*)arg[3]
                    );
                    break;
                case 5:
                    (*_fun.g5)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2],
                        *(A*)arg[3],
                        *(A*)arg[4]
                    );
                    break;
                case 6:
                    (*_fun.g6)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2],
                        *(A*)arg[3],
                        *(A*)arg[4],
                        *(A*)arg[5]
                    );
                    break;
                case 7:
                    (*_fun.g7)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2],
                        *(A*)arg[3],
                        *(A*)arg[4],
                        *(A*)arg[5],
                        *(A*)arg[6]
                    );
                    break;
                case 8:
                    (*_fun.g8)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2],
                        *(A*)arg[3],
                        *(A*)arg[4],
                        *(A*)arg[5],
                        *(A*)arg[6],
                        *(A*)arg[7]
                    );
                    break;
                case 9:
                    (*_fun.g9)(
                        kernel,
                        *(A*)arg[0],
                        *(A*)arg[1],
                        *(A*)arg[2],
                        *(A*)arg[3],
                        *(A*)arg[4],
                        *(A*)arg[5],
                        *(A*)arg[6],
                        *(A*)arg[7],
                        *(A*)arg[8]
                    );
                    break;
            }
        }
    }
}

// ===========================================================================
// Handler0

void Handler0::execute (Kernel& kernel) const
{
    if (_obj) {
        (_obj->*_fun.m0)(kernel);
    }
    else {
        (*_fun.g0)(kernel);
    }
}

