/* match.cpp: Utility functions for matching file contents with regular expressions 
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

#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

#include "match.h"

using namespace std;

/************************
 *
 * Function:        match_file
 * Description:     match the content of a file against an extended regular expression
 * Function Inputs: char *file_name,
 *		     char *reg_pattern
 *		     int* match 
 * Global Inputs: None  
 * Return Values: Return 0 if successful, else negative value if error found  
 * Global Outputs:  
 * Errors:          
 * Pre-Conditions:  
 * Post-Contidions:  *good_match = 1 if true, else 0.
 * Design:
 * Notes:           
 *
 ************************/
int match_file(const char *file_name, char* reg_pattern, int* good_match)
{
  struct stat file_info;
  char *file_buffer = NULL;
  ifstream is;

  // first get the size of the file
  if(stat(file_name,&file_info) != 0) {
    return -1;
  }

  // open the file
  is.open(file_name);
  if(is.bad()) {
    return -2;
  }

  // next allocate memory, including 1 byte for the NULL terminator
  file_buffer = (char *)malloc(file_info.st_size+1);
  if(file_buffer == NULL) {
    return -3;
  }
  
  // read in the data
  is.read(file_buffer,file_info.st_size);
  if(is.bad()) {  
    free(file_buffer);
    return -4;
  }
  
  // check if we actually read as many bytes as we wanted
  if(is.gcount() != file_info.st_size) {
    free(file_buffer);
    return -5;
  }
  
  // close the file
  is.close();

  // Ensure that the buffer is null terminated.
  *(file_buffer + file_info.st_size) = '\0';

  // Check the buffer against the regular expression 
  // and store result into *good_match.
  if (match(file_buffer,reg_pattern))
  {
    *good_match = 1;
  }
  else
  {
    *good_match = 0;
  }

  // free the memory after using it
  free(file_buffer);
  return 0;
}


/************************
 *
 * Function:        match
 * Description:     Match string against the extended regular expression in
 * 		     pattern, treating errors as no match.
 *
 * Function Inputs: char *string,
 *		     char *reg_pattern
 *		     
 * Global Inputs: None  
 * Return Values: Return 0 if match, else negative value   
 * Global Outputs:  
 * Errors:          
 * Pre-Conditions:  
 * Post-Contidions: 
 * Design:
 * Notes:           
 *
 ************************/
int match(const char *string, char *pattern)
{
  int  status;
  regex_t   re;
  
  if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
    return(0);      /* report error */
  }
  status = regexec(&re, string, (size_t) 0, NULL, 0);
  regfree(&re);
  if (status != 0) {
    return(0);      /* report error */
  }

  return(1);
}

