// b_ptr_float.tpl : Ballista Datatype Template for a float pointer
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

name float* b_ptr_float;

parent b_ptr_void;

includes
[
{
#include "b_ptr_void.h"
#include "math.h"
}
]

global_defines
[
 {
   static float temp_float;
 }
]

dials
[
  enum_dial VALUE :  FMINEXP,  LN2, PI, HALF_PI, TWO_PI, SQRT2, E, MAXFLOAT, MINFLOAT, NEGMAXFLOAT, NEGMINFLOAT, 
                     ZERO, NEGONE, ONE;
]

access
[
  FMINEXP
  {
     temp_float = FMINEXP; // (-(DMAXEXP + DSIGNIF - _HIDDENBIT - 3))
  }
  LN2
  {
     temp_float = M_LN2; // 6.9314718055994530942E-1 /*Hex  2^-1 * 1.62E42FEFA39EF */
  }
  PI
  {
     temp_float = M_PI;// 3.1415926535897932385E0  /*Hex  2^ 1 * 1.921FB54442D18 */
  }
  HALF_PI
  {
     temp_float = M_PI_2;  // PI/2
  }
  TWO_PI
  {
     temp_float = M_PI * 2.0;  
  }
  SQRT2
  {
     temp_float= M_SQRT2;// 1.4142135623730950488E0  /*Hex  2^ 0 * 1.6A09E667F3BCD */
  }
  E
  {
     temp_float = M_E; // 2.718281828459045235360287
  }
  MAXFLOAT
  {
     temp_float = MAXFLOAT;
  }
  MINFLOAT
  {
     temp_float = MINFLOAT;
  }
  NEGMAXFLOAT
  {
     temp_float = -MAXFLOAT;
  }
  NEGMINFLOAT
  {
     temp_float = -MINDOUBLE;
  }
  ZERO
  {
     temp_float = 0.0;
  }
  NEGONE
  {
     temp_float = -1.0;
  }
  ONE
  {
     temp_float = 1.0;
  }
{
   _theVariable = &temp_float;
}  

]

commit
[
]

cleanup
[
]
