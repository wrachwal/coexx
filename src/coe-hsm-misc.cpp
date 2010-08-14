// coe-hsm-misc.cpp

#include "coe-hsm-misc.h"

#include <set>

namespace coe {
namespace hsm {

using namespace std;

// ---------------------------------------------------------------------------

namespace {

    class StateAbbrev : public StateVisitor {
    public:
        StateAbbrev (map<string, aState*>& abbr, const char* sep) : _abbr(abbr), _sep(sep)
            { _abbr.clear(); }
        void visit (BasicState& state) { _add(state); }
        void visit (  OR_State& state) { _add(state); }
        void visit ( AND_State& state) { _add(state); }
    private:
        void _add (aState& state);
        map<string, aState*>&   _abbr;
        string                  _sep;
        set<string>             _lock;
    };

}   /// namespace

// ------------------------------------

void StateAbbrev::_add (aState& state)
{
    string  name;

    for (aState* node = &state; node != NULL; node = node->parent_()) {

        name = name.empty() ? string(node->state_name())
                            : string(node->state_name()) + _sep + name;

        if (NULL == node->parent_()) {  //  absolute path name
            _abbr[name] = &state;
            _lock.insert(name);
        }
        else if (! _lock.count(name)) {
            if (_abbr.count(name)) {    // duplicate found
                _abbr.erase(name);
                _lock.insert(name);
            }
            else {                      // not used so far
                _abbr[name] = &state;
            }
        }
    }
}

// ------------------------------------

void build_abbrev_to_state_map (map<string, aState*>& a2s, aState& root, const char* sep)
{
    StateAbbrev namer(a2s, sep);
    walk_top_down(root, namer);
}

void state_to_least_abbrev_map (map<aState*, string>& s2a, const map<string, aState*>& a2s)
{
    s2a.clear();
    for (map<string, aState*>::const_iterator i = a2s.begin(); i != a2s.end(); ++i) {
        string& abbr = s2a[(*i).second];
        if (abbr.length() < (*i).first.length()) {
            abbr = (*i).first;
        }
    }
}

// ===========================================================================

} ///// namespace hsm
} ///// namespace coe

