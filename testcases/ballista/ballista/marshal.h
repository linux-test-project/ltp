/* marshal.h: Handles the marshalling of dials
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

#ifndef _MARSHAL_H
#define _MARSHAL_H

#include "ballista.h"

// Define the characters that terminates at end of each dial, dial settings, and parameter list
#define DIAL_END ','
#define DIAL_SETTINGS_END '@'
#define PARAMETER_LIST_END '\0'

// Define the constant string used to concatenate terminators to marshalled data string.
const char DIAL_MARKER[2] = { DIAL_END, '\0' };
const char DIAL_SETTINGS_MARKER[2] = { DIAL_SETTINGS_END, '\0'};
const char PARAMETER_LIST_MARKER[2] = { PARAMETER_LIST_END, '\0'};

int marshal(const MARSHAL_DATA_TYPE data, char *string);

#endif // _MARSHAL_H
