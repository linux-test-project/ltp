// b_ptr_sched_param.tpl : Ballista Datatype Template for schedule parameter pointer
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

name structschedparamptr b_ptr_sched_param;

parent b_ptr_buf;

includes
[
{
#define structschedparamptr struct sched_param*
#include <sys/types.h>
#include <sched.h>
#include "b_ptr_buf.h"
}
]

global_defines
[
{
  static struct sched_param sched_param_temp;
}
]

dials
[
  enum_dial PRIORITY : 
     CURRENT,
     ZERO,
     NEG_ONE,
     MIN, 
     MAX,
     ONE;
]

access
[
  CURRENT
  {
    if ((sched_getparam(getpid(), &sched_param_temp)) == -1)
    {
      FILE* logFile = NULL;
      
      if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
      {
        exit(99);
      }
      fprintf(logFile, "b_ptr_sched_param CURRENT - error getting sched_param  - function not tested\n");
      fclose(logFile);
      exit(99);
    }
  }

  ZERO   
  {
    sched_param_temp.sched_priority = 0;
  }
  NEG_ONE
  {
    sched_param_temp.sched_priority = -1;
  }
  MIN
  {
    sched_param_temp.sched_priority = -MAXINT;
  }
  MAX
  {
    sched_param_temp.sched_priority = MAXINT;
  }
  ONE
  {
    sched_param_temp.sched_priority = 1;
  }

{
  _theVariable=&sched_param_temp;
}

]

commit
[
]

cleanup
[
]
