// hsm-play.h

#ifndef __HSM_PLAY_H
#define __HSM_PLAY_H

namespace coe { namespace hsm { class aComposition; } }

// ===========================================================================

struct RegisterExampleStateMachine {
    RegisterExampleStateMachine (const char* type, coe::hsm::aComposition* (*creator)());
};

#define REGISTER_EXAMPLE_STATE_MACHINE_(Type)                   \
    static coe::hsm::aComposition* create_sm_ ## Type ()  {     \
        return new Type(); }                                    \
    static RegisterExampleStateMachine                          \
        req_sm_ ## Type (#Type, &create_sm_##Type);

// ===========================================================================

#endif

