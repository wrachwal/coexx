// coe-config.h

#ifndef __COE_CONFIG_H
#define __COE_CONFIG_H

// =======================================================================
// CoeConfig

struct CoeConfig {

    CoeConfig ();

    int time_signo;     // signal notification on timer expiry
    int kick_signo;     // signal to break blocked I/O
};

// -----------------------------------------------------------------------

bool coe_init (CoeConfig& cfg);

// =======================================================================

#endif

