// b_jmp_bufDEFAULT.tpl : Ballista Datatype Template for jump buffer
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

name jmp_buf b_jmp_buf;

parent b_ptr_int;

includes
[
{
#include "b_ptr_int.h"
#include <setjmp.h>
}
]

global_defines
[
 {
 }
]

dials
[
  enum_dial VALUE : VALID, VALID_PLUS1, SIG_VALID_SAVE, SIG_VALID_NOSAVE, SIG_VALID_NOSAVE_PLUS1;
]

access
[
  VALID
  {
    setjmp(_theVariable);
  }

  VALID_PLUS1
  {
    setjmp(_theVariable);
    _theVariable[0] ++;
  }

  SIG_VALID_SAVE
  {
    sigsetjmp(_theVariable,1);
  }

  SIG_VALID_NOSAVE
  {
    sigsetjmp(_theVariable,0);
  }

  SIG_VALID_NOSAVE_PLUS1
  {
    sigsetjmp(_theVariable,0);
    _theVariable[0] ++;
  }

]

commit
[
]

cleanup
[
]
