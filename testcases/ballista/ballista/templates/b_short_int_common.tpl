// b_short_int_common.tpl : Ballista Datatype Template for common short integers
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

name intBase b_short_int_common;

parent paramAccess;

includes
[
 {
  #include "values.h"  //for digital unix
  #include "bTypes.h"
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
  enum_dial VALUE : ZERO, ONE, TWO, FIFTEEN, SIXTEEN, SEVENTEEN, SC_PAGE, SIXTYFOUR,SIXTYFIVE, ONE27, ONE29, TWO55, TWO56, TWO57, I4K, I8K, I8193, I16383, MAXSHORT;
]

access
[
  ZERO
  {
     _theVariable=0;
  }
  ONE
  {
    _theVariable=1;
  }
  TWO
  {
    _theVariable=2;
  }
  FIFTEEN
  {
    _theVariable=15;
  }
  SIXTEEN
  {
    _theVariable=16;
  }
  SEVENTEEN
  {
    _theVariable=17;
  }
  SC_PAGE
  {
    _theVariable=_SC_PAGESIZE;
  }
  SIXTYFOUR
  {
    _theVariable=64;
  }
  SIXTYFIVE
  {
    _theVariable=65;
  }
  ONE27
  {
    _theVariable=127;
  }
  ONE29
  {
    _theVariable=129;
  }
  TWO55
  {
    _theVariable=255;
  }
  TWO56
  {
    _theVariable=256;
  }
  TWO57
  {
    _theVariable=257;
  }
  I4K
  {
    _theVariable=4096;
  }
  I8K
  {
    _theVariable=8192;
  }
  I8193
  {
    _theVariable=8193;
  }
  I16383
  {
    _theVariable=16383;
  }
  MAXSHORT
  {   
    _theVariable=MAXSHORT;
  }
]

commit
[
]

cleanup
[
]
