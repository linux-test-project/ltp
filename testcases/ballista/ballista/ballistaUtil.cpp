/* ballistaUtil.cpp : Ballista general purpose utility functions
 * Copyright (C) 1998-2001  Carnegie Mellon University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License  
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <assert.h>
#include <string.h>
#include <sys/time.h> //for get_time_in_microseconds
#include <iostream>
#include <stdlib.h>

#include "ballistaUtil.h"
#include "ballista.h"

using namespace std;

/************************
 *
 * Function: safe_strncpy
 * Description: wrapper function for strncpy
                does a strncpy and assures that the resulting string is null
                terminated.  strncpy alone will not NULL terminate the string
		if strlen(dst) >= n
 * Function Inputs: see description of strncpy
      dst - destination buffer
      src - source buffer
      n - maximum length of dst
 * Global Inputs:
 * Return Values:  dst
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Contidions:
 * Design:
 * Notes:
 *    make sure that the absolute last character is NULL, then call
 *    strncpy
 ************************/
char *safe_strncpy(char *dst, const char *src, size_t n)
{
  assert(dst != NULL);

  if(n > 0) {
    dst[n-1] = 0;
    return strncpy(dst,src,n-1);
  }
  else
    return dst;
}

/************************
 *
 * Function: get_time_in_microseconds
 * Description: gets the current time of day and converts that to microseconds
 * Function Inputs: none
 * Global Inputs: none
 * Return Values:  a double long (long long) containing time of day in microsec.
 * Global Outputs: none
 * Errors: exits if we cannot get the time of day
 * Pre-Conditions: none
 * Post-Contidions: none
 * Design: Made because we were using gethrtime before, which is not
 *         supported by Linux.  gettimeofday works on Linux and still works
 *         on Solaris.
 * Notes:
 ************************/
long long get_time_in_microseconds(void)
{
  struct timeval timeofday;
  struct timezone tz;
  long long time;

  if(gettimeofday(&timeofday,&tz) != 0) {
    cerr << "Error getting time." << endl;
    exit(-100);
  }
  time = timeofday.tv_sec;
  time *= 1000000;
  time += timeofday.tv_usec;
  return time;
}



