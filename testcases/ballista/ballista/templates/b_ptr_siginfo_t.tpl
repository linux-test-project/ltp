// b_ptr_siginfo_t.tpl : Ballista Datatype Template for a signal info pointer
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

name siginfo_t* b_ptr_siginfo_t;

parent b_ptr_buf;

includes
[
{
#include <signal.h>
#include "b_ptr_buf.h"
}
]

global_defines
[
{
static siginfo_t siginfo_t_temp;
}
]

dials
[
  enum_dial SI_SIGNO : SIGABRT, SIGILL, SIGUSR1, SIGUSR2, MAXINT, MININT,ZERO,ONE,NEG_ONE;
  enum_dial SI_CODE : SI_USER, SI_QUEUE, SI_TIMER, SI_ASYNCIO, SI_MESGQ, C_MAXINT, C_MININT,C_ZERO,C_ONE,C_NEG_ONE;
  // SI_VALUE intentionally excluded.
]

access
[
  SIGABRT
  {
    siginfo_t_temp.si_signo = SIGABRT ;
  }
  SIGILL
  {
    siginfo_t_temp.si_signo = SIGILL;
  }
  SIGUSR1
  {
    siginfo_t_temp.si_signo = SIGUSR1;
  }
  SIGUSR2
  {
    siginfo_t_temp.si_signo = SIGUSR2;
  }
  MAXINT
  {
    siginfo_t_temp.si_signo = MAXINT;
  }
  MININT
  {
    siginfo_t_temp.si_signo = -MAXINT;
  }
  ZERO
  {
    siginfo_t_temp.si_signo = 0;
  }
  ONE
  {
    siginfo_t_temp.si_signo = 1;
  }
  NEG_ONE
  {
    siginfo_t_temp.si_signo = -1;
  }

  SI_USER
  {
    siginfo_t_temp.si_code = SI_USER;
  }
  SI_QUEUE
  {
    siginfo_t_temp.si_code = SI_QUEUE;
  }
  SI_TIMER
  {
    siginfo_t_temp.si_code = SI_TIMER;
  }
  SI_ASYNCIO
  {
    siginfo_t_temp.si_code = SI_ASYNCIO;
  }
  SI_MESGQ
  {
    siginfo_t_temp.si_code = SI_MESGQ;
  }
  C_MAXINT
  {
    siginfo_t_temp.si_code = MAXINT;
  }
  C_MININT
  {
    siginfo_t_temp.si_code = -MAXINT;
  }
  C_ZERO
  {
    siginfo_t_temp.si_code = 0;
  }
  C_ONE
  {
    siginfo_t_temp.si_code = 1;
  }
  C_NEG_ONE
  {
    siginfo_t_temp.si_code = -1;
  }

{
  _theVariable = &siginfo_t_temp;
}
]

commit
[
]

cleanup
[
]
