// coe-config--d4c.h

#ifndef __COE_CONFIG__D4C_H
#define __COE_CONFIG__D4C_H

#include "coe-sys.h"    //TODO: <pthread.h> ?!

// =======================================================================
// d4Config

struct d4Config {

    static d4Config* instance ();

    pthread_key_t   key_d4t;

private:
    d4Config ();
    static d4Config* _instance;
};

// =======================================================================

#endif

