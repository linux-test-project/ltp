// b_ptr_int.tpl : Ballista Datatype Template for an integer pointer
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

name int* b_ptr_int;

parent b_ptr_void;

includes
[
{
#include "b_ptr_void.h"
#include "bTypes.h"
}
]

global_defines
[
 {
   static int temp_int;
 }
]

dials
[
  enum_dial VALUE : ZERO,ONE,MININT,NEG_ONE;    // MAXINT is in b_ptr_void
]

access
[

  ZERO
  {
    temp_int = 0;
  }
  ONE
  {
    temp_int = 1;
  }
  MININT
  {
    temp_int = -MAXINT-1;
  }
  NEG_ONE
  {
    temp_int = -1;
  }

{
  _theVariable = &temp_int;
}
]

commit
[
]

cleanup
[
]
