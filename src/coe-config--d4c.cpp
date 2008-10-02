// $Id$

/*************************************************************************
Copyright (c) 2008 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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
*************************************************************************/

#include "coe-config--d4c.h"

#include <cstdio>   // perror
#include <cstdlib>  // abort

using namespace std;
using namespace coe;

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

