/*
 *   Copyright (C) Bull S.A. 1996
 *   Copyright (c) International Business Machines  Corp., 2001
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
 */
/*---------------------------------------------------------------------+
|                           semaphore_test_01                          |
| ==================================================================== |
|                                                                      |
| Description:  Simplistic test to verify the semget () and semctl ()  |
|               system function calls.                                 |
|                                                                      |
|               o  Create semaphore with semget ()                     |
|                                                                      |
|               o  Set the semaphore user id, group id and permission  |
|                  bits with semctl (IPC_SET)                          |
|                                                                      |
|               o  Retrieve the semaphore status with semctl (IPC_STAT)|
|                  and compare with expected values                    |
|                                                                      |
|               o  Print out the semaphore id for comparison with the  |
|                  output of the 'ipcs -s' command                     |
|                                                                      |
| System calls: The following system calls are made                    |
|                                                                      |
|               semget () - Gets a set of semaphores                   |
|               semctl () - Controls semaphore operations              |
|                                                                      |
| Usage:        semaphore_test_01                                      |
| NOTE: this program creates a system semaphore, but never deletes it. |
|       In order to delete it, use `ipcrm sem <id_num>`.  An alternate |
|       to this is to run the program as follows:                      |
|               ipcrm sem `./semaphore_test_01`                        |
|       This will delete the semaphore after the program exits, but    |
|       does not print out the semaphore id to see.                    |
|                                                                      |
| To compile:   cc -o semaphore_test_01 semaphore_test_01.c            |
|                                                                      |
| Last update:   Ver. 1.2, 2/14/94 00:18:16                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     050689  CTU   Initial version                             |
|    0.2     111993  DJK   Modify for AIX version 4.1                  |
|    1.2     021494  DJK   Move to "prod" directory                    |
|                                                                      |
+---------------------------------------------------------------------*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#ifdef _LINUX_
# include <sys/stat.h>
#endif

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
  int val;                  /* value for SETVAL */
  struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
  unsigned short *array;    /* array for GETALL, SETALL */
                            /* Linux specific part: */
  struct seminfo *__buf;    /* buffer for IPC_INFO */
};
#endif

/* Defines
 *
 * NUM_SEMAPHORES: number of semaphores to create
 */
#define NUM_SEMAPHORES	1

/*
 * Function prototypes
 *
 * sys_error (): System error message function
 * error (): Error message function
 */
static void sys_error (const char *, int);
static void error (const char *, int);

/*---------------------------------------------------------------------+
|                               main                                   |
| ==================================================================== |
|                                                                      |
| Function:  Main program  (see prolog for more details)               |
|                                                                      |
| Returns:   (0)  Successful completion                                |
|            (-1) Error occurred                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int main (int argc, char **argv)
{
	uid_t uid = getuid ();		/* User's user id */
	gid_t gid = getgid ();		/* User's group id */
	int semid;			/* Unique semaphore id */
	int nsems = NUM_SEMAPHORES;	/* Number of semaphores to create */
	struct semid_ds exp_semdata;	/* Expected semaphore values */
	union semun exp_semdatap;
	struct semid_ds act_semdata;	/* Actual semaphore values */
	union semun act_semdatap;

	exp_semdatap.buf = &exp_semdata;
	act_semdatap.buf = &act_semdata;

	umask (0000);

	/* SET semid_ds STRUCTURE TO DESIRED VALUES........ */

	/*
	 * Initialize the "expected" sempahore value structure
	 */
	exp_semdata.sem_perm.cuid = exp_semdata.sem_perm.uid  = uid;
	exp_semdata.sem_perm.cgid = exp_semdata.sem_perm.gid  = gid;
	exp_semdata.sem_perm.mode = 0660;
	exp_semdata.sem_nsems     = nsems;

	/*
	 * Create a semaphore, set the semaphore fields and then
	 * retrieve the fields.
	 */
	if ((semid = semget (IPC_PRIVATE, nsems, IPC_CREAT|0666)) < 0)
		sys_error ("semget (IPC_PRIVATE) failed", __LINE__);

	if (semctl (semid, nsems, IPC_SET, exp_semdatap) < 0)
		sys_error ("semctl (IPC_SET) failed", __LINE__);

	if (semctl (semid, nsems, IPC_STAT, act_semdatap) < 0)
		sys_error ("semctl (IPC_STAT) failed", __LINE__);

	/*
	 * Verify that the semaphore fields were set correctly
	 */
	if (act_semdata.sem_perm.cuid != exp_semdata.sem_perm.cuid)
		error ("sem_perm.cuid field was not set!", __LINE__);
	if (act_semdata.sem_perm.uid != exp_semdata.sem_perm.uid)
		error ("sem_perm.uid field was not set!", __LINE__);
	if (act_semdata.sem_perm.cgid != exp_semdata.sem_perm.cgid)
		error ("sem_perm.cgid field was not set!", __LINE__);
	if (act_semdata.sem_perm.gid != exp_semdata.sem_perm.gid)
		error ("sem_perm.gid field was not set!", __LINE__);
	if (act_semdata.sem_perm.mode != exp_semdata.sem_perm.mode)
		error ("sem_perm.mode field was not set!", __LINE__);
	if (act_semdata.sem_nsems != exp_semdata.sem_nsems)
		error ("sem_nsems field was not set!", __LINE__);

	/*
	 * Print out the id of the newly created semaphore for comparison
	 * with the 'ipcs -s' command and then exit.
	 */
	printf ("%d\n", semid);

	return (0);
}

/*---------------------------------------------------------------------+
|                             sys_error ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Creates system error message and calls error ()           |
|                                                                      |
+---------------------------------------------------------------------*/
static void sys_error (const char *msg, int line)
{
	char syserr_msg [256];

	sprintf (syserr_msg, "%s: %s\n", msg, strerror (errno));
	error (syserr_msg, line);
}

/*---------------------------------------------------------------------+
|                               error ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Prints out message and exits...                           |
|                                                                      |
+---------------------------------------------------------------------*/
static void error (const char *msg, int line)
{
	fprintf (stderr, "ERROR [line: %d] %s\n", line, msg);
	exit (-1);
}