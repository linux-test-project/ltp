// b_pid.tpl : Ballista Datatype Template for process id
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

name pid_t b_pid;

parent b_int;

includes
[
{
#include <sys/types.h>
#include <unistd.h>
#include "bTypes.h"
#include "b_int.h" 

}
]

global_defines
[
{
}
]

dials
[
  enum_dial PID : 
            PID_SELF,
            PID_CHILD,
            PID_65535,
            PID_65536;
]

access
[

  PID_SELF
  {
     _theVariable = getpid();
  }

  PID_CHILD
  {
     static int pid_kid;
     if((pid_kid = fork())==0)   /* many syscalls taking a pid need a child to exist */
     {
        sleep (1); //child wait for sometime long enough
        exit(0); //child exits
     }
     _theVariable = pid_kid;
  }

  PID_65535
  {
     _theVariable = 65535;
  }

  PID_65536
  {
     _theVariable = 65536;
  }
]

commit
[
]

cleanup
[
]
