// b_wchar.tpl : Ballista Datatype Template for wide character
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

name wchar_t b_wchar;

parent b_char;
includes
[
 {
  #include <wchar.h>
  #include "b_char.h"
  #include "values.h"  //for digital unix
  #include "bTypes.h"
 }
]

global_defines
[
 {
 }
]

dials
[
  enum_dial HVAL : WCHAR_MAX,WCHAR_MIN,WCHAR_SPACE;
]

access
[
  WCHAR_MAX
  {
    _theVariable=WCHAR_MAX;
  }
  WCHAR_MIN
  {
    _theVariable=WCHAR_MIN;
  }
  WCHAR_SPACE
  {
    _theVariable= ' ';
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
