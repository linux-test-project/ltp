// b_float.tpl : Ballista Datatype Template for float datatypes
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

name float b_float;

parent paramAccess;

includes
[
 {
  #include "values.h"  //for digital unix
  #include "bTypes.h"
  #include <math.h>
  #define intBase unsigned short
 }
]

global_defines
[
 {
 }
]

dials
[
  enum_dial HVAL :  ZERO, ONE, NEGONE, FMINEXP, MLN2, MPI, HALF_PI, QUARTER_PI, TWO_PI, MSQRT2, E, MAXFLOAT, MINFLOAT, NEGMAXFLOAT, NEGMINFLOAT;
]

access
[
  ZERO
  {
    _theVariable=0.0;
  }
  ONE
  {
    _theVariable=1.0;
  }
  NEGONE
  {
    _theVariable=-1.0;
  }
  FMINEXP
  {
    _theVariable=FMINEXP; // (-(DMAXEXP + DSIGNIF - _HIDDENBIT - 3))
  }
  MLN2
  {
    _theVariable=M_LN2; // 6.9314718055994530942E-1 /*Hex  2^-1 * 1.62E42FEFA39EF */
  }
  MPI
  {
    _theVariable=M_PI;// 3.1415926535897932385E0  /*Hex  2^ 1 * 1.921FB54442D18 */
  }
  HALF_PI
  {
    _theVariable = M_PI/2;
  }
  QUARTER_PI
  {
    _theVariable = M_PI/4;
  }
  TWO_PI
  {
    _theVariable = M_PI * 2.0;
  }
  MSQRT2
  {
    _theVariable=M_SQRT2;// 1.4142135623730950488E0  /*Hex  2^ 0 * 1.6A09E667F3BCD */
  }
  E
  {
    _theVariable= M_E;   //2.718281828459045235360287
  }
  MAXFLOAT
  {
    _theVariable=MAXFLOAT;
  }
  MINFLOAT
  {
    _theVariable=MINFLOAT;
  }
  NEGMAXFLOAT
  {
    _theVariable=-MAXFLOAT;
  }
  NEGMINFLOAT
  {
    _theVariable=-MINFLOAT;
  }
]

commit
[
]

cleanup
[
]
