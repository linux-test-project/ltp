/* marshal.cpp : Handles the marshalling of dials
   Copyright (C) 1998-2001  Carnegie Mellon University

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <assert.h>
#include <stdio.h>	//testing only
#include <stdlib.h>
#include <string.h>

#include "ballista.h"
#include "marshal.h"

/************************
 *
 * Function:        marshal
 * Description:     marshal dial settings into a string
 *
 * Function Inputs: string:  string to extract  marshalled dial settings,
 *                  data: two-dimensial array of dial settings=
 * Global Inputs:   - N.A.
 *
 * Return Values:   0 : success
 * Errors           -1 : exceeded the number of max dial settings error
 *                  -2 : exceeded the number of max parameters error
 * Global Outputs:  - N.A.
 * 
 * Pre-Conditions:  dial settings list is null-terminated,
 *                  string has sufficient space allocated 
 *
 * Post-Conditions: the string contains the marshalled data in the format specified 
 *                  by the design below.
 *
 * Design:          dial setting are marshalled by concatenating the strings
 *                  in the data[param_idx][dial_idx] array.  Individual dial
 *                  settings are separated by DIAL_MARKERS. A set of dial settings
 *                  for one parameter list is separated by DIAL_SETTING_MARKER.  Finally
 *                  the PARAMETER_LIST_MARKER marks the end of the parameter list.
 *                  
 *                 
 * Notes:           The number of dial settings per parameter list can vary.
 *                  The program must find Null string that terminates a list.
 *
 ************************/

int marshal(const MARSHAL_DATA_TYPE data, char string[])
{

  int param_idx = 0; // index into Parameters
  int dial_idx = 0;   // index into Dials


  assert(data != NULL);
  assert(string != NULL);

  //Make sure that the string is null termintated first
  string[0] = '\0';

  // Check if at the end of the parameter lists, if first dial setting is null character
  while ( data[param_idx][0][0]!= '\0' )
  {    
    // Abnormal exit if the index into the Parameters exceeded max
    // Note:  The maximum X in data[X][.] is MAXP+1.
    if (param_idx >= MAXP)
      return -2;

    // Reset control parameter for while loop
    dial_idx=0;

    while (data[param_idx][dial_idx][0] != '\0') 
    {
	// Abnormal exit if the index into the Dial Settings exceeded max
	if (dial_idx >= MAXD)
		return -1;

	// Concat the individual dial setting to the end of string
	strcat(string,data[param_idx][dial_idx] );
	strcat(string, DIAL_MARKER);

	// Point to the next dial setting
	dial_idx++;	
    }
    // At end of a set of dial settings
    strcat(string, DIAL_SETTINGS_MARKER); 

    param_idx++;      
  }
  // Mark the end of the string signifying the end of the parameter list
  strcat(string, PARAMETER_LIST_MARKER);

  //Success:  Exit the program
  return 0;
}


