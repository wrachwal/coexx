// $Id$

/*************************************************************************
Copyright (c) 2008 Waldemar Rachwal <waldemar.rachwal@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*************************************************************************/

#ifndef __COE__LIST_H
#define __COE__LIST_H

#include <cstdlib>
#include <cassert>
#include <iterator>

namespace coe { /////

// =======================================================================

template<class T>
struct dLink {
    dLink () : next(NULL), prev(NULL) {}
    T*  next;
    T*  prev;
};

// -----------------------------------------------------------------------

template<class T, size_t LinkOffset>
struct dList {

    dList () : _head(NULL), _tail(NULL), _size(0) {}

    bool  empty () const { return 0 == _size; }
    size_t size () const { return _size; }

    void swap (dList& other);

    T* peek_head () const { return _head; }
    T* peek_tail () const { return _tail; }

    T* get_head ();
    T* get_tail ();

    void put_head (T* data);
    void put_tail (T* data);

    T* remove (T* data);

    enum { _LINK_OFFSET = LinkOffset }; // defined for debug purposes

    // --------------------------------

    class iterator;

    iterator begin () { return iterator(*this); }   // STL
    iterator end ()   { return iterator(); }        // STL

    class iterator : public std::iterator<std::forward_iterator_tag, T*> {
    public:
        iterator () : _data(0), _list(0) {}         // STL
        iterator (dList& list) : _data(list._head), _list(&list) {}
        operator bool () const { return 0 != _data; }
        operator T* () const { return _data; }
        T* operator-> () const { return _data; }
        T* operator* () const { return _data; }     // STL
        iterator& operator++ ()                     // STL
            {
                if (_data)
                    _move_next();
                return *this;
            }
        iterator operator++ (int)                   // STL
            {
                if (_data) {
                    iterator temp(*this);
                    _move_next();
                    return temp;
                }
                return *this;
            }
        bool operator== (const iterator& rhs) const // STL
            { return _data == rhs._data; }
        bool operator!= (const iterator& rhs) const // STL
            { return _data != rhs._data; }

    private:
        void _move_next ()
            {
                _data = _link(_data)->next;
                if (_data == _list->_head)
                    _data = 0;
            }
        T*      _data;
        dList*  _list;
    };

private:
    static dLink<T>* _link (T* data)
        {
            return reinterpret_cast<dLink<T>*>(reinterpret_cast<char*>(data) + LinkOffset);
            // return (dLink<T>*)((char*)data + LinkOffset);    // TODO: or this version?
        }
    static void _insert_element (T* data)
        {
            _link(data)->next =
            _link(data)->prev = data;
        }
    static void _insert_element (T* data, T* prev, T* next)
        {
            _link(data)->next = next;
            _link(data)->prev = prev;
            _link(prev)->next =
            _link(next)->prev = data;
        }
    static void _remove_element (T* data)
        {
            dLink<T>*   link = _link(data);
            T*   prev = link->prev;
            T*   next = link->next;
            _link(prev)->next = next;
            _link(next)->prev = prev;
            link->prev =
            link->next = NULL;
        }
    void _remove_thelast (T* data)
        {
            _link(data)->prev =
            _link(data)->next = NULL;
            _head =
            _tail = NULL;
        }

    // ------------
    T*      _head;
    T*      _tail;
    size_t  _size;
};

// -----------------------------------------------------------------------
// dList<void, 0> -- helpful specialization for recursive data types

template<>
struct dList<void, 0> {

    dList () : _head(NULL), _tail(NULL), _size(0) {}

    // This is important the specialization to have same layout of member data
    // ------------
    void*   _head;
    void*   _tail;
    size_t  _size;
};

// =======================================================================

template<class T, size_t LinkOffset>
void dList<T, LinkOffset>::swap (dList& other)
{
    std::swap(_head, other._head);
    std::swap(_tail, other._tail);
    std::swap(_size, other._size);
}

template<class T, size_t LinkOffset>
T* dList<T, LinkOffset>::get_head ()
{
    T*  data = _head;
    if (data) {
        if (1 == _size) {
            _remove_thelast(data);
        }
        else {
            _head = _link(data)->next;
            _remove_element(data);
        }
        -- _size;
    }
    return data;
}

template<class T, size_t LinkOffset>
T* dList<T, LinkOffset>::get_tail ()
{
    T*  data = _tail;
    if (data) {
        if (1 == _size) {
            _remove_thelast(data);
        }
        else {
            _tail = _link(data)->prev;
            _remove_element(data);
        }
        -- _size;
    }
    return data;
}

template<class T, size_t LinkOffset>
void dList<T, LinkOffset>::put_head (T* data)
{
    if (data) {
        assert(NULL == _link(data)->next);
        assert(NULL == _link(data)->prev);
        if (_head) {
            _insert_element(data, _tail, _head);
            _head = data;
        }
        else {  // empty list
            _insert_element(data);
            _head = _tail = data;
        }
        ++ _size;
    }
}

template<class T, size_t LinkOffset>
void dList<T, LinkOffset>::put_tail (T* data)
{
    if (data) {
        assert(NULL == _link(data)->next);
        assert(NULL == _link(data)->prev);
        if (_head) {
            _insert_element(data, _tail, _head);
            _tail = data;
        }
        else {  // empty list
            _insert_element(data);
            _head = _tail = data;
        }
        ++ _size;
    }
}

template<class T, size_t LinkOffset>
T* dList<T, LinkOffset>::remove (T* data)
{
    if (data) {
        // these easy checks are not able to verify list membership
        assert(_link(_link(data)->prev));
        assert(_link(_link(data)->next));
        assert(_link(_link(data)->prev)->next == data);
        assert(_link(_link(data)->next)->prev == data);
        // therefore the following can show up inconsistency later
        assert(_size > 0);

        if (1 == _size) {
            _remove_thelast(data);
        }
        else {
            if (_head == data)
                _head = _link(data)->next;
            if (_tail == data)
                _tail = _link(data)->prev;
            _remove_element(data);
        }
        -- _size;
    }
    return data;
}

// =======================================================================

} ///// namespace coe

#endif

