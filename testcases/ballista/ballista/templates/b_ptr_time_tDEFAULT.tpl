// b_ptr_time_tDEFAULT.tpl : Ballista Datatype Template for a time_t pointer
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

name time_t* b_ptr_time_t;

parent b_ptr_long;

includes
[
{
#include <time.h>
#include "bTypes.h"
#include "b_ptr_long.h"

}
]

global_defines
[
{
//need exact value for midnight_2000
#define MIDNIGHT_2000 (2000-1970)*365*24*60*60   
#define YEAR 365*24*60*60
#define DAY 24*60*60
#define HOUR 60*60
#define MINUTE 60
}
]

dials
[
  enum_dial SECONDS : 
	NOW,
	TOMORROW,	
	NEXTWEEK, 
	NEXTMONTH, 
	NEXTYEAR, 
	NEXTDECADE, 
	NEXTCENTURY,
	NEXTMILLENIUM,
	Y2K,
	Y2K_MINUS_ONE,
	Y2K_PLUS_ONE;
]

access
[
{
  time_t time_since_epoch = time(NULL);
}
  NOW
  {
    _theVariable= &time_since_epoch;
  }
  TOMORROW
  {
    time_since_epoch += DAY;
    _theVariable = &time_since_epoch;
  }
  NEXTWEEK
  {
    time_since_epoch += (DAY*7);
    _theVariable = &time_since_epoch;
  }
  NEXTMONTH
  {
    time_since_epoch += (DAY*30);
    _theVariable= &time_since_epoch;
  }
  NEXTYEAR
  {
    time_since_epoch += YEAR;
    _theVariable= &time_since_epoch;
  }  
  NEXTDECADE
  {
    time_since_epoch += (YEAR*10);
    _theVariable= &time_since_epoch;
  }
  NEXTCENTURY
  {
    time_since_epoch += (YEAR*100);
    _theVariable= &time_since_epoch;
  }
  NEXTMILLENIUM
  {
    time_since_epoch += (YEAR*1000);
    _theVariable=&time_since_epoch;
  }
  Y2K
  {
    time_since_epoch = MIDNIGHT_2000;
    _theVariable=&time_since_epoch;
  }
  Y2K_MINUS_ONE
  {
    time_since_epoch = MIDNIGHT_2000-1;
    _theVariable=&time_since_epoch;

  }
  Y2K_PLUS_ONE
  {
    time_since_epoch = MIDNIGHT_2000+1;
    _theVariable=&time_since_epoch;
  }
]

commit
[
]

cleanup
[
]
