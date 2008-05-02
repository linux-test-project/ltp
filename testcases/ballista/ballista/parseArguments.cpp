/* parseArguments.cpp: class for parsing through command line options
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

#include <stdlib.h>
#include <iostream>
#include <ctype.h>
#include <assert.h>
#include "parseArguments.h"
#include "ballistaUtil.h"

using namespace std;

/************************
 *
 * Function: Parse_arguments
 * Description: constructor
 * Function Inputs:
 * Global Inputs:
 * Return Values:
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Contidions: number_of_arguments=0
 * Design:
 * Notes:
 *
 ************************/
Parse_arguments::Parse_arguments()
{
  number_of_arguments=0;
}

/************************
 *
 * Function: init_and_find_index
 * Description: This method initializes the internal representation of the 
                command line options and returns to the index into argv 
		for the item right after the options
		
 * Function Inputs: first_index - this is the first index into argv 
                                  where the options may be found
                    argc - number of argments on command line
                    argv - command line arguments
 * Global Inputs:
 * Return Values:  index into argv of the first non option argument.  
                   This is always >= first index
 * Global Outputs:
 * Errors: If too many arguments are specified, or if an argument was not 
           specified for an option then the program will exit()
 * Pre-Conditions: number_of_arguments=0
 * Post-Contidions: the arguments are copied into the internal representation.
 * Design: loop through the arguments until one does not start with a hyphen.
           For each argument, read in the name and its value. 
	   Check that each option has a value.  
	   Store these data in the data array.
 * Notes: All options must be stored in pairs, there will be a 
          -argument_name and argument_value
 *
 ************************/
int Parse_arguments::init_and_find_index(int first_index, int argc, 
				    const char * const *argv)
{
  int i;
  int j = 0;

  assert(argv != NULL);

  number_of_arguments=0;
  i = first_index;

  while(i < argc && argv[i][0] == '-') {
    if(number_of_arguments == MAXIMUM_NUMBER_OF_ARGUMENTS) {
      cerr << "Error: Too many command line options." << endl;
      exit(1);
    }

    if(i + 1 == argc) {
      cerr << "Error: " << argv[i];
      cerr << " flag was used with invalid argument." << endl;
      exit(1);
    }

    // determine if multiple copies of the same argument exist
    j = 0; 
    while(j < number_of_arguments) {
      if(!strcasecmp(argv[i]+1,data[j].name)) {
        cerr << "Error: " << argv[i];
        cerr << " flag is repeated." << endl;
        exit(1);
      }
      j++;
    }

    safe_strncpy(data[number_of_arguments].name,
		 argv[i]+1,MAXIMUM_ARGUMENT_NAME_LENGTH);
    safe_strncpy(data[number_of_arguments].argument,
		 argv[i+1],MAXIMUM_ARGUMENT_LENGTH);
    i += 2;
    number_of_arguments++;
  }
  return i;
}

/************************
 *
 * Function: get_argument
 * Description:  get a command line argument
 * Function Inputs: argument_name - the argument that you want to find 
                                    without the hyphen
                    return_buffer - storage for the argument text
		    max_return_buffer - maximum size of return buffer
 * Global Inputs:
 * Return Values:  BOOL_TRUE if the argument existed, BOOL_FALSE if not
 * Global Outputs:
 * Errors:
 * Pre-Conditions:  
 * Post-Contidions: return_buffer will contain the argument text if it existed
 * Design: look through all entries for argument, return it if found
 * Notes: The lookup is not case sensitive.
          if the argument is not found, then nothing will be copied into 
	  the return_buffer.
 *
 ************************/
BOOL_TYPE Parse_arguments::get_argument(const char *argument_name, 
					char *return_buffer,
					int max_return_buffer)
{
  int i;

  assert(argument_name != NULL);
  assert(return_buffer != NULL);

  for(i=0;i<number_of_arguments;i++) {
    if(!strcasecmp(argument_name,data[i].name)) {
      safe_strncpy(return_buffer,data[i].argument,max_return_buffer);
      return BOOL_TRUE;
    }
  }
  return BOOL_FALSE;
}

/************************
 *
 * Function: get_and_validate_integer
 * Description: get a command line argument that is supposed to be an 
 *              integer and validate that it is in a certain range.  
 *      	If it is not specified, then return a default value.
 * Function Inputs: return_value - the integer argument is copied here
 *                  argument_name - the name of the argument you want to
 *		                    find without the hyphen.
 *		    default_value - value to be copied into return_value
 *                                  if the argument was not found
 *                  min_range - minimum range for the value
 *                  max_range - maximum range for the value
 * Global Inputs:
 * Return Values: BOOL_TRUE if the argument was found, BOOL_FALSE otherwise
 * Global Outputs:
 * Errors: if a value was specified outside of its range, the program will
 *         give an error message and abort
 * Pre-Conditions:
 * Post-Contidions: a value will be copied into return_value
 * Design:  Look up the argument.  If found, convert to integer and 
 *          check range.
 *          If in range, copy into return value and return BOOL_TRUE.
 *          If not in range, give and error and exit()
 *          If not found, copy the default value into return_value 
 *          and return BOOL_TRUE
 * Notes: This method allows default values to be specified when arguments 
 *        are not on the command line.  It is normal for arguments not to be
 *	  found and to use the default values
 *
 ************************/
BOOL_TYPE Parse_arguments::get_and_validate_integer(int *return_value,
						    const char *argument_name,
						    int default_value,
						    int min_range,
						    int max_range)
{
  char arg_buf[256];
  int value;
  int j;

  assert(return_value != NULL);
  assert(argument_name != NULL);

  if (get_argument(argument_name,arg_buf,sizeof(arg_buf))) { //check flags
    j = 0; // check to insure numbers for argument
    while (j < strlen(arg_buf)) {
      if (!(isdigit(arg_buf[j]) || (arg_buf[j] == '-' && j == 0))) {
        cerr << "Error: -" << argument_name;
        cerr << " flag used with invalid nondigit value: " << arg_buf << endl;
        cerr << "Valid range is " << min_range << " to " << max_range << endl;
        exit(1);
      }
    j++;
    }
    value = atoi(arg_buf); // check for proper range
    if (value < min_range || value > max_range){
      cerr << "Error: -" << argument_name;
      cerr << " flag used with invalid value: " << arg_buf << endl;
      cerr << "Valid range is " << min_range << " to " << max_range << endl;
      exit(1);
    }
    *return_value = value;
    return BOOL_TRUE;
  }
  *return_value = default_value;
  return BOOL_FALSE;
}

/************************
 *
 * Function: get_argument
 * Description:  get a command line argument
 * Function Inputs: argument_name - the argument that you want to find 
                                    without the hyphen
                    return_buffer - storage for the argument text
		    max_return_buffer - maximum size of return buffer
 * Global Inputs:
 * Return Values:  BOOL_TRUE if the argument existed, BOOL_FALSE if not
 * Global Outputs:
 * Errors:
 * Pre-Conditions:  
 * Post-Contidions: return_buffer will contain the argument text if it existed
 * Design: look through all entries for argument, return it if found
 * Notes: The lookup is not case sensitive.
          if the argument is not found, then nothing will be copied into 
	  the return_buffer.
 *
 ************************/
// BOOL_TYPE Parse_arguments::get_argument(const char *argument_name, 
// 					char *return_buffer,
// 					int max_return_buffer)
// {
//   int i;

//   assert(argument_name != NULL);
//   assert(return_buffer != NULL);

//   for(i=0;i<number_of_arguments;i++) {
//     if(!strcasecmp(argument_name,data[i].name)) {
//       safe_strncpy(return_buffer,data[i].argument,max_return_buffer);
//       return BOOL_TRUE;
//     }
//   }
//   return BOOL_FALSE;
// }

/************************
 *
 * Function: get_and_validate_float
 * Description: get a command line argument that is supposed to be an 
 *              float and validate that it is in a certain range.  
 *      	If it is not specified, then return a default value.
 * Function Inputs: return_value - the integer argument is copied here
 *                  argument_name - the name of the argument you want to
 *		                    find without the hyphen.
 *		    default_value - value to be copied into return_value
 *                                  if the argument was not found
 *                  min_range - minimum range for the value
 *                  max_range - maximum range for the value
 * Global Inputs:
 * Return Values: BOOL_TRUE if the argument was found, BOOL_FALSE otherwise
 * Global Outputs:
 * Errors: if a value was specified outside of its range, the program will
 *         give an error message and abort
 * Pre-Conditions:
 * Post-Contidions: a value will be copied into return_value
 * Design:  Look up the argument.  If found, convert to integer and 
 *          check range.
 *          If in range, copy into return value and return BOOL_TRUE.
 *          If not in range, give and error and exit()
 *          If not found, copy the default value into return_value 
 *          and return BOOL_TRUE
 * Notes: This method allows default values to be specified when arguments 
 *        are not on the command line.  It is normal for arguments not to be
 *	  found and to use the default values
 *
 ************************/
BOOL_TYPE Parse_arguments::get_and_validate_float(double *return_value,
						    const char *argument_name,
						    double default_value,
						    double min_range,
							double max_range)
{
  char arg_buf[256];
  double value;
  int j;

  assert(return_value != NULL);
  assert(argument_name != NULL);

  if (get_argument(argument_name,arg_buf,sizeof(arg_buf))) { //check flags
    j = 0; // check to insure numbers for argument
    while (j < strlen(arg_buf)) {
      if (!(isdigit(arg_buf[j]) || (arg_buf[j] == '-' && j == 0)||arg_buf[j]=='.')) {
        cerr << "Error: -" << argument_name;
        cerr << " flag used with invalid nondigit value: " << arg_buf << endl;
        cerr << "Valid range is " << min_range << " to " << max_range << endl;
        exit(1);
      }
    j++;
    }
    value = atof(arg_buf); // check for proper range
    if (value <= min_range || value > max_range){
      cerr << "Error: -" << argument_name;
      cerr << " flag used with invalid value: " << arg_buf << endl;
      cerr << "Valid range is " << min_range << " to " << max_range << endl;
      exit(1);
    }
    *return_value = value;
    return BOOL_TRUE;
  }
  *return_value = default_value;
  return BOOL_FALSE;
}








