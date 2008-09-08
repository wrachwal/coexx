// coe-session--r4s.h

#ifndef __COE_SESSION__R4S_H
#define __COE_SESSION__R4S_H

// =======================================================================

struct r4Session {  //TODO: derive from... `Resource'

    Session*    _handle;

    SiD         _sid;
    void*       _heap;

    r4Kernel*   _kernel;
    r4Session*  _parent;
    //TODO:     _list_children;

    r4Session ();
    void release_resource ();
};

// =======================================================================

#endif

