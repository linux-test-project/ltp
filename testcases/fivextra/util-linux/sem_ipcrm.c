/*
 *
 *   (C) Copyright IBM Corp. 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 *  FILE        : semaphore.c
 *  DESCRIPTION : Creates a semaphore for testing the ipcrm command
 *
 *  HISTORY:
 *    03/04/2003 Andrew Pham (apham@us.ibm.com)
 *      -written
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/types.h>

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

int errors = 0;

int main(int argc, char *argv[]) {
  int semid;
  union semun semunion;
  
 /* set up the semaphore */
  if((semid = semget((key_t)9142, 1, 0666 | IPC_CREAT)) < 0) {
    return -1;
  }
  semunion.val = 1;
  if(semctl(semid, 0, SETVAL, semunion) == -1) {
    return -1;
  }

  printf ("%d", semid);
  return semid;
}
