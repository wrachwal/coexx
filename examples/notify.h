// notify.h

#ifndef __NOTIFY_H
#define __NOTIFY_H

#include "coe-kernel.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <map>

// ===========================================================================

struct EventReceptor : public boost::shared_ptr<coe::Callback> {

    EventReceptor& callback (coe::Kernel& kernel,
                             const coe::CoeStr& ev,
                             coe::ValParam* pfx=0)
        {
            reset(kernel.callback(ev, pfx));
            return *this;
        }

    void unsubscribe ()
        {
            reset();
        }
};

// ---------------------------------------------------------------------------

template<typename Event>
class EventNotifier : boost::noncopyable {
public:

    EventNotifier () : _gc_thresh(1) {}

    typedef boost::shared_ptr<coe::Callback> Receptor;

    // receiver
    EventNotifier& subscribe (Event event, const Receptor& receptor);

    // publisher
    void notify_event (Event event, coe::Kernel& kernel, coe::EventArg* arg=0);

private:
    typedef std::multimap<Event, boost::weak_ptr<coe::Callback> > ERmap;
    ERmap   _receptors;
    size_t  _gc_thresh;
};

// ===========================================================================
// ///////////////////////////////////////////////////////////////////////////

#include <iostream>

template<typename Event>
EventNotifier<Event>& EventNotifier<Event>::subscribe (Event event, const Receptor& receptor)
    {
        if (receptor.get()) {

            boost::weak_ptr<coe::Callback>  r(receptor);
            typename ERmap::iterator    h = _receptors.find(event), i = h;

            while (i != _receptors.end() && (*i).first == event) {

                if (! ((*i).second < r) && ! (r < (*i).second)) {
                    return *this;
                }
                else
                if ((*i).second.expired()) {
                    if (i == h) {
                        _receptors.erase(i++);
                        h = i;
                    }
                    else
                        _receptors.erase(i++);
                }
                else {
                    ++i;
                }
            }

            _receptors.insert(h, typename ERmap::value_type(event, r));

            std::cout << "_receptors::size() -> " << _receptors.size() << std::endl;

            if (_receptors.size() > _gc_thresh) {

                typename ERmap::iterator    i = _receptors.begin();

                while (i != _receptors.end()) {
                    if ((*i).second.expired())
                        _receptors.erase(i++);
                    else
                        ++i;
                }

                _gc_thresh = 2 * _receptors.size();
            }
        }

        return *this;
    }

template<typename Event>
void EventNotifier<Event>::notify_event (Event event, coe::Kernel& kernel, coe::EventArg* arg)
    {
        boost::scoped_ptr<coe::EventArg>    __arg(arg);

        typename ERmap::iterator    i = _receptors.find(event);

        while (i != _receptors.end() && (*i).first == event) {

            boost::shared_ptr<coe::Callback>    locked_cb = (*i).second.lock();

            coe::Callback*  cb = locked_cb.get();

            if (0 == cb) {
                _receptors.erase(i++);
            }
            else {
                if (arg)
                    cb->call_keep_arg(kernel, *arg);
                else
                    cb->call(kernel);
                ++i;
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////

#endif

