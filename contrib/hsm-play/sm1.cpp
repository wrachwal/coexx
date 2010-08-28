// sm1.cpp

// ===========================================================================
// header

#include "coe-hsm.h"

BEG__MACHINE_OR_STATE_(SM1, ())
    BEG__OR_STATE_(L2)
        BEG__OR_STATE_(L3)
            BEG__OR_STATE_(L4)
                BEG__OR_STATE_(L5)
                    BEG__OR_STATE_(L6)
                        BEG__OR_STATE_(L7)            ON_EX // --> L4
                            BEG__OR_STATE_(L8)
                                BEG__OR_STATE_(L9)
                                    // xor-state without any nested states
                                    // will internally behave as plain state.
                                END__OR_STATE_(L9)
                            END__OR_STATE_(L8)
                        END__OR_STATE_(L7)
                    END__OR_STATE_(L6)
                END__OR_STATE_(L5)
            END__OR_STATE_(L4)
        END__OR_STATE_(L3)
    END__OR_STATE_(L2)
END__MACHINE_OR_STATE_()

// ===========================================================================
// source

#include <iostream>

using namespace std;
using namespace coe;

DEF__MACHINE_OR_STATE_(::, SM1, ()) //XXX: `::' to avoid 'empty macro argument' warning :)
,   L2(*this)
{
    machine().initialize();     // one have to remember to call this!
}

DEF__OR_STATE_(SM1::, L2)
,   L3(*this)
{
}

DEF__OR_STATE_(SM1::L2::, L3)
,   L4(*this)
{
}

DEF__OR_STATE_(SM1::L2::L3::, L4)
,   L5(*this)
{
}

DEF__OR_STATE_(SM1::L2::L3::L4::, L5)
,   L6(*this)
{
}

DEF__OR_STATE_(SM1::L2::L3::L4::L5::, L6)
,   L7(*this)
{
}

DEF__OR_STATE_(SM1::L2::L3::L4::L5::L6::, L7)
,   L8(*this)
{
}

void SM1::L2::L3::L4::L5::L6::L7::on_ex (Kernel& kernel)
{
    L4& jumpto = context<L4>();

    cout << "(* inside " << state_name() << "::" << __FUNCTION__ << "()"
         << " transition to " << jumpto.state_name() << " *)" << endl;

    jumpto.transition_to(kernel);
}

DEF__OR_STATE_(SM1::L2::L3::L4::L5::L6::L7::, L8)
,   L9(*this)
{
}

DEF__OR_STATE_(SM1::L2::L3::L4::L5::L6::L7::L8::, L9)
{
}

// ===========================================================================
// harness

#include "hsm-play.h"

REGISTER_EXAMPLE_STATE_MACHINE_(SM1)

