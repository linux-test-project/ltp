// b_sched_policy.tpl : Ballista Datatype Template for schedule policy
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

name int b_sched_policy;

parent b_int;

includes
[
 {
   #include <sched.h>
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
  enum_dial HVAL : FIFO,RR,OTHER;
]

access
[
  FIFO
  {
    _theVariable= SCHED_FIFO;
  }
  RR
  {
    _theVariable= SCHED_RR;
  }
  OTHER
  {
    _theVariable= SCHED_OTHER;
  }
]

commit
[
]

cleanup
[
]
