// b_ptr_char_mode.tpl : Ballista Datatype Template for file modes
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

name char* b_ptr_char_mode;

parent b_ptr_char;

includes
[
 {
  #include "b_ptr_char.h" //parent class include file
 }
]

global_defines
[
 {
   static char  charArray[3];
 }
]

dials
[
  enum_dial MODE : R,W,A,R_PLUS,W_PLUS,A_PLUS,B,C,N;  // note c and n are Microsoft defined but not std. - good exceptional cases
]

access
[

  R
  {
     charArray[0] = 'r';
     charArray[1] = '\0';
  }
  R_PLUS 
  { 
     charArray[0] = 'r';
     charArray[1] = '+';
     charArray[2] = '\0';
  }  
  W 
  { 
     charArray[0] = 'w';
     charArray[1] = '\0';
  }  
  W_PLUS 
  { 
     charArray[0] = 'w';
     charArray[1] = '+';
     charArray[2] = '\0';
  }  
  A 
  { 
     charArray[0] = 'a';
     charArray[1] = '\0';
  }  
  A_PLUS 
  { 
     charArray[0] = 'a';
     charArray[1] = '+';
     charArray[2] = '\0';
  }
  B 
  { 
     charArray[0] = 'b';
     charArray[1] = '\0';
  }  
  C 
  { 
     charArray[0] = 'c';
     charArray[1] = '\0';
  }  
  N 
  { 
     charArray[0] = 'n';
     charArray[1] = '\0';
  }  
  
{
  _theVariable = charArray;
}

]

commit
[
]

cleanup
[
]
