// b_gid_t.tpl : Ballista Datatype Template for group id pointer
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

name gid_t* b_ptr_gid_t;

parent b_ptr_unsigned_int;

includes
[
{
//unsigned int gid_t
#include <sys/types.h>
#include <unistd.h>//for getgid()
#include "b_ptr_unsigned_int.h"
}
]

global_defines
[
{
  static gid_t temp_gid;
}
]

dials
[
  enum_dial VALUE : GID_SELF, EGID_SELF, UID_SELF, EUID_SELF, GID_PLUS1;
]

access
[
  GID_SELF
  {
    temp_gid= getgid();
    _theVariable=&temp_gid;
  }
  EGID_SELF 
  {
    temp_gid= getegid();
    _theVariable=&temp_gid;
  }
  UID_SELF 
  {
    temp_gid= getuid();
    _theVariable=&temp_gid;
  }
  EUID_SELF 
  {
    temp_gid= geteuid();
    _theVariable=&temp_gid;
  }
  GID_PLUS1 
  {
    temp_gid= getgid() +1;
    _theVariable=&temp_gid;
  }
]

commit
[
]

cleanup
[
]
