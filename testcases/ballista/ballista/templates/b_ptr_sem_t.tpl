// b_ptr_sem_t.tpl : Ballista Datatype Template for semaphore pointer
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

name structsemtptr b_ptr_sem_t;

parent b_ptr_buf;

includes
[
{
#include <semaphore.h>
#define structsemtptr sem_t*
#include <fcntl.h>
#include "b_ptr_buf.h"
}
]

global_defines
[
{
  #define SEMAPHORE_NAME "/tmp/ballista_semaphore"
  static sem_t sem;
  static sem_t* temp_sem;
}
]

dials
[
  enum_dial HVAL : 
     OPEN,
     CLOSED,
     INIT,
     DESTROYED,
     UNLINKED; 

]

access
[
{
  system("chmod 777 /tmp/ballista_semaphore");
  system("rm /tmp/ballista_semaphore");
}

  OPEN, CLOSED, UNLINKED
  {
    // mode and value parameters are necessary
    temp_sem = sem_open(SEMAPHORE_NAME, O_CREAT);
    sem = *temp_sem;
    _theVariable = temp_sem;
  }

//  CLOSED,UNLINKED
  UNLINKED
  {   
    if ((sem_unlink(SEMAPHORE_NAME)) == -1)
    {
      FILE* logFile = NULL;
      if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
      {
        exit(99);
      }
      fprintf (logFile, "b_ptr_sem_t CLOSED error closing semaphore - function not tested \n");
      fclose(logFile);
      exit(99);
    }
    _theVariable = temp_sem;
  }
 
  CLOSED
  {
    if ((sem_close(temp_sem)) == -1)
    {
      FILE* logFile = NULL;
      if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
      {
        exit(99);
      }
      fprintf (logFile, "b_ptr_sem_t CLOSED error closing semaphore - function not tested \n");
      fclose(logFile);
      exit(99);
    }
    _theVariable = temp_sem;
  }

  INIT, DESTROYED
  {
    temp_sem = &sem;
    if ((sem_init(temp_sem, 1, 1)) == -1)
    {
      FILE* logFile = NULL;
      if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
      {
        exit(99);
      }
      fprintf (logFile, "b_ptr_sem_t INIT/DESTROYED error initializing semaphore - function not tested \n");
      fclose(logFile);
      exit(99);   
    }
    _theVariable = temp_sem;
  }

  DESTROYED
  {
    if ((sem_destroy(temp_sem)) == -1)
    {
      FILE* logFile = NULL;
      if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
      {
        exit(99);
      }
      fprintf (logFile, "b_ptr_sem_t DESTOYED error destroying semaphore - function not tested \n");
      fclose(logFile);
      exit(99);   
    }
    _theVariable = temp_sem;
  }

]

commit
[
]

cleanup
[
  INIT
  {
    sem_destroy(temp_sem);
  }
  OPEN
  {
    sem_unlink(SEMAPHORE_NAME);
  }
  OPEN,UNLINKED
  {
    sem_close(temp_sem);
  }
{
  system("chmod 777 /tmp/ballista_semaphore");
  system("rm /tmp/ballista_semaphore");
}

]
