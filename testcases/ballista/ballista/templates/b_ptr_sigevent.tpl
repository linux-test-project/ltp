// b_ptr_sigevent.tpl : Ballista Datatype Template for signal event pointer
// Copyright (C) 1998-2001  Carnegie Mellon University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

name structsigeventptr b_ptr_sigevent;

parent b_ptr_buf;

includes
[
{
#include <signal.h>
#include "b_ptr_buf.h"
#define structsigeventptr struct sigevent*
}
]

global_defines
[
{
  static struct sigevent sigEvent;
}
]

dials
[
  enum_dial NOTIFY :
     NOTIFY_ZERO,
     NOTIFY_MAXINT,
     NOTIFY_MININT,
     NOTIFY_NEGONE,
     NOTIFY_ONE,
     NOTIFY_NONE,
     NOTIFY_SIGNAL;

  enum_dial SIGNO :
     SIGABRT,
     SIGINT,
     SIGILL,
     SIGSEGV,
     SIGRTMIN,
     SIGRTMAX,
     MAXINT,
     MININT,
     ZERO,
     ONE,
     NEGONE;
]

access
[
  NOTIFY_ZERO
  {
    sigEvent.sigev_notify = 0;
  }
  NOTIFY_MAXINT
  {
    sigEvent.sigev_notify = MAXINT;
  }     
  NOTIFY_MININT
  {
    sigEvent.sigev_notify = -MAXINT;
  }
  NOTIFY_NEGONE
  {
    sigEvent.sigev_notify = -1;
  }
  NOTIFY_ONE
  {
    sigEvent.sigev_notify = 1;
  }
  NOTIFY_NONE
  {
    sigEvent.sigev_notify = SIGEV_NONE;
  }
  NOTIFY_SIGNAL
  {
    sigEvent.sigev_notify = SIGEV_SIGNAL;
  }

  SIGABRT
  {
    sigEvent.sigev_signo = SIGABRT;
  }
  SIGINT
  {
    sigEvent.sigev_signo = SIGINT;
  }
  SIGILL
  {
    sigEvent.sigev_signo = SIGILL;
  }
  SIGSEGV
  {
    sigEvent.sigev_signo = SIGSEGV;
  }
  SIGRTMIN
  {
    sigEvent.sigev_signo = SIGRTMIN;
  }
  SIGRTMAX
  {
    sigEvent.sigev_signo = SIGRTMAX;
  }
  MAXINT
  {
    sigEvent.sigev_signo = MAXINT;
  }
  MININT
  { 
    sigEvent.sigev_signo = -MAXINT;
  }
  ZERO
  {
    sigEvent.sigev_signo = 0;
  }
  ONE
  {
    sigEvent.sigev_signo = 1;
  }
  NEGONE
  {
    sigEvent.sigev_signo = -1;
  }
{
  _theVariable=&sigEvent;
}

]

commit
[
]

cleanup
[
]
