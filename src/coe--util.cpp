// coe--util.cpp

/*****************************************************************************
Copyright (c) 2008, 2009 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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

#include "coe--util.h"
#include "coe-sys-sync.h"

#include <cstdlib>      // malloc
#include <cxxabi.h>

using namespace std;
using namespace coe;

// ===========================================================================

static Mutex    g_Mutex_ABI;

string coe::demangle (const type_info& ti)
{
    Mutex::Guard    guard(g_Mutex_ABI);     // protect buf/len

    static char*    buf = NULL;
    static size_t   len = 0;

    if (0 == len) {
        if (NULL != (buf = (char*)malloc(256)))
            len = 256;
    }

    int status;
    const char* demangled = abi::__cxa_demangle(ti.name(), buf, &len, &status);
    if (NULL == demangled) {
        return string("bad-mangled(\"") + ti.name() + "\")";
    }

    return demangled;
}

