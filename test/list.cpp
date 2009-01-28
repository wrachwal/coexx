// list.cpp

#include "../src/coe--list.h"

#include <string>

#include <vector>

#include <iostream>
#include <assert.h>

using namespace std;
using namespace coe;

// ===========================================================================

class Event {
public:
    Event (const string& name) : _name(name) {}
    virtual ~Event ()
        {
            assert(NULL == _link_active.prev);
            assert(NULL == _link_active.next);
        }
    const string& name () const { return _name; }

    void _print_link ()
        {
            cout << "{ prev = " << _link_active.prev
                 << ", next = " << _link_active.next
                 << " }";
        }
private:
    string          _name;
    friend struct EventList;
    dLink<Event>    _link_active;
};

// ------------------------------------

struct EventList {
    typedef dList<Event, offsetof(Event, _link_active)> Active;
};

// ===========================================================================

static const char* g_Str[] = {
    "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten",
    NULL    // <--- terminator
};

static void print_list (const char* prefix, EventList::Active& list)
{
    cout << prefix;
    for (EventList::Active::iterator iter(list); iter; ++iter)
        cout << ' ' << iter->name();
    cout << " (" << list.size() << ')' << endl;
}

static int fill_list (EventList::Active& list)
{
    cout << "##\n## list.put_tail(new Event(n))\n##" << endl;
    print_list("++", list);

    int max = 0;
    while (NULL != g_Str[max]) {
        list.put_tail(new Event(g_Str[max++]));
        print_list("++", list);
    }
    return max;
}

// ***************************************************************************

int main ()
{
    EventList::Active   list;
    int max = fill_list(list);

    cout << "##\n## list.put_tail(list.get_head())\n##" << endl;

    for (int i = 0; i < max; ++i) {
        list.put_tail(list.get_head());
        print_list("<-", list);
    }

    cout << "##\n## list.put_head(list.get_tail())\n##" << endl;

    for (int i = 0; i < max; ++i) {
        list.put_head(list.get_tail());
        print_list("->", list);
    }

    cout << "##\n## copy(list.begin(), list.end(), back_inserter(stl_vec))\n##" << endl;
    {
#if 1
        Event* pEv = *list.begin();
        // *(list.begin()) = pEv;       // error: invalid lvalue in assignment
        pEv = pEv;
#endif

        vector<Event*>  stl_vec;
        copy(list.begin(), list.end(), back_inserter(stl_vec));

        cout << ":)";
        for (vector<Event*>::iterator i = stl_vec.begin(); i != stl_vec.end(); ++i)
            cout << ' ' << (*i)->name();
        cout << endl;
    }

    cout << "##\n## list.remove(iter++); iter++\n##" << endl;
    {
        EventList::Active::iterator iter = list.begin();
        do {
            list.remove(iter++);
            iter++;
            print_list("~~", list);
        } while(iter);
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    {
        EventList::Active   list;   // shadow earlier variable

        fill_list(list);

        EventList::Active::iterator iter = list.begin();

        cout << "!!\n"
             << "!! BEWARE !!! INVALID USAGE\n"
             << "!! ----------------------------------------\n"
             << "!!     list.remove(iter);\n"
             << "!!     if (iter)   // PASSES THIS CHECK !!!\n"
             << "!!         iter->_print_link()\n"
             << "!! ----------------------------------------\n"
             << "!!     OUTPUT : ";

        list.remove(iter);
        if (iter) {
            iter->_print_link();
        }
        cout << endl
             << "!! LATER USAGE COULD RESULT IN SEGFAULT !!!\n"
             << "!!\n"
             << "!! REMEMBER THE FORM WHICH ** WORKS ** :\n"
             << "!!     list.remove(iter++); // postincrement\n"
             << "!!\n";
    }
}

