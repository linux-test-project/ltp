// b_ptr_sigset_t.tpl : Ballista Datatype Template for a set signal pointer
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

name sigset_t* b_ptr_sigset_t;

parent b_ptr_long;

includes
[
{

#include <signal.h>
#include "b_ptr_long.h"

}
]

global_defines
[
{
static sigset_t sigset_tmp;   

}
]

dials
[
  enum_dial SIGPTR : SIGSET_EMPTY,SIGSET_FULL,SIGSET_SIGINT,SIGSET_SIGSEGV,SIGSET_GARBAGE;
]

access
[
     SIGSET_EMPTY{
       	if((sigemptyset (&sigset_tmp))!=0){
		perror ("sigemptyset at SIGSET_EMPTY failed. Function not tested");
		exit(99);
	}
      	_theVariable = &sigset_tmp;
      }

     SIGSET_FULL{
       	if((sigfillset (&sigset_tmp))!=0){
		perror ("sigemptyset at SIGSET_FULL failed. Function not tested");
		exit(99);
	}
      _theVariable = &sigset_tmp;
      }

     SIGSET_SIGINT{
      if((sigemptyset (&sigset_tmp))!=0){
		perror ("sigemptyset at SIGSET_SIGINT failed. Function not tested");
		exit(99);
	}
      if((sigaddset (&sigset_tmp, SIGINT))!=0){
		perror ("sigaddset at SIGSET_SIGINT failed. Function not tested");
		exit(99);
	}
       _theVariable = &sigset_tmp;
      }

     SIGSET_SIGSEGV{
      if((sigemptyset (&sigset_tmp))!=0){
		perror ("sigfillset at SIGSET_SIGSEGV failed. Function not tested");
		exit(99);
	}
      if((sigaddset (&sigset_tmp, SIGSEGV))!=0){
		perror ("sigaddset at SIGSET_SIGSEGV failed. Function not tested");
		exit(99);
	}
      _theVariable = &sigset_tmp;
      }

     SIGSET_GARBAGE{
	char *buf = (char *)&sigset_tmp;
	for ( int i=0 ; i< sizeof (sigset_t); i++ ) *buf++ = 'a';
      _theVariable = &sigset_tmp;
      }


]

commit
[
{
#ifdef DEBUG
        printf("VALUE:%x %x in commit\n",_theVariable, *_theVariable);
#endif
}
]

cleanup
[
  {
#ifdef DEBUG
        printf("VALUE:%x %x in cleanup\n",_theVariable, *_theVariable);
#endif
  }

]
