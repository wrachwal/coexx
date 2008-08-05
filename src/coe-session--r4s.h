// coe-session--r4s.h

#ifndef __COE_SESSION__R4S_H
#define __COE_SESSION__R4S_H

// =======================================================================

struct r4Session {  //TODO: derive from... `Resource'

    Session*    _handle;

    SiD         _sid;
    void*       _heap;

    r4Kernel*   _kernel;    //FIXME: do we need this? if so, not Kernel*?
    r4Session*  _parent;
    r4Session*  _children_list;

    r4Session ();
    void release_resource ();
};

// =======================================================================

#endif

