// typeinfo.cpp

#include <typeinfo>
#include <vector>
#include <list>
#include <iostream>
#include <algorithm>

using namespace std;

#define TABLEN(tab)     (int(sizeof(tab) / sizeof((tab)[0])))

extern string demangle (const type_info* ti);

// -----------------------------------------------------------------------

struct TI {
    TI () : info(0) {}
    TI (const char* t, const type_info* i) : type(t), info(i)
        {
            dmgl = demangle(info);
        }
    string              type;   // type (as input string)
    const type_info*    info;   // RTTI
    string              dmgl;   // demangled type
};

template<class T>
TI make_ti (const char* name)
{
    return TI(name, &typeid(T));
}
#define make_TI(type)   make_ti<type>(#type)

// ------------------------------------

struct before_order : public binary_function<TI, TI, bool> {
    bool operator() (const TI& lhs, const TI& rhs) const {
        return lhs.info->before(*rhs.info);
    }
};

// -----------------------------------------------------------------------

static int s_type_length = 0;
static int s_name_length = 0;
static int s_dmgl_length = 0;

ostream& operator<< (ostream& os, const TI& info)
{
    int pad;

    os << '"' << info.type << '"';
    if ((pad = s_type_length - info.type.length()) > 0)
        os << string(pad, ' ');

    os << " -->" << info.info->name() << "<--";
    if ((pad = s_name_length - strlen(info.info->name())) > 0)
        os << string(pad, ' ');

    os << '"' << info.dmgl << '"';
    if ((pad = s_dmgl_length - info.dmgl.length()) > 0)
        os << string(pad, ' ');

    os << "  " << info.info;

    return os;
}

// -----------------------------------------------------------------------

class Udt {
public:
    Udt ();
    int a, b, c;
};

// =======================================================================

void type_info_show ()
{
    TI  Tinfo[] = {
        make_TI(void),
        make_TI(char),
        make_TI(unsigned char),
        make_TI(signed char),
        make_TI(short),
        make_TI(unsigned short),
        make_TI(int),
        make_TI(unsigned int),
        make_TI(long),
        make_TI(unsigned long),
        //FIXME: g++ 3.4.4 error: ISO C++ does not support `long long'
        //make_TI(long long),
        //make_TI(unsigned long long),
        make_TI(float),
        make_TI(double),
        make_TI(long double),

        make_TI(int* const),
        make_TI(int**),
        make_TI(int**&),
        make_TI(const int**&),
        make_TI(const int**),

        make_TI(char[10]),
        make_TI(int[10]),
        make_TI(int[6][9]),

        make_TI(int),
        make_TI(const int),
        make_TI(int&),
        make_TI(const int&),

        make_TI(Udt),
        make_TI(const Udt),
        make_TI(Udt&),
        make_TI(const Udt&),

        make_TI(int*),
        make_TI(const int*),
        make_TI(int*&),
        make_TI(const int*&),

        make_TI(Udt*),
        make_TI(const Udt*),
        make_TI(Udt*&),
        make_TI(const Udt*&),

        make_TI(void(*)()),
        make_TI(void(*)(int)),
        make_TI(void(*)(const int)),
        make_TI(void(*)(int&)),
        make_TI(void(*)(int*)),
        make_TI(void(*)(const int*)),
        make_TI(int(*)(int)),
        make_TI(int(*)(int&)),
        make_TI(void(*)(long double)),
        make_TI(void(*)(long double&)),

        make_TI(string),
        make_TI(vector<int>),
        make_TI(vector<int*>),
        make_TI(list<int>),
        make_TI(list<int*>),

        make_ti<pair<int,int> >("pair<int,int>"),
        make_ti<pair<int,pair<int,int> > >("pair<int,pair<int,int> >"),
        make_ti<pair<int,pair<int,int> >&>("pair<int,pair<int,int> >&"),
        make_ti<auto_ptr<Udt> >("auto_ptr<Udt>")
    };

    // calculate max. lengths for best printout
    for (int i = 0; i < TABLEN(Tinfo); ++i) {
        int len;
        const TI*   entry = &Tinfo[i];
        if ((len = entry->type.length()) > s_type_length)
            s_type_length = len;
        if ((len = strlen(entry->info->name())) > s_name_length)
            s_name_length = len;
        if ((len = entry->dmgl.length()) > s_dmgl_length)
            s_dmgl_length = len;
    }

    cout << "original input order:" << endl;
    for (int i = 0; i < TABLEN(Tinfo); ++i)
        cout << "  - " << Tinfo[i] << endl;
    cout << endl;

    sort(Tinfo, Tinfo+TABLEN(Tinfo), before_order());

    cout << "type_info::before order:" << endl;
    for (int i = 0; i < TABLEN(Tinfo); ++i)
        cout << "  - " << Tinfo[i] << endl;
    cout << endl;

    bool    Tused [TABLEN(Tinfo)];
    for (int i = 0; i < TABLEN(Tused); ++i)
        Tused[i] = false;

    cout << "groupped by type_info::operator==:" << endl;
    for (int k = 0; k < TABLEN(Tinfo); ++k) {
        if (Tused[k])
            continue;
        for (int i = k; i < TABLEN(Tinfo); ++i) {
            if (! Tused[i] && *Tinfo[k].info == *Tinfo[i].info) {
                if (i == k)
                    cout << "  -" << endl;
                cout << "    - " << Tinfo[i] << endl;
                Tused[i] = true;
            }
        }
    }
    cout << endl;
}

