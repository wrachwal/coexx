// coe-kernel--s4k.h

#ifndef __COE_KERNEL__S4K_H
#define __COE_KERNEL__S4K_H

#include "coe-session.h"

// =======================================================================

struct s4Kernel : public Session {

    r4Kernel*   _r4kernel;

    s4Kernel ();
    /*final*/ void _start (EvCtx& ctx);
};

// =======================================================================

#endif

