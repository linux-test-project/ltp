// b_id.tpl : Ballista Datatype Template for id (user & group)
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

name uid_t b_id;   // using uid_t arbitarily could have used gid_t

parent b_int;

includes
[
{
#include <sys/types.h>
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
  enum_dial ID : 
    UID_SELF,
    GID_SELF,
    EUID_SELF,
    EGID_SELF,
    UID_PLUS1,
    GID_PLUS1;
]

access
[
   UID_SELF
   {
      _theVariable = getuid();
   }
   GID_SELF
   {
      _theVariable = getgid();
   }
   EUID_SELF
   {
      _theVariable = geteuid();
   }
   EGID_SELF
   {
      _theVariable = getegid();
   }
   UID_PLUS1 
   {
      _theVariable = getuid() + 1; 
   } 
   GID_PLUS1
   {
      _theVariable = getgid() + 1;
   }   
]

commit
[
]

cleanup
[
]
