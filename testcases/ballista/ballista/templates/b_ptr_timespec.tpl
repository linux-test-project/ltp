// b_ptr_timespec.tpl : Ballista Datatype Template for timespec pointer
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

name structtimespecptr b_ptr_timespec;

parent b_ptr_buf;

includes
[
{
#include <time.h>
#include "bTypes.h"
#include "b_ptr_buf.h"
#define structtimespecptr struct timespec*
}
]

global_defines
[
{
  static struct timespec timeout_tmp;
}
]

dials
[
  enum_dial HVAL : 
     REALTIME,
     TIMEOUT_ZERO,
     TIMEOUT_NEG_ONE_NS,
     TIMEOUT_ILL, 
     TIMEOUT_NEG_ONE_SEC, 
     TIMEOUT_MIN, 
     TIMEOUT_MAX, 
     TIMEOUT_MIXED_1, 
     TIMEOUT_MIXED_2;
]

access
[
  REALTIME
  {
    if (clock_gettime(CLOCK_REALTIME, &timeout_tmp) == -1)
    {
      FILE* logFile = NULL;

      if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
      {
        exit(99);
      }
      fprintf(logFile, "b_ptr_timespec REALTIME - error encountered getting timespec for REALTIME - function not tested\n");
      fclose(logFile);
      exit(99);
    }
  }
  TIMEOUT_ZERO
  {
    timeout_tmp.tv_sec = 0;
    timeout_tmp.tv_nsec = 0;
  }
  TIMEOUT_NEG_ONE_NS
  {
    timeout_tmp.tv_sec = 0;
    timeout_tmp.tv_nsec = -1;
  }
  TIMEOUT_ILL
  {
    timeout_tmp.tv_sec = 0;
    timeout_tmp.tv_nsec = (int) 10e10;
  }
  TIMEOUT_NEG_ONE_SEC
  {
    timeout_tmp.tv_sec = -1;
    timeout_tmp.tv_nsec = 0;
  }
  TIMEOUT_MIN
  {
    timeout_tmp.tv_sec = -MAXLONG;  
    timeout_tmp.tv_nsec = -MAXINT;
  }  
  TIMEOUT_MAX
  {
    timeout_tmp.tv_sec = MAXLONG;
    timeout_tmp.tv_nsec = MAXINT;
  }
  TIMEOUT_MIXED_1
  {
    timeout_tmp.tv_sec = -1;
    timeout_tmp.tv_nsec = 1;
  }
  TIMEOUT_MIXED_2
  {
    timeout_tmp.tv_sec = 1;      
    timeout_tmp.tv_nsec = -1;      
  }

{
  _theVariable=&timeout_tmp;
}

]

commit
[
]

cleanup
[
]
