// coe--res.h

#ifndef __COE__RES_H
#define __COE__RES_H

// =======================================================================
// Resource

class Resource {
public:
    enum Type {
        KERNEL  = 0,    // r4Kernel
        SESSION = 1,    // r4Session
        IO_POLL = 2,    // IOWatcher
        TIMER   = 3,    // Timer
        WHEEL   = 4,    // r4Wheel
        MESSAGE = 5     // EvMessage
    };

    // static operator new/delete...
    virtual void ~Resource ();

    virtual Type res_type () const = 0;
};

// -----------------------------------------------------------------------
// IOWatcher

class IOWatcher : public Resource {
public:
    IOWatcher (int fd, IO_mode mode);
};

// =======================================================================

#endif

