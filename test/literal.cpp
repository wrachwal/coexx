// literal.cpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>    // sort
#include <string>

#include <cstring>      // strncmp
#include <cassert>

using namespace std;

// ---------------------------------------------------------------------------

typedef unsigned long long  Addr;
typedef pair<Addr, Addr>    Region;     // [first, second)

static vector<Region>       SegmRO;

// ===========================================================================

static void read_proc_maps ()
{
    const char* proc = "/proc/self/maps";
    cout << "### reading (" << proc << ")" << endl;
    ifstream    ifs(proc);
    if (ifs.fail()) {
        cout << "!!! not supported by your system :(" << endl;
        return;
    }
    string  line;
    while (getline(ifs, line)) {
        cout << line << endl;
        istringstream   iss(line);
        string  range, perm;
        Addr    beg, end;
        char    dash;
        bool    match = false;
        if (iss >> range >> perm) {
            istringstream   irs(range);
            if (irs >> hex >> beg >> dash >> end && dash == '-') {
                assert(beg <= end);
                match = true;
            }
        }
        if (! match) {
            cout << "!!! [" << proc << "] bad syntax" << endl;
            continue;
        }
        if (0 == strncmp(perm.c_str(), "r-", 2)) {
            cout << "^^^ [" << beg << ".." << end << "] r-* :)" << endl;
            SegmRO.push_back(make_pair(beg, end));
        }
    }
/// reverse(SegmRO.begin(), SegmRO.end());
    sort(SegmRO.begin(), SegmRO.end());
}

static inline bool in_range (Addr addr, const Region& range)
{
    return addr >= range.first && addr < range.second;
}

bool is_ro_seg (const void *ptr)
{
    if (SegmRO.empty())
        return false;
    const Addr  addr = (Addr)ptr;
    const Region*   a = &SegmRO[0];
    const Region*   b = &SegmRO[SegmRO.size() - 1];
    while (b - a > 1) {
        const Region*   m = a + (b - a) / 2;
        if      (addr <  m-> first) b = m;
        else if (addr >= m->second) a = m;
        else    return true;
    }
    return in_range(addr, *a) || (a != b ? in_range(addr, *b) : false);
}

// ---------------------------------------------------------------------------

void check_seg (const char* ptr)
{
    cout << (is_ro_seg(ptr) ? "@ " : "- ") << (void*)ptr << " " << ptr << endl;
}

// ***************************************************************************

static       char un_st_buf[128];
             char un_ex_buf[128];
static       char in_st_buf[] = "{intialized} static char buf[*]";
             char in_ex_buf[] = "{intialized global} char buf[*]";
static const char cv_st_buf[] = "{intialized} static const char buf[*]";
       const char cv_ex_buf[] = "{intialized global} const char buf[*]";

// ---------------------------------------------------------------------------

int main ()
{
    read_proc_maps();

    if (! SegmRO.empty()) {
        cout << "### ------------ peeking upper byte at:" << endl;
        for (vector<Region>::iterator r = SegmRO.begin(); r != SegmRO.end(); ++r) {
            const char* last = (char*)(void*)(*r).second;
            cout << "### -1] " << (void*)(last - 1) << " = " << flush;
                          cout << int(*(last - 1)) << "(dec)" << endl;
            //
            // the following code is commented out because it SIGSEGV
            // due to <Region>.second is in fact "past a last valid" address.
            //
#if 0
            cout << "### +0] " << (void*)(last + 0) << " = " << flush;
                          cout << int(*(last + 0)) << "(dec)" << endl;
#endif
        }
    }

    cout << "### -----------------------------------" << endl;

    check_seg("C-string literal");

    char    stack[128];
    strcpy(stack, "on the stack (auto char buf[*])");
    check_seg(stack);

    string  cppstr;
    cppstr.assign("on the heap (std::string)");
    check_seg(cppstr.c_str());

    char*   heap = strdup("on the heap (strdup)");
    check_seg(heap);
    free(heap);
    heap = NULL;

    strcpy(un_st_buf, "{un-initialized} static char buf[*]");
    check_seg(un_st_buf);

    strcpy(un_ex_buf, "{un-initialized global} char buf[*]");
    check_seg(un_ex_buf);

    check_seg(in_st_buf);

    check_seg(in_ex_buf);

    check_seg(cv_st_buf);

    check_seg(cv_ex_buf);
}

