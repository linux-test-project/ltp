// b_char.tpl : Ballista Datatype Template for a character
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

name char b_char;

parent paramAccess;
includes
[
 {
  #include "values.h"  //for digital unix
  #include "bTypes.h"
 }
]

global_defines
[
 {
  #define MaxBytes sizeof(unsigned short);
 }
]

dials
[
  enum_dial HVAL : NUL, SOH, TAB, LF, CR, DLE, ESC, SPACE, COMMA, NUM0, NUM9, AT, BIGA, BIGZ, LEFTSQBRKT, APST, SMLA, SMLZ, LEFTCUVBRKT, FF;
]

access
[
 NUL
    {
       _theVariable=0;
    }
 SOH
    {
       _theVariable=1;//smilly face
    }
 TAB
    {
       _theVariable=9;
    }
 LF
    {
       _theVariable=10;//line feed
    }
 CR
    {
       _theVariable=13;//carriage return
    }
 DLE
    {
       _theVariable=16;//delete
    }
 ESC
    {
       _theVariable=27;
    }
 SPACE
    {
       _theVariable=32;
    }
 COMMA
    {
       _theVariable=44;
    }
 NUM0
    {
       _theVariable=48; //'0'
    }
 NUM9
    {
       _theVariable=57;//'9'
    }
 AT
    {
       _theVariable=64; //'@'
    }
 BIGA
    {
       _theVariable=65;//'A'
    }
 BIGZ
    {
       _theVariable=90;//'Z'
    }
 LEFTSQBRKT
    {
       _theVariable=91;//'['
    }
 APST
    {
       _theVariable=96;//'`'
    }
 SMLA
    {
       _theVariable=97;//'a'
    }
 SMLZ
    {
       _theVariable=122;//'z'
    }
 LEFTCUVBRKT
    {
       _theVariable=123;//'{'
    }

 FF
    {
       _theVariable=255;//blank 'FF'
    }
]

commit
[
{
#ifdef DEBUG
	printf("VALUE:%c in commit\n", _theVariable);
#endif
}
]

cleanup
[
{
#ifdef DEBUG
	printf("VALUE:%c in cleanup\n", _theVariable);
#endif
}
]
