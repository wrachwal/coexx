// coe--errno.h

/*****************************************************************************
Copyright (c) 2008, 2009 Waldemar Rachwal <waldemar.rachwal@gmail.com>

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
*****************************************************************************/

#ifndef __COE__ERRNO_H
#define __COE__ERRNO_H

#include "coe-kernel--r4k.h"

namespace coe { /////

// ===========================================================================

inline bool session_alive (r4Session* r4s)
    {
        if (NULL == r4s || NULL == r4s->_handle
                /*FIXME ACCESS ISSUE: || NULL == r4s->_handle->_r4session*/) {
            //errno = ???   //TODO
            return false;
        }
        return true;
    }

inline bool kernel_attached (r4Kernel* r4k)
    {
        if (NULL == r4k || NULL == r4k->_thread) {
            //errno = ???  //TODO
            return false;
        }
        return true;
    }

inline bool current_session_active (r4Kernel* r4k)
    {
        if (   NULL == r4k
            || NULL == r4k->_current_context
            || NULL == r4k->_current_context->session
            || r4k->_current_context->session->local.stopper.isset())
        {
            //errno = ???
            return false;
        }
        return true;
    }

inline bool kernel_equal (r4Kernel* r4k, SiD on)
    {
        if (r4k->_kid != on.kid()) {
            //errno = ???   //TODO
            return false;
        }
        return true;
    }

inline bool target_valid (TiD target)
    {
        if (! target.isset()) {
            //errno = ???   //TODO
            return false;
        }
        return true;
    }

inline bool target_valid (SiD target)
    {
        if (! target.isset()) {
            //errno = ???   //TODO
            return false;
        }
        return true;
    }

inline bool user_evname (const std::string& ev)
    {
        if (ev.empty() || '.' == ev[0]) {
            //errno = ???   //TODO
            return false;
        }
        return true;
    }

inline bool delay_gt0 (const TimeSpec& ts)
    {
        if (ts <= TimeSpec::ZERO()) {
            //errno = ???   //TODO
            return false;
        }
        return true;
    }

inline bool fd_valid (int fd)
    {
        if (fd < 0 || fd >= FD_SETSIZE) {
            //errno = ???   //TODO
            return false;
        }
        return true;
    }

inline bool mode_valid (IO_Mode mode)
    {
        if (IO_read != mode && IO_write != mode && IO_error != mode) {
            //error = ???   //TODO
            return false;
        }
        return true;
    }

// ===========================================================================

} ///// namespace coe

#endif

