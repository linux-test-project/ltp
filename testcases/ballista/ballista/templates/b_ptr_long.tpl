// b_ptr_long.tpl : Ballista Datatype Template for a long pointer
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

name long* b_ptr_long;

parent b_ptr_int;

includes
[
{
#include "b_ptr_int.h"
}
]

global_defines
[
 {
   static long temp_long;
 }
]

dials
[
  enum_dial VALUE : MAXLONG, NEG_MAXLONG;
]

access
[
  MAXLONG
  {
    temp_long = MAXLONG;
  }

  NEG_MAXLONG
  {
    temp_long = - temp_long;
  }

{
  _theVariable = &temp_long;
}
]

commit
[
]

cleanup
[
]
