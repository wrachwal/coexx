// coe-session.h

#ifndef __COE_SESSION_H
#define __COE_SESSION_H

#include "coe-kernel.h"

struct r4Session;

// =======================================================================
// Session

class Session {
public:
    SiD ID () const;

protected:
    Session ();
    virtual ~Session ();

    void  set_heap (void* heap);        // handlers will get it in EvCtx
    void* get_heap () const;

    /*
     * session management
     */
    virtual void _start (EvCtx& ctx) = 0;

private:
    Session (const Session&);           // prohibited
    void operator= (const Session&);    // prohibited

    friend struct r4Kernel;
    friend struct r4Session;
    r4Session*   _r4session;
};

// =======================================================================

#endif

