// b_map_flag.tpl : Ballista Datatype Template for mapping flag
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

name int b_map_flag;   

parent b_int;

includes
[
{
#include <sys/mman.h>
#include <unistd.h>
#include "bTypes.h"
#include "b_int.h" 

}
]

global_defines
[
{
}
]

dials
[
  enum_dial MAP_FLAG : 
    SHARED,
    PRIVATE,
    FIXED,
    ALL;
]

access
[
{
   _theVariable = 0;
}
   SHARED,ALL  
   {
      _theVariable |= MAP_SHARED;
   }
   PRIVATE,ALL
   {
      _theVariable |= MAP_PRIVATE;
   }
   FIXED,ALL
   {
      _theVariable |= MAP_FIXED;
   }
]

commit
[
]

cleanup
[
]
