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

#ifndef _PARSE_ARGUMENTS_H_
#define _PARSE_ARGUMENTS_H_

#include "ballista.h"

const int MAXIMUM_NUMBER_OF_ARGUMENTS = 30;
const int MAXIMUM_ARGUMENT_NAME_LENGTH = 30;
const int MAXIMUM_ARGUMENT_LENGTH = 256;

struct Argument_data
{
  char name[MAXIMUM_ARGUMENT_NAME_LENGTH];
  char argument[MAXIMUM_ARGUMENT_LENGTH];
};


class Parse_arguments
{
private:
  int number_of_arguments;
  Argument_data data[MAXIMUM_NUMBER_OF_ARGUMENTS];

public:
  Parse_arguments();
  int init_and_find_index(int first_index, int argc, const char * const *argv);
  BOOL_TYPE get_argument(const char *argument_name, 
			 char *return_buffer,
			 int max_return_buffer);

  BOOL_TYPE get_and_validate_integer(int *return_value,
				     const char *argument_name,
				     int default_value,
				     int min_range,
				     int max_range);  
  BOOL_TYPE get_and_validate_float(double *return_value,
				     const char *argument_name,
								   double default_value,
				     double min_range,
				     double max_range);  

};

#endif // _PARSE_ARGUMENTS_H_




















