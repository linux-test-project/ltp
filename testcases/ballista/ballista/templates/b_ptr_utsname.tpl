// b_ptr_utsname.tpl : Ballista Datatype Template for an utsname pointer
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

name utsnamePtr b_ptr_utsname;

parent b_ptr_buf;

includes
[
{
#define utsnamePtr struct utsname *
#include <sys/utsname.h>
#include "b_ptr_buf.h"
}
]

global_defines
[
{
static struct utsname utsname_temp  ;
}
]

dials
[
  enum_dial SYSNAME : S_NULL, S_SIZE1, S_SYS_NMLN, S_SELF;
  enum_dial NODENAME : N_NULL, N_SIZE1, N_SYS_NMLN, N_SELF;
  enum_dial RELEASE : R_NULL, R_SIZE1, R_SYS_NMLN, R_SELF;
  enum_dial VERSION : V_NULL, V_SIZE1, V_SYS_NMLN, V_SELF;
  enum_dial MACHINE : M_NULL, M_SIZE1, M_SYS_NMLN, M_SELF;
]

access
[
{
  // open logFile
  FILE* logFile = NULL;
  if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
  {
    exit(99);
  }
}

  S_SELF
  {
    struct utsname utsname_self;
    if(!(uname(&utsname_self) ==0))
    {
      fprintf(logFile, "b_ptr_utsname - S_SELF error occurred in uname function not tested.\n");
      fclose(logFile);
      exit(99);
    }
    strcpy (utsname_temp.sysname, utsname_self.sysname);
  }

  S_NULL
  {
    utsname_temp.sysname[0] = '\0';
  }

  S_SIZE1
  {
    utsname_temp.sysname[0] = 'a';
    utsname_temp.sysname[1] = '\0';
  }

  S_SYS_NMLN
  { //ending null intentionally missing
    int i;
    for (i=0; i<SYS_NMLN; i++)
    {
      utsname_temp.sysname[i] = 'a'; 
    }
  }

  N_SELF
  {
    struct utsname utsname_self;
    if(!(uname(&utsname_self) ==0))   
    {
      fprintf(logFile, "b_ptr_utsname - N_SELF error occurred in uname function not tested.\n");
      fclose(logFile);
      exit(99);
    }
    strcpy (utsname_temp.nodename, utsname_self.nodename);
  }

  N_NULL
  {
    utsname_temp.nodename[0] = '\0';
  }

  N_SIZE1
  {
    utsname_temp.nodename[0] = 'a';
    utsname_temp.nodename[1] = '\0';
  }

  N_SYS_NMLN
  {
    int i;
    for (i=0; i<SYS_NMLN; i++)
    {
      utsname_temp.nodename[i] = 'a';
    }
  }

  R_SELF
  {
    struct utsname utsname_self;
    if(!(uname(&utsname_self) ==0))   
    {
      fprintf(logFile, "b_ptr_utsname - R_SELF error occurred in uname function not tested.\n");
      fclose(logFile);
      exit(99);
    }
    strcpy (utsname_temp.release, utsname_self.release);
  }

  R_NULL
  {
    utsname_temp.release[0] ='\0';
  }

  R_SIZE1
  {
    utsname_temp.release[0] ='a';
    utsname_temp.release[1] ='\0';
  }

  R_SYS_NMLN
  {
    int i;
    for (i=0; i<SYS_NMLN; i++)
    {
      utsname_temp.release[i] = 'a'; 
    }
  }

  V_SELF
  {
    struct utsname utsname_self;
    if(!(uname(&utsname_self) ==0))   
    {
      fprintf(logFile, "b_ptr_utsname - V_SELF error occurred in uname function not tested.\n");
      fclose(logFile);
      exit(99);
    }
    strcpy (utsname_temp.version, utsname_self.version);
  }

  V_NULL
  {
    utsname_temp.version[0] = '\0';
  }

  V_SIZE1
  {
    utsname_temp.version[0] = 'a';
    utsname_temp.version[1] = '\0';
  }

  V_SYS_NMLN
  {
    int i;
    for (i=0; i<SYS_NMLN; i++)
    {
      utsname_temp.version[i] = 'a'; 
    }
  }

  M_SELF
  {
    struct utsname utsname_self;
    if(!(uname(&utsname_self) ==0))   
    {
      fprintf(logFile, "b_ptr_utsname - M_SELF error occurred in uname function not tested.\n");
      fclose(logFile);
      exit(99);
    }
    strcpy (utsname_temp.machine, utsname_self.machine);
  }

  M_NULL
  {
    utsname_temp.machine[0] = '\0';
  }

  M_SIZE1
  {
    utsname_temp.machine[0] = 'a';
    utsname_temp.machine[1] = '\0';
  }

  M_SYS_NMLN
  {
    int i;
    for (i=0; i<SYS_NMLN; i++)
    {
      utsname_temp.machine[i] = 'a'; 
    }
  }
{
  _theVariable = &utsname_temp;
  fclose(logFile);
}
]

commit
[
]

cleanup
[
]
