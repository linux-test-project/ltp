// b_ptr_utimbuf.tpl : Ballista Datatype Template for an utimbuf pointer
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

name structUtimbufPtr b_ptr_utimbuf;

parent b_ptr_buf;

includes
[
{
#define structUtimbufPtr struct utimbuf *
#include <sys/types.h>
#include <sys/time.h>
#include <utime.h>
#include "b_ptr_buf.h"

}
]

global_defines
[
{
#define MIDNIGHT_2000 (2000-1970)*365*24*60*60   
#define YEAR 365*24*60*60
#define DAY 24*60*60
#define HOUR 60*60
#define MINUTE 60
static struct utimbuf tmp_utimbuf;
}
]

dials
[	
  enum_dial ACTIME : A_NOW,A_TOMORROW,A_NEXTWEEK,A_NEXTMONTH, A_NEXTYEAR, A_NEXTDECADE,A_NEXTCENTURY,A_NEXTMILLENIUM,A_Y2K,A_Y2K_MINUS_ONE,A_Y2K_PLUS_ONE,A_MAX,A_MIN; 
  enum_dial MODTIME : M_NOW,M_TOMORROW,M_NEXTWEEK,M_NEXTMONTH, M_NEXTYEAR, M_NEXTDECADE,M_NEXTCENTURY,M_NEXTMILLENIUM,M_Y2K,M_Y2K_MINUS_ONE,M_Y2K_PLUS_ONE,M_MAX,M_MIN;
]

access
[ 
{
tmp_utimbuf.actime = time(NULL);//initialized to NOW
tmp_utimbuf.modtime = time(NULL);
}
    A_NOW
    {
    }
    A_TOMORROW
    {
	tmp_utimbuf.actime += DAY;
    }
    A_NEXTWEEK
    {
	tmp_utimbuf.actime += (DAY*7);
    }
    A_NEXTMONTH
    {
	tmp_utimbuf.actime += (DAY*30);
    }
    A_NEXTYEAR
    {
	tmp_utimbuf.actime += YEAR;
    }  
    A_NEXTDECADE
    {
	tmp_utimbuf.actime += (YEAR*10);
    }
    A_NEXTCENTURY
    {
	tmp_utimbuf.actime += (YEAR*100);
    }
    A_NEXTMILLENIUM
    {
	tmp_utimbuf.actime += (YEAR*1000);
    }
    A_Y2K
     {
	tmp_utimbuf.actime = MIDNIGHT_2000;
    }
    A_Y2K_MINUS_ONE
    {
	tmp_utimbuf.actime = MIDNIGHT_2000-1;
    }
    A_Y2K_PLUS_ONE
    {
	tmp_utimbuf.actime = MIDNIGHT_2000+1;
    }
    A_MAX
    {
         tmp_utimbuf.actime = MAXINT;
    }
    A_MIN 
    {
         tmp_utimbuf.actime = - MAXINT;
    }


    M_NOW
    {
    }
    M_TOMORROW
    {
	tmp_utimbuf.modtime += DAY;
    }
    M_NEXTWEEK
    {
	tmp_utimbuf.modtime += (DAY*7);
    }
    M_NEXTMONTH
    {
	tmp_utimbuf.modtime += (DAY*30);
    }
    M_NEXTYEAR
    {
	tmp_utimbuf.modtime += YEAR;
    }  
    M_NEXTDECADE
    {
	tmp_utimbuf.modtime += (YEAR*10);
    }
    M_NEXTCENTURY
    {
	tmp_utimbuf.modtime += (YEAR*100);
    }
    M_NEXTMILLENIUM
    {
	tmp_utimbuf.modtime += (YEAR*1000);
    }
    M_Y2K
     {
	tmp_utimbuf.modtime = MIDNIGHT_2000;
    }
    M_Y2K_MINUS_ONE
    {
	tmp_utimbuf.modtime = MIDNIGHT_2000-1;
    }
    M_Y2K_PLUS_ONE
    {
	tmp_utimbuf.modtime = MIDNIGHT_2000+1;
    }
    M_MAX
    {
         tmp_utimbuf.modtime = MAXINT;
    }
    M_MIN
    {
         tmp_utimbuf.modtime = - MAXINT;
    }
{
    _theVariable = &tmp_utimbuf;
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
