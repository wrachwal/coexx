// coe-config--d4c.cpp

#include "coe-config--d4c.h"

#include <cstdio>   // perror
#include <cstdlib>  // abort

using namespace std;

// -----------------------------------------------------------------------

static Mutex       s_instance_mutex;
d4Config* d4Config::_instance = NULL;

// =======================================================================
// d4Config

d4Config::d4Config ()
{
}

d4Config* d4Config::instance ()
{
    if (NULL == _instance) {

        Mutex::Guard    guard(::s_instance_mutex);

        if (NULL == _instance) {

            d4Config*   d4c = new d4Config;

            //TODO: in case of errors here, throw exception!

            int status = pthread_key_create(&d4c->key_d4t, NULL); //TODO: add destructor
            if (status != 0) {
                //TODO: better error-handling
                perror("pthread_key_create");
                abort();
            }

            _instance = d4c;
        }
    }

    return _instance;
}

