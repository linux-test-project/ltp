// b_ptr_tms.tpl : Ballista Datatype Template for tms pointer
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

name structtmsptr b_ptr_tms;

parent b_ptr_buf;

includes
[
{
#include <sys/times.h>

#include "bTypes.h"
#include "b_ptr_buf.h"
#define structtmsptr struct tms*
}
]

global_defines
[
{
   static struct tms tmp_tms;
}
]

dials
[
  enum_dial TMSPTR : 
        THIS,
        ALL_ZERO,
        ALL_ONE,
        ALL_MAX,
        ALL_NEGONE;
]

access
[
{
   // Setup log file for template information

   FILE* logFile = NULL;

   if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
   {
      exit(99);
   }
}

  THIS
  {
     if((times(&tmp_tms)) == (clock_t)-1)
     {
        fprintf(logFile, "b_ptr_tms - THIS: times() failed. Function not tested\n");
        fclose(logFile);
        exit(99);
     }
  }

  ALL_ZERO
  {
     tmp_tms.tms_utime=0;              /* user time */
     tmp_tms.tms_stime=0;              /* system time */
     tmp_tms.tms_cutime=0;             /* user time, children */
     tmp_tms.tms_cstime=0;             /* system time, children */
  }

  ALL_ONE
  { 
     tmp_tms.tms_utime=1;              /* user time */
     tmp_tms.tms_stime=1;              /* system time */
     tmp_tms.tms_cutime=1;             /* user time, children */
     tmp_tms.tms_cstime=1;             /* system time, children */
  }  

  ALL_MAX
  { 
     tmp_tms.tms_utime=MAXINT;              /* user time */
     tmp_tms.tms_stime=MAXINT;              /* system time */
     tmp_tms.tms_cutime=MAXINT;             /* user time, children */
     tmp_tms.tms_cstime=MAXINT;             /* system time, children */
  }
  
  ALL_NEGONE
  { 
     tmp_tms.tms_utime=-1;              /* user time */
     tmp_tms.tms_stime=-1;              /* system time */
     tmp_tms.tms_cutime=-1;             /* user time, children */
     tmp_tms.tms_cstime=-1;             /* system time, children */
  }  

{
   fclose(logFile);
   _theVariable = &tmp_tms;
}
]

commit
[
]

cleanup
[
]
