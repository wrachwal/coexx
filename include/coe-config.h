// coe-config.h

#ifndef __COE_CONFIG_H
#define __COE_CONFIG_H

// =======================================================================
// CoeConfig

struct CoeConfig {

    CoeConfig ();

    int timer_signo;    // reserved signal for timer manager
};

// -----------------------------------------------------------------------

bool coe_init (CoeConfig& cfg);

// =======================================================================

#endif

