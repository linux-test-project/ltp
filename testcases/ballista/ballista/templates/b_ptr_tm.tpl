// b_ptr_tm.tpl : Ballista Datatype Template for tm pointer
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

name StructtmPtr b_ptr_tm;

parent b_ptr_buf;

includes
[
{

#include <time.h>
#include "bTypes.h"
#include "b_ptr_buf.h"
#define StructtmPtr struct tm*
}
]

global_defines
[
{
 static struct tm tm_struct;
}
]

dials
[
enum_dial TM_STRUCT_PTR : 
	LOCALTIME_R_NOW,
	GMTIME_R_NOW,
        ALL_ZERO,
        ALL_TOP_RANGE,
        ALL_OVERFLOW,
        ALL_MAX,
        ALL_MIN;
]

access
[
{  
   time_t time_since_epoch = time(NULL);

   FILE* logFile = NULL;
   if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
   {
      exit(99);
   }
}

   LOCALTIME_R_NOW
    {
	if((localtime_r(&time_since_epoch,&tm_struct))==NULL)
        {
	   fprintf(logFile, "LOCALTIME_R_NOW: localtime_r failed in b_ptr_tm. Function not tested\n");
           fclose(logFile);
 	   exit(99);
	}
    }
   GMTIME_R_NOW
    {
	if((gmtime_r(&time_since_epoch,&tm_struct))==NULL)
        {
	   fprintf(logFile, "GMTIME_R_NOW: gmtime_r failed in b_ptr_tm. Function not tested\n");
           fclose(logFile);
 	   exit(99);
	}
    }
   
   ALL_ZERO
    {  // some OSs add fields to the tm struct - these fields are stated in POSIX
       tm_struct.tm_sec = 0; /* seconds after the minute [0-60] */
       tm_struct.tm_min = 0;         /* minutes after the hour [0-59] */
       tm_struct.tm_hour = 0;        /* hours since midnight [0-23] */
       tm_struct.tm_mday = 0;        /* day of the month [1-31] */
       tm_struct.tm_mon = 0;         /* months since January [0-11] */
       tm_struct.tm_year = 0;        /* years since 1900 */
       tm_struct.tm_wday = 0;        /* days since Sunday [0-6] */
       tm_struct.tm_yday = 0;        /* days since January 1 [0-365] */
       tm_struct.tm_isdst = 0;       /* Daylight Savings Time flag */
    }
  ALL_TOP_RANGE
    {// some OSs add fields to the tm struct - these fields are stated in POSIX
       tm_struct.tm_sec = 60; /* seconds after the minute [0-60] */
       tm_struct.tm_min = 59;         /* minutes after the hour [0-59] */
       tm_struct.tm_hour = 23;        /* hours since midnight [0-23] */
       tm_struct.tm_mday = 31;        /* day of the month [1-31] */
       tm_struct.tm_mon = 11;         /* months since January [0-11] */
       tm_struct.tm_year = 9999;        /* years since 1900 */
       tm_struct.tm_wday = 6;        /* days since Sunday [0-6] */
       tm_struct.tm_yday = 365;        /* days since January 1 [0-365] */
       tm_struct.tm_isdst = 1;       /* Daylight Savings Time flag */
    }
  ALL_OVERFLOW
    {// some OSs add fields to the tm struct - these fields are stated in POSIX
       tm_struct.tm_sec = 61; /* seconds after the minute [0-60] */
       tm_struct.tm_min = 60;         /* minutes after the hour [0-59] */
       tm_struct.tm_hour = 24;        /* hours since midnight [0-23] */
       tm_struct.tm_mday = 32;        /* day of the month [1-31] */
       tm_struct.tm_mon = 12;         /* months since January [0-11] */
       tm_struct.tm_year = 0;        /* years since 1900 */
       tm_struct.tm_wday = 7;        /* days since Sunday [0-6] */
       tm_struct.tm_yday = 366;        /* days since January 1 [0-365] */
       tm_struct.tm_isdst = -1;       /* Daylight Savings Time flag */
    }
  ALL_MAX
    {// some OSs add fields to the tm struct - these fields are stated in POSIX
       tm_struct.tm_sec = MAXINT; /* seconds after the minute [0-60] */   
       tm_struct.tm_min = MAXINT;         /* minutes after the hour [0-59] */
       tm_struct.tm_hour = MAXINT;        /* hours since midnight [0-23] */
       tm_struct.tm_mday = MAXINT;        /* day of the month [1-31] */
       tm_struct.tm_mon = MAXINT;         /* months since January [0-11] */
       tm_struct.tm_year = MAXINT;        /* years since 1900 */
       tm_struct.tm_wday = MAXINT;        /* days since Sunday [0-6] */
       tm_struct.tm_yday = MAXINT;        /* days since January 1 [0-365] */
       tm_struct.tm_isdst = MAXINT;       /* Daylight Savings Time flag */
    }
  ALL_MIN
    {// some OSs add fields to the tm struct - these fields are stated in POSIX
       tm_struct.tm_sec = -MAXINT; /* seconds after the minute [0-60] */   
       tm_struct.tm_min = -MAXINT;         /* minutes after the hour [0-59] */
       tm_struct.tm_hour = -MAXINT;        /* hours since midnight [0-23] */
       tm_struct.tm_mday = -MAXINT;        /* day of the month [1-31] */
       tm_struct.tm_mon = -MAXINT;         /* months since January [0-11] */
       tm_struct.tm_year = -MAXINT;        /* years since 1900 */
       tm_struct.tm_wday = -MAXINT;        /* days since Sunday [0-6] */
       tm_struct.tm_yday = -MAXINT;        /* days since January 1 [0-365] */
       tm_struct.tm_isdst = -MAXINT;       /* Daylight Savings Time flag */
    }
{
  _theVariable=&tm_struct;
  fclose(logFile);
}
]

commit
[
{
}
]

cleanup
[
{
}
]
