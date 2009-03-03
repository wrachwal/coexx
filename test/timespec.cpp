// timespec.cpp

#include "coe-kernel.h"

#include <iostream>

using namespace std;
using namespace coe;

// ***************************************************************************

#define TS(delay)                                                       \
    do {                                                                \
        TimeSpec    ts(delay);                                          \
        cout << "TimeSpec(" #delay ") -> " << ts                        \
             << " {" << ts.tv_sec << "," << ts.tv_nsec << "}" << endl;  \
    } while(0)

int main ()
{
    TS(0.0);
    TS(0.01);
    TS(-0.01);
    TS(0.1);
    TS(-0.1);
    TS(1.9);
    TS(-1.9);
    TS(2.0);
    TS(-2.0);
    TS(10.0);
    TS(-10.0);
}

