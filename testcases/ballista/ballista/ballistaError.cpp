// ballistaError.cpp : Ballista Error Class for exceptions
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

/****************/
#include <assert.h>

#include "ballistaError.h"
#include "ballistaUtil.h"

/************************
 *
 * Function: Ballistic_error constructor
 * Description:
 * Function Inputs: error message
 * Global Inputs:
 * Return Values:
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Conditions: error message is copied into class buffer
 * Design: copy the error message
 * Notes:
 *
 ************************/
Ballistic_error::Ballistic_error(const char *exception_message) 
{
  assert(exception_message != NULL);

  safe_strncpy(error_message,exception_message,sizeof(error_message));
}

/************************
 *
 * Function: get_error_message
 * Description: return the error message
 * Function Inputs:
 * Global Inputs:
 * Return Values: pointer to error_message array
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Conditions:
 * Design:
 * Notes: this should be used to display the error message to the screen,
          not to obtain a pointer to the buffer for modification
 *
 ************************/
char *Ballistic_error::get_error_message(void)
{
  return error_message;
}
