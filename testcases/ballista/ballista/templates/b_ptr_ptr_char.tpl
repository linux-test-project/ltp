// b_ptr_ptr_char.tpl : Ballista Datatype Template for a pointer to a char/string pointer
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

name char** b_ptr_ptr_char;

parent b_ptr_buf;

includes
[
 {
  #include "b_ptr_buf.h" //parent class include file
 }
]

global_defines
[
 {
  static char  *temp = NULL;
  static char *cp_temp = NULL;
  char  fillVar1;
  int  size1;
#define sup_fillstring(buf,len,fillChar)   for (int i=0; i<len; i++) buf[i] = fillChar
 }
]

dials
[
  enum_dial SIZE : S1,S4,S16,S8K;
  enum_dial CONTENTS : NUM,NASTY,ESC,CHAR;
  enum_dial FILL : EMPTY,SOME,FULL;
]

access
[
  S1
    {
      cp_temp = temp = (char *)malloc(1);
    }
  S4
    {
      cp_temp = temp = (char *)malloc(4);
    }
  S16
    {
      cp_temp = temp = (char *)malloc(16);
    }
  S8K
    {
      cp_temp = temp = (char *)malloc(8192);
    }

  CHAR
    {
      fillVar1 = 'A';
    }
  NUM
    {
      fillVar1 = '1';
    }
  NASTY
    {
      fillVar1 = char(22);
    }
  ESC
    {
      fillVar1 = char(27);
    }
  SOME
    {
      sup_fillstring(temp, (int)(size1/3), 'A');
      temp[(int)(size1/3)+1]='\0';
    }
  FULL
    {
       sup_fillstring(temp, size1, fillVar1);
    }
  EMPTY
    {
      temp[0]='\0';
    }
{
  _theVariable = &temp;
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
   if (temp !=NULL)
	free(cp_temp);
 }
]
