// b_ptr_double.tpl : Ballista Datatype Template for a double pointer
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

name double* b_ptr_double;

parent b_ptr_float; 

includes
[
 {
//double: usually 8 bytes(64bits). sizeof(double)=8  15-16bit mantissa
//for more read /usr/include/values.h
   #include "b_ptr_float.h"
   #include "math.h"
 }
]

global_defines
[
 {
  static double temp_double;
 }
]

dials
[
  enum_dial HVAL : DMAXEXP, MAXDOUBLE, MINDOUBLE, NEGMAXDOUBLE, NEGMINDOUBLE;
]

access
[
  DMAXEXP
    {
      temp_double = DMAXEXP;
    }
  MAXDOUBLE
    {
      temp_double = MAXDOUBLE;
    }
  MINDOUBLE
    {
      temp_double = MINDOUBLE;
    }
  NEGMAXDOUBLE
    {
      temp_double = -MAXDOUBLE;
    }
  NEGMINDOUBLE
    {
      temp_double = -MINDOUBLE;
    }
{
   _theVariable = &temp_double;
}


]

commit
[
{
}
]

cleanup
[
{
}

]
