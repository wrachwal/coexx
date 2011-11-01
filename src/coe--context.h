// coe--context.h

/*****************************************************************************
Copyright (c) 2008-2011 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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

#ifndef __COE__CONTEXT_H
#define __COE__CONTEXT_H

#include "coe--event.h"

namespace coe { /////

struct r4Session;

// ===========================================================================
// ExecuteContext

struct ExecuteContext {

    explicit ExecuteContext (r4Kernel* kernel);

    ExecuteContext (r4Session* session,
                    EventContext::Type type,
                    const CoeStr& state,
                    r4Session* stopper = 0);

    ExecuteContext (r4Session* session,
                    EvUser*    event);

    ~ExecuteContext ();

    void        prefix (const _TypeDN* type, void* pval[]);
    void locked_prefix (ValParam* vp);

    void        argument (EventArg* ea);
    void locked_argument (ValParam* vp);

    bool execute (Kernel& kernel, const HandlerX& handler);

    // --------------------------------

    void*               magic;
    ExecuteContext*     parent;
    int                 level;
    EventContext::Type  type;

    r4Session*          session;
    CoeStr              state;
    SiD                 sender;
    CoeStr              sender_state;
    AiD                 alarm_id;

    Handler0            continuation;

    const _TypeDN*      pfx_type;
    void**              pfx_pval;
    ValParam*           pfx_locked;

    EventArg*           arg;
    ValParam*           arg_locked;

    void*               argptr[9 + 9];  // pfx + arg

private:
    void _print_stack (std::ostream& os);
    void _print_frame (std::ostream& os);
};

// ===========================================================================

} ///// namespace coe

#endif

