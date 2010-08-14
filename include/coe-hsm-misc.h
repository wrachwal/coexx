// coe-hsm-misc.h

#ifndef __COE_HSM_MISC_H
#define __COE_HSM_MISC_H

#include "coe-hsm.h"

#include <map>

namespace coe {
namespace hsm {

// ===========================================================================

void build_abbrev_to_state_map (std::map<std::string, aState*>& a2s,
                                aState& root, const char* sep = ".");
void state_to_least_abbrev_map (std::map<aState*, std::string>& s2a,
                                const std::map<std::string, aState*>& a2s);

// ===========================================================================

} ///// namespace hsm
} ///// namespace coe

#endif

