// b_ptr_sigactionSUN.tpl : Ballista Datatype Template for signal action pointer - SunOS
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

name structSigactionPtr b_ptr_sigaction;

parent b_ptr_buf;

includes
[
{
#define structSigactionPtr struct sigaction*
#include <signal.h>
#include "b_ptr_buf.h"
}
]

global_defines
[
{
struct sigaction sigaction_temp;
void foo_handler1(int a){
}
void foo_action1(int sig, siginfo_t * b, void * c){
}
}
]

dials
[
  enum_dial SA_HANDLER : NULL, SIG_DFL, SIG_IGN, USR_FUNC, SIG_HOLD, SIG_ERR;
  enum_dial SA_MASK : EMPTY, FULL, SIGABRT, SIGSEGV, SIGINT, SIGILL, ZERO, MAXINT;
  enum_dial SA_FLAGS : SA_NOCLDSTOP_SET, SA_SIGINFO_SET, SA_ONSTACK, SA_RESTART, SA_ALL, NO_EXTRA, SA_ZERO, SA_MAXINT;
  enum_dial SA_SIGACTION : ACTION_NULL, ACTION_USR_FUNC;
]

access
[
{
  sigaction_temp.sa_flags = 0;
  sigaction_temp.sa_mask.__sigbits[0] = 0;
  sigaction_temp.sa_mask.__sigbits[1] = 0;
  sigaction_temp.sa_mask.__sigbits[2] = 0;
  sigaction_temp.sa_mask.__sigbits[3] = 0;
}

  NULL
  {
    sigaction_temp.sa_handler = NULL;
  }
  SIG_DFL
  {
    sigaction_temp.sa_handler = SIG_DFL;
  }
  SIG_IGN
  {
    sigaction_temp.sa_handler = SIG_IGN;
  }
  USR_FUNC
  {
    sigaction_temp.sa_handler = foo_handler1;
  }
  SIG_HOLD
  {
    sigaction_temp.sa_handler = SIG_HOLD;
  }
  SIG_ERR
  {
    sigaction_temp.sa_handler = SIG_ERR;
  }
 
  EMPTY
  {//no signals blocked
    if((sigemptyset (&sigaction_temp.sa_mask))!=0)
    {
      FILE* logFile = NULL;
   
      if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
      {
        exit(99);
      }
      fprintf (logFile, "b_ptr_sigaction - sigemptyset at EMPTY failed. Function not tested\n");
      fclose(logFile);
      exit(99);
    }
  }
  FULL
  {//all signals blocked.
    if((sigfillset (&sigaction_temp.sa_mask))!=0)
    {
      FILE* logFile = NULL;
   
      if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
      {
        exit(99);
      }
      fprintf (logFile, "b_ptr_sigaction - sigfullset at FULL failed. Function not tested\n");
      fclose(logFile);
      exit(99);
    }
  }
  SIGABRT
  {
    sigaction_temp.sa_mask.__sigbits[0] = SIGABRT;
  }
  SIGSEGV
  {
    sigaction_temp.sa_mask.__sigbits[0] = SIGSEGV;
  }
  SIGINT
  {
    sigaction_temp.sa_mask.__sigbits[0] = SIGINT;
  }
  SIGILL 
  {
    sigaction_temp.sa_mask.__sigbits[0] = SIGILL;
  }
  ZERO 
  {
    sigaction_temp.sa_mask.__sigbits[0] = 0;
  }
  MAXINT 
  {
    sigaction_temp.sa_mask.__sigbits[0] = MAXINT;
  }

  SA_NOCLDSTOP_SET, SA_ALL
  {
    sigaction_temp.sa_flags |= SA_NOCLDSTOP;
  }
  SA_SIGINFO_SET, SA_ALL
  {
    sigaction_temp.sa_flags |= SA_SIGINFO;
  }
  SA_ONSTACK, SA_ALL
  {
    sigaction_temp.sa_flags |= SA_ONSTACK;
  }
  SA_RESTART, SA_ALL
  {
    sigaction_temp.sa_flags |= SA_RESTART;
  }
  SA_ZERO
  {
    sigaction_temp.sa_flags |= 0;
  }
  SA_MAXINT
  {
    sigaction_temp.sa_flags |= MAXINT;
  }
  SA_ALL
  {
    sigaction_temp.sa_flags |= SA_RESTART | SA_NODEFER |  SA_RESETHAND |  SA_NOCLDWAIT;
  }    

  ACTION_NULL
  {
    sigaction_temp.sa_sigaction = NULL;
  }	
  ACTION_USR_FUNC
  {
    sigaction_temp.sa_sigaction = foo_action1;
  }	

{
  _theVariable = &sigaction_temp;
}
]

commit
[
]

cleanup
[
]
