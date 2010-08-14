// sm2.cpp

// ===========================================================================
// header

#include "coe-hsm.h"

BEG__MACHINE_OR_STATE_(SM2, ())
    BEG__OR_STATE_(A)
        BEG__OR_STATE_(C)
            BEG__STATE_(C1)
            END__STATE_(C1)
            BEG__STATE_(C2)
            END__STATE_(C2)
        END__OR_STATE_(C)
        BEG__OR_STATE_(D)
            BEG__STATE_(D1)
            END__STATE_(D1)
            BEG__STATE_(D2)
            END__STATE_(D2)
        END__OR_STATE_(D)
    END__OR_STATE_(A)
    BEG__AND_STATE_(B)
        BEG__OR_STATE_(E)
            BEG__STATE_(E1)
            END__STATE_(E1)
            BEG__STATE_(E2)
            END__STATE_(E2)
        END__OR_STATE_(E)
        BEG__OR_STATE_H_(F, SHALLOW_HISTORY)
            BEG__STATE_(F1)
            END__STATE_(F1)
            BEG__STATE_(F2)
            END__STATE_(F2)
        END__OR_STATE_(F)
    END__AND_STATE_(B)
END__MACHINE_OR_STATE_()

// ===========================================================================
// source

DEF__MACHINE_OR_STATE_(::, SM2, ()) //XXX: `::' to avoid 'empty macro argument' warning :)
,   A(*this)
,   B(*this)
{
    machine().initialize();     // one have to remember to call this!
}

DEF__OR_STATE_(SM2::, A)
,   C(*this)
,   D(*this)
{
}

DEF__OR_STATE_(SM2::A::, C)
,   C1(*this)
,   C2(*this)
{
}

DEF__STATE_(SM2::A::C::, C1) {}
DEF__STATE_(SM2::A::C::, C2) {}

DEF__OR_STATE_(SM2::A::, D)
,   D1(*this)
,   D2(*this)
{
}

DEF__STATE_(SM2::A::D::, D1) {}
DEF__STATE_(SM2::A::D::, D2) {}

DEF__AND_STATE_(SM2::, B)
,   E(*this)
,   F(*this)
{
}

DEF__OR_STATE_(SM2::B::, E)
,   E1(*this)
,   E2(*this)
{
}

DEF__STATE_(SM2::B::E::, E1) {}
DEF__STATE_(SM2::B::E::, E2) {}

DEF__OR_STATE_(SM2::B::, F)
,   F1(*this)
,   F2(*this)
{
}

DEF__STATE_(SM2::B::F::, F1) {}
DEF__STATE_(SM2::B::F::, F2) {}

// ===========================================================================
// harness

#include "hsm-play.h"

REGISTER_EXAMPLE_STATE_MACHINE_(SM2)

