/* unmarshal.cpp Handle the marshalling and demarshalling of dial settings
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

#include "ballista.h"
#include "unmarshal.h"
#include "marshal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h> // for testing only

/************************
 *
 * Function:        unmarshal
 * Description:     unmarshal a string into dial settings
 * Function Inputs: string:  string to store marshalled dial settings,
 *                  data: two-dimensial array of dial settings
 * Global Inputs:   - N.A.
 * 
 * Return Values:   0 : success
 * Errors           -1 : exceeded the number of max dial settings error
 *                  -2 : exceeded the number of max parameters error
 * Global Outputs:  N.A.
 *         
 * Pre-Conditions:  dial settings is correctly allocated,
 *                  string string contains correctly marshalled data
 *
 * Post-Contidions: data contains the extracted dial settings in the string
 *
 * Design:          This should be the reverse process of marshal() function.
 *                  The outer loop extract the dial settings for one parameter.
 *                  The inner loop copy the individual dial setting into the array
 *                  as indicated by data[param_idx][dial_idx] for each parameter.
 *                  
 *                  
 * Notes:           An additional Null string is added after the last dial setting  
 *                  (end of each row) to replicate the original data[][] structure.
 *
 ************************/

int unmarshal(MARSHAL_DATA_TYPE data, const char string[])
{
  int param_idx = 0; // index into Parameters
  int dial_idx = 0;   // index into Dial Settings
  const char *param_ptr;      // point to the beginning of a set of dial setting for a parameter
  const char *dial_start_ptr;  // use to signify the beginning of a dial setting
  const char *dial_end_ptr;    // use to signify the end of a dial setting
    
  // initialize to the dial settings for the first parameter.
  param_ptr = string;

  // Exit the while loop if at null string
  while ( *param_ptr )
  {
    // Abnormal exit if the index into the Parameters exceeded max
    if (param_idx >= MAXP)
      return -2;

    //Initialize the pointer to the beginning of a set of dial settings
    dial_start_ptr = param_ptr;
    dial_idx=0;

    //Exit the while loop if the dial start pointer is after the end of the dial settings.
    while (dial_start_ptr < strchr(param_ptr, DIAL_SETTINGS_END))
    {

      // Abnormal exit if the index into the Dial Settings exceeded max
      if (dial_idx >= MAXD)
	return -1;

      //Copy a dial setting to the proper cell in data array.
      dial_end_ptr = strchr(dial_start_ptr, DIAL_END);
      strncpy(data[param_idx][dial_idx], dial_start_ptr, dial_end_ptr-dial_start_ptr);
      data[param_idx][dial_idx][dial_end_ptr-dial_start_ptr]='\0';  //terminate the string
           
      // Change index and pointer to the next dial setting.
      dial_idx++;
      dial_start_ptr = dial_end_ptr + 1;
    }

    //Add a null string after the end of the last dial setting for a parameter list
    data[param_idx][dial_idx][0] = '\0';
    
    // Change pointer to the next set of dial settings for the next parameter
    param_ptr = strchr(param_ptr,DIAL_SETTINGS_END) + 1;
    param_idx++;
  }

  //Add a null string after the end of all parameter lists.
  data[param_idx][dial_idx][0] = '\0';
	
  // return success
  return 0;
}
