// b_ptr_itimerspec.tpl : Ballista Datatype Template for itimerspec pointer
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

name structitimerspecptr b_ptr_itimerspec;

parent b_ptr_buf;

includes
[
{
#include <time.h>
#include "bTypes.h"
#include "b_ptr_buf.h"
#define structitimerspecptr struct itimerspec*
}
]

global_defines
[
{
  static struct itimerspec itimerspec_temp;
}
]

dials
[
  enum_dial INTERVAL : 
     INTERVAL_REALTIME,
     INTERVAL_GETTIME,
     INTERVAL_ZERO,
     INTERVAL_NEG_ONE_NS,
     INTERVAL_ILL, 
     INTERVAL_NEG_ONE_SEC, 
     INTERVAL_MIN, 
     INTERVAL_MAX, 
     INTERVAL_MIXED_1, 
     INTERVAL_MIXED_2;

  enum_dial VALUE :
     VALUE_REALTIME,
     VALUE_GETTIME,
     VALUE_ZERO,
     VALUE_NEG_ONE_NS,
     VALUE_ILL,
     VALUE_NEG_ONE_SEC,
     VALUE_MIN,
     VALUE_MAX,
     VALUE_MIXED_1,
     VALUE_MIXED_2;
]

access
[
  INTERVAL_GETTIME, VALUE_GETTIME
  {
    if ((timer_gettime ( TIMER_ABSTIME, &itimerspec_temp)) == -1)
    {
      FILE* logFile = NULL;
      
      if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
      {
        exit(99);
      }
      fprintf(logFile, "b_ptr_itimerspec GETTIME - error encountered timer_gettime  - function not tested\n");
      fclose(logFile);
      exit(99);
    }
  }


  INTERVAL_REALTIME   
  {
    if (clock_gettime(CLOCK_REALTIME, &(itimerspec_temp.it_interval)) == -1)
    {
      FILE* logFile = NULL;

      if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
      {
        exit(99);
      }
      fprintf(logFile, "b_ptr_itimerspec INTERVAL_REALTIME - error encountered with clock_gettime  - function not tested\n");
      fclose(logFile);
      exit(99);
    }
  }
  INTERVAL_ZERO   
  {
    itimerspec_temp.it_interval.tv_sec = 0;
    itimerspec_temp.it_interval.tv_nsec = 0; 
  }
  INTERVAL_NEG_ONE_NS
  {
    itimerspec_temp.it_interval.tv_sec = 0; 
    itimerspec_temp.it_interval.tv_nsec = -1;
  }
  INTERVAL_ILL
  {
    itimerspec_temp.it_interval.tv_sec = 0;
    itimerspec_temp.it_interval.tv_nsec = (int) 10e10;
  }
  INTERVAL_NEG_ONE_SEC
  {
    itimerspec_temp.it_interval.tv_sec = -1;
    itimerspec_temp.it_interval.tv_nsec = 0;
  }
  INTERVAL_MIN
  {
    itimerspec_temp.it_interval.tv_sec = -MAXLONG;
    itimerspec_temp.it_interval.tv_nsec = -MAXINT;
  }
  INTERVAL_MAX
  {
    itimerspec_temp.it_interval.tv_sec = MAXLONG;
    itimerspec_temp.it_interval.tv_nsec = MAXINT;
  }
  INTERVAL_MIXED_1
  {
    itimerspec_temp.it_interval.tv_sec = -1;
    itimerspec_temp.it_interval.tv_nsec = 1;
  }
  INTERVAL_MIXED_2
  {
    itimerspec_temp.it_interval.tv_sec = 1; 
    itimerspec_temp.it_interval.tv_nsec = -1;
  }


  VALUE_REALTIME
  {
    if (clock_gettime(CLOCK_REALTIME, &(itimerspec_temp.it_value)) == -1)
    {
      FILE* logFile = NULL;

      if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
      {
        exit(99);
      }
      fprintf(logFile, "b_ptr_itimerspec VALUE_REALTIME - error encountered with clock_gettime - function not tested\n");
      fclose(logFile);
      exit(99);
    }
  }
  VALUE_ZERO
  {
    itimerspec_temp.it_value.tv_sec = 0;
    itimerspec_temp.it_value.tv_nsec = 0;
  }
  VALUE_NEG_ONE_NS
  {
    itimerspec_temp.it_value.tv_sec = 0;
    itimerspec_temp.it_value.tv_nsec = -1;
  }
  VALUE_ILL
  {
    itimerspec_temp.it_value.tv_sec = 0;
    itimerspec_temp.it_value.tv_nsec = (int) 10e10;
  }
  VALUE_NEG_ONE_SEC
  {
    itimerspec_temp.it_value.tv_sec = -1;
    itimerspec_temp.it_value.tv_nsec = 0;
  }
  VALUE_MIN
  {
    itimerspec_temp.it_value.tv_sec = -MAXLONG;  
    itimerspec_temp.it_value.tv_nsec = -MAXINT;
  }  
  VALUE_MAX
  {
    itimerspec_temp.it_value.tv_sec = MAXLONG;
    itimerspec_temp.it_value.tv_nsec = MAXINT;
  }
  VALUE_MIXED_1
  {
    itimerspec_temp.it_value.tv_sec = -1;
    itimerspec_temp.it_value.tv_nsec = 1;
  }
  VALUE_MIXED_2
  {
    itimerspec_temp.it_value.tv_sec = 1;      
    itimerspec_temp.it_value.tv_nsec = -1;      
  }

{
  _theVariable=&itimerspec_temp;
}

]

commit
[
]

cleanup
[
]
