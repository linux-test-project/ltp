// b_ptr_char_id.tpl : Ballista Datatype Template for char id pointer
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

name char* b_ptr_char_id;

parent b_ptr_char;

includes
[
 {
  #include "b_ptr_char.h" //parent class include file
  #include <sys/types.h>
  #include <pwd.h>
  #include <grp.h>
 }
]

global_defines
[
 {
   static char  charArray[64];
 }
]

dials
[
  enum_dial NAME : SYSTEM, ROOT, GROUP, USER;  
]

access
[

  SYSTEM
  {
     strcpy (charArray,"SYSTEM");
  }
  ROOT 
  { 
     strcpy (charArray,"ROOT");
  }  
  GROUP
  {
     struct group* tempGroup;
     tempGroup = getgrgid(getgid());
     strncpy(charArray,tempGroup->gr_name, 63);
  }
  USER
  {
    struct passwd* tempPasswd;
    tempPasswd = getpwuid(getuid());
    strncpy(charArray, tempPasswd->pw_name,63);
  }
  
{
  charArray[63] = '\0';
  _theVariable = charArray;
}

]

commit
[
]

cleanup
[
]
