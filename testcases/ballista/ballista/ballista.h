// ballista.h : Main Ballista test harness header file
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

#ifndef _BALLISTIC_H_ 
#define _BALLISTIC_H_

// we have to use BOOL_TRUE and BOOL_FALSE since
// TRUE and FALSE are defined in rpc.h
typedef int BOOL_TYPE;
#define BOOL_TRUE -1
#define BOOL_FALSE 0

#define MAXP 16            //arbitrary parameter max
#define MAXD 32            //arbitrary max number of dials
#define MAX_RETURN 2048    //This is the array max size for rval returns.

typedef char b_param[255];

// extra elements are required to ensure null termination of both 
// lists of test value attributes and lists of dial settings
typedef b_param MARSHAL_DATA_TYPE[MAXP+1][MAXD+1]; 

#endif // _BALLISTIC_H_

