// b_ptr_char.tpl : Ballista Datatype Template for character/string pointer
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

name char* b_ptr_char;

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
   static char  *save_b_ptr_char=NULL;
#define sup_fillstring(buf,len,fillChar)   for (int i=0; i<len; i++) buf[i] = fillChar
 }
]

dials
[
  enum_dial SIZE : S1,S4,S16,S8K;
  enum_dial CONTENTS : CHAR,NUM,NASTY,ESC;
  enum_dial FILL : EMPTY,SOME,FULL;
]

access
[
{
  char  fillVar;
  int   divisor;
  int  size;
  char  *temp_b_ptr_char=NULL;
  FILE* logFile = NULL;

  if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
  {
    exit(99);
  }
}


  S1
  {
    size = 1;
    temp_b_ptr_char = (char *)malloc(size);
    if(temp_b_ptr_char == NULL)
    {
      fprintf(logFile,"b_ptr_char - S1 malloc failed, function not tested\n");
      fclose(logFile);
      exit(99);
    }	
  }

  S4
  { 
    size = 4;
    temp_b_ptr_char = (char *)malloc(size);
    if(temp_b_ptr_char == NULL)
    {
      fprintf(logFile, "b_ptr_char - S4 malloc failed, function not tested\n");
      fclose(logFile);
      exit(99);
    }	
  }

  S16
  {
    size = 16;
    temp_b_ptr_char = (char *)malloc(size);
    if(temp_b_ptr_char == NULL)
    {
      fprintf(logFile,"b_ptr_char - S16 malloc failed, function not tested\n");
      fclose(logFile);
      exit(99);
    }	
  }

  S8K
  {
    size = 8192;
    temp_b_ptr_char = (char *)malloc(size);
    if(temp_b_ptr_char == NULL)
    {
      fprintf(logFile,"b_ptr_char - S8K malloc failed, function not tested\n");
      fclose(logFile);
      exit(99);
    }	
  }

  CHAR
    {
      fillVar = 'A';
    }
  NUM
    {
      fillVar = '1';
    }
  NASTY
    {
      fillVar = char(22);
    }
  ESC
    {
      fillVar = char(27);
    }
  SOME
    {
      sup_fillstring(temp_b_ptr_char, (int)(size/3), fillVar);
      temp_b_ptr_char[(int)(size/3)+1]='\0';
    }
  FULL
    {
      sup_fillstring(temp_b_ptr_char, size, fillVar);
    }
  EMPTY
    {
      temp_b_ptr_char[0]='\0';
    }
{
  fclose(logFile);
  save_b_ptr_char = temp_b_ptr_char;
  _theVariable = temp_b_ptr_char;
}

]

commit
[
]

cleanup
[
 {
   if (save_b_ptr_char !=NULL)
   {
     free(save_b_ptr_char);
   }
 }
]
