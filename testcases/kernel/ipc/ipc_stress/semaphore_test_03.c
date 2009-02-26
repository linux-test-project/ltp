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
|                           semaphore_test_03                          |
| ==================================================================== |
|                                                                      |
| Description:  Verify semop () command options                        |
|                                                                      |
| Algorithm:    o  Spawn N child processes                             |
|                                                                      |
|               o  Obtain N semaphores with semget (IPC_PRIVATE)       |
|                                                                      |
|               o  Call semop () with variations of the following      |
|                  parameters:                                         |
|                                                                      |
|                     sem_op:  negative, 0, positive                   |
|                     sem_flg: IPC_NOWAIT, SEM_UNDO                    |
|                                                                      |
| System calls: The following system calls are made                    |
|                                                                      |
|               semget () - Gets a set of semaphores                   |
|               semctl () - Controls semaphore operations              |
|               semop () - Performs semaphore operations               |
|                                                                      |
| Usage:        semaphore_test_03 [-p nprocs] [-s nsems]               |
|                                                                      |
|               where:  nprocs - number of child processes to spawn    |
|                       nsems  - number of semaphores (per process)    |
|                                                                      |
| To compile:   cc -o semaphore_test_03 semaphore_test_03.c            |
|                                                                      |
| Last update:   Ver. 1.6, 7/21/94 13:37:28                            |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     050689  CTU   Initial version                             |
|    0.2     111993  DJK   Modify for AIX version 4.1                  |
|    1.2     021494  DJK   Moved to "prod" directory                   |
|    1.3     Jan-28-02 	Manoj Iyer, IBM Austin, TX.manjo@austin.ibm.com|
|			Modified - Ported to work on PPC64.            |
|                                                                      |
+---------------------------------------------------------------------*/

#include <errno.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/param.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * Defines
 *
 * MAX_SEMAPHORES: maximum number of semaphores per id (limited by
 * maximum number of semaphore operations per call by semop () function).
 *
 * MAX_CHILDREN:   maximum number of child processes to spawn
 *
 * DEFAULT_NUM_SEMAPHORES: default number of semaphores to create unless
 * specified with (-s nsems) command line option
 *
 * DEFAULT_NUM_CHILDREN: default number of child processes to spawn unless
 * specified with (-p nprocs) command line option
 *
 * USAGE: usage statement macro
 *
 * SEMOP_TABLE: macro for printing attempted semop command combinations
 */
# define MAX_SEMAPHORES	        32
# define MAX_CHILDREN		200
# define DEFAULT_NUM_SEMAPHORES	16
# define DEFAULT_NUM_CHILDREN	0

#define USAGE	"\nUsage: %s [-s nsems] [-p nproc]\n\n" \
		"\t-s nsems  number of semaphores (per process)\n\n"	\
		"\t-p nproc  number of child processes to spawn\n\n"
#define SEMOP_TABLE(p1,p2,p3,p4)	\
	if (proc_pid == parent_pid)	\
		printf ("\t   %3d    %3d    %-10s %-20s\n", p1, p2, p3, p4)

#define SAFE_FREE(p) { if(p) { free(p); (p)=NULL; } }

/*
 * Function prototypes
 *
 * setup_signal_handler (): Sets up signal handler for SIGUSR1
 * test_commands (): Tests semget () and semctl () commands
 * handler (): Signal handler
 * sys_error (): System error message function
 * error (): Error message function
 * parse_args (): Parse command line arguments
 * catch: Signal catching function for SIGUSR1 signal
 */
static void setup_signal_handler ();
static void test_commands ();
static void sys_error (const char *, int);
static void error (const char *, int);
static void parse_args (int, char **);
static void catch (int);


/*
 * Structures and Global variables:
 *
 * nsems: number of semaphores to create (per process)
 * nprocs: number of child processes to spawn
 * childpid: array containing process id's of the child processes
 * parent_pid: process id of parent process
 */
int	nsems = DEFAULT_NUM_SEMAPHORES;
int	nprocs = DEFAULT_NUM_CHILDREN;
pid_t   childpid [MAX_CHILDREN];
pid_t   parent_pid;
pid_t   errpid;

union semun {
   int val;
   struct semid_ds *buf;
   unsigned short *array;
} arg;

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
	pid_t	pid;			/* Child's process id */
	int	proc;			/* Fork loop index */
	int	status;			/* Child's exit status */

        /*
         * Parse command line arguments, print out program header, setup
	 * signal handler (for SIGUSR1) and save parent process id.
         */
        parse_args (argc, argv);
        printf ("%s: IPC Semaphore TestSuite program\n", *argv);
	fflush (stdout);
	setup_signal_handler ();
	errpid = parent_pid = getpid ();

	if (nsems < 8) nsems = 8;

	/*
	 * Fork off the additional processes.
	 */
	if (nprocs > 0) {
		printf ("\n\tParent: spawning %d child processes\n", nprocs);
		fflush (stdout);
	}
	for (proc = 1; proc < nprocs; proc++) {
		/*
		 * Child leaves loop, parent continues to fork.
		 */
		if ((pid = fork ()) < 0)
			sys_error ("fork failed", __LINE__);
		else if (pid == (pid_t)0)
		    {
		        errpid = pid;
		        break;
		    }
		else
			childpid [proc] = pid;
	}
	pid = getpid();

        /*
         * Test the semget () and semctl () commands
         */
	test_commands (pid);

        /*
         * Finished testing commands, only parent process needs to continue
         */
        if (pid != parent_pid) exit (0);

        /*
         * Wait for all of the child processes to complete & check their
         * exit status.
         *
         * Upon completion of the child proccesses, exit program with success.
         */
        for (proc = 1; proc < nprocs; proc++) {
                waitpid (childpid [proc], &status, 0);

                if (WEXITSTATUS (status))
                        sys_error ("child process terminated abnormally",
                                __LINE__);
        }
	if (nprocs > 0)
		printf ("\n\tAll child processes verified commands successfully\n");
        printf ("\nsuccessful!\n");
	return (0);
}


/*---------------------------------------------------------------------+
|                             test_commands ()                         |
| ==================================================================== |
|                                                                      |
| Function:  Verifies options for semop () system function call.       |
|                                                                      |
+---------------------------------------------------------------------*/
static void test_commands (pid_t proc_pid)
{
	int	i;			/* Misc loop index */
	int	val;			/* Value (semctl parameter) */
	int	semid;			/* Unique semaphore id */
	int	status;			/* Child's exit status */
	int	expected_value;		/* Expected semaphore value */
	pid_t	pid;			/* Misc process id */
	gid_t	gid = getgid ();	/* Misc group id */
	uid_t	uid = getuid ();	/* Misc user id */
	mode_t	mode = 0666;		/* Misc mode bits */
	// ushort	array [MAX_SEMAPHORES];	/* Misc array of semaphore values */
	struct sembuf semoparray [MAX_SEMAPHORES];

	/*
	 * Create the semaphores...
	 */
	if (proc_pid == parent_pid)
		printf ("\n\tCreating %d semaphores ...\n", nsems);
	if ((semid = semget (IPC_PRIVATE, nsems, IPC_CREAT|mode)) < 0)
		sys_error ("semget (IPC_PRIVATE) failed", __LINE__);
		   
	/*
	 * Set the semaphore uid, gid and mode
	 */
	if (proc_pid == parent_pid)
		printf ("\n\tSetting semaphore uid, gid and mode ... semid = %d\n", semid);
	arg.buf = (struct semid_ds *) calloc (1, sizeof (struct semid_ds));
	if(!arg.buf)
		error("calloc failed", __LINE__);
	arg.buf->sem_perm.uid = uid;
	arg.buf->sem_perm.gid = gid;
	arg.buf->sem_perm.mode = mode;
	if (semctl (semid, 0, IPC_SET, arg) < 0)
		sys_error ("semctl failed", __LINE__);

	/*
	 * Verify that semaphore uid, gid and mode were set correctly
	 */
	if (proc_pid == parent_pid)
		printf ("\n\tVerifying semaphore info ...\n");
	if (semctl (semid, 0, IPC_STAT, arg) < 0)
		sys_error ("semctl (IPC_STAT) failed", __LINE__);
	if (arg.buf->sem_perm.uid != uid)
		error ("semctl: uid was not set", __LINE__);
	if (arg.buf->sem_perm.gid != gid)
		error ("semctl: gid was not set", __LINE__);
	if ((arg.buf->sem_perm.mode & 0777) != mode)
		error ("semctl: mode was not set", __LINE__);
	if (arg.buf->sem_nsems != nsems)
		error ("semctl: nsems (number of semaphores) was not set",
			__LINE__);
	SAFE_FREE(arg.buf);

	/*
	 * Set the value of each semaphore in the set to 2.
	 */
        arg.array = malloc(sizeof(int) * nsems);
	if(!arg.array)
		error("malloc failed", __LINE__);
	for (i = 0; i < nsems; i++)
		arg.array [i] = 2;
	if (semctl (semid, 0, SETALL, arg) < 0)
		sys_error ("semctl (SETALL) failed", __LINE__);
	SAFE_FREE(arg.array);

	/* ------------------------------------------------------------------ */
	/*  possibilities for sem_flg are:                                    */
	/*               0                                                    */
	/*               SEM_UN                                               */
	/*               IPC_NOWAIT                                           */
	/*               Return Immediately                                   */
	/* ------------------------------------------------------------------ */
	if (proc_pid == parent_pid) {
		printf ("\n\tTesting semop() with all Semaphore values, options and flags\n");
		printf ("\n\t   Semval Semop  Semflag    Description\n");
	}

	/* ------------------------------------------------------------------ */
	/* TEST # 1  --- semval = 2, sem_op = -1, sem_flg = 0                 */
	/*           --- semval > |sem_op| THEN (semval - |sem_op|) = 1       */
	/*           THE FOLLOWING SHOULD SHOW semval = 1.                    */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(2, -1, "0", "Obtain resource");
	for (i = 0; i < nsems; i++) {
		semoparray [i].sem_num = i;
		semoparray [i].sem_op  = -1;
		semoparray [i].sem_flg = 0;
	}
	if (semop (semid, semoparray, nsems) < 0)
		sys_error ("semop failed", __LINE__);

	expected_value = 1;
	for (i = 0; i < nsems; i++) {
		arg.val = 0;
		if ((val = semctl (semid, i, GETVAL, arg)) < 0)
			sys_error ("semctl (GETVAL) failed", __LINE__);
		if (val != expected_value)
			error ("incorrect semaphore value", __LINE__);
	}

	/* ------------------------------------------------------------------ */
	/* TEST # 2  --- semval = 1, sem_op = -1, sem_flg = 0                 */
	/*           --- semval = |sem_op| THEN (semval - |sem_op|) = 0       */
	/*           THE FOLLOWING SHOULD SHOW semval = 0                     */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(1, -1, "0", "Obtain resource");
	for (i = 0; i < nsems; i++) {
		semoparray [i].sem_num = i;
		semoparray [i].sem_op  = -1;
		semoparray [i].sem_flg = 0;
	}
	if (semop (semid, semoparray, nsems) < 0)
		sys_error ("semop failed", __LINE__);

	expected_value = 0;
	for (i = 0; i < nsems; i++) {
		arg.val = 0;
		if ((val = semctl (semid, i, GETVAL, arg)) < 0)
			sys_error ("semctl (GETVAL) failed", __LINE__);
		if (val != expected_value)
			error ("incorrect semaphore value", __LINE__);
	}

	/* ------------------------------------------------------------------ */
	/* TEST # 3 --- semval = 0, sem_op = 0, sem_flg = 0                   */
	/*          --- semop =  0 AND semval = 0 returns immediately.        */
	/*           THE FOLLOWING SHOULD SHOW semval = 0                     */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(0, 0, "0", "Semop function returns immediately");
	for (i = 0; i < nsems; i++) {
		semoparray [i].sem_num = i;
		semoparray [i].sem_op  = 0;
		semoparray [i].sem_flg = 0;
	}
	if (semop (semid, semoparray, nsems) < 0)
		sys_error ("semop failed", __LINE__);

	expected_value = 0;
	for (i = 0; i < nsems; i++) {
		arg.val = 0;
		if ((val = semctl (semid, i, GETVAL, arg)) < 0)
			sys_error ("semctl (GETVAL) failed", __LINE__);
		if (val != expected_value)
			error ("incorrect semaphore value", __LINE__);
	}

	/* ------------------------------------------------------------------ */
	/* TEST # 4 --- semval = 5, sem_op = 1, sem_flg = 0                   */
	/*          --- semop > 0 THEN (semval + sem_op) = 6                  */
	/*           THE FOLLOWING SHOULD SHOW semval = 6                     */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(5, 1, "0", "Return resource");
        arg.array = malloc(sizeof(int) * nsems);
	if(!arg.array)
		error("malloc failed", __LINE__);
	for (i = 0; i < nsems; i++) {
		arg.array [i] = 5;
	}
	if (semctl (semid, 0, SETALL, arg) < 0)
		sys_error ("semctl (SETALL) failed", __LINE__);
	SAFE_FREE(arg.array);

	for (i = 0; i < nsems; i++) {
		semoparray [i].sem_num = i;
		semoparray [i].sem_op  = 1;
		semoparray [i].sem_flg = 0;
	}
	if (semop (semid, semoparray, nsems) < 0)
		sys_error ("semop failed", __LINE__);

	expected_value = 6;
	for (i = 0; i < nsems; i++) {
		arg.val = 0;
		if ((val = semctl (semid, i, GETVAL, arg)) < 0)
			sys_error ("semctl (GETVAL) failed", __LINE__);
		if (val != expected_value)
			error ("incorrect semaphore value", __LINE__);
	}

	/* ------------------------------------------------------------------ */
	/* TEST # 5 --- semval = 6, sem_op = -7, sem_flg = IPC_NOWAIT         */
	/*          --- semval < |sem_op| && IPC_NOWAIT, THEN return immed.   */
	/*         THE FOLLOWING SHOULD SHOW semval = 6.                      */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(6, -7, "IPC_NOWAIT", "Semop function returns immediately");
	for (i = 0; i < nsems; i++) {
		semoparray [i].sem_num = i;
		semoparray [i].sem_op  = -7;
		semoparray [i].sem_flg = IPC_NOWAIT;
	}
	if (semop (semid, semoparray, nsems) >= 0)
		error ("semop did not return EAGAIN", __LINE__);
	else
		if (errno != EAGAIN)
			sys_error ("semop failed", __LINE__);

	expected_value = 6;
	for (i = 0; i < nsems; i++) {
		arg.val = 0;
		if ((val = semctl (semid, i, GETVAL, arg)) < 0)
			sys_error ("semctl (GETVAL) failed", __LINE__);
		if (val != expected_value)
			error ("incorrect semaphore value", __LINE__);
	}

	/* ------------------------------------------------------------------ */
	/* TEST # 6 --- semval = 6, sem_op = 0, sem_flg = IPC_NOWAIT          */
	/*          --- semop = 0 AND semval != 0 AND IPC_NOWAIT,          */
	/*          ---              THEN  return immediately.                */
	/*         THE FOLLOWING SHOULD SHOW semval = 6.                      */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(6, 0, "IPC_NOWAIT", "Semop function returns immediately");
	for (i = 0; i < nsems; i++) {
		semoparray [i].sem_num = i;
		semoparray [i].sem_op  = 0;
		semoparray [i].sem_flg = IPC_NOWAIT;
	}
	if (semop (semid, semoparray, nsems) >= 0)
		error ("semop did not return EAGAIN", __LINE__);
	else
		if (errno != EAGAIN)
			sys_error ("semop failed", __LINE__);

	expected_value = 6;
	for (i = 0; i < nsems; i++) {
		arg.val = 0;
		if ((val = semctl (semid, i, GETVAL, arg)) < 0)
			sys_error ("semctl (GETVAL) failed", __LINE__);
		if (val != expected_value)
			error ("incorrect semaphore value", __LINE__);
	}

	/* ------------------------------------------------------------------ */
	/* TEST # 7  --- semval = 6, sem_op = 1, sem_flg = 0                  */
	/*          --- semop > 0 THEN (semval + sem_op) = 7               */
	/*         THE FOLLOWING SHOULD SHOW semval = 7.                      */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(6, 1, "0", "Return resource");
	for (i = 0; i < nsems; i++) {
		semoparray [i].sem_num = i;
		semoparray [i].sem_op  = 1;
		semoparray [i].sem_flg = 0;
	}
	if (semop (semid, semoparray, nsems) < 0)
		sys_error ("semop failed", __LINE__);

	expected_value = 7;
	for (i = 0; i < nsems; i++) {
		arg.val = 0;
		if ((val = semctl (semid, i, GETVAL, arg)) < 0)
			sys_error ("semctl (GETVAL) failed", __LINE__);
		if (val != expected_value)
			error ("incorrect semaphore value", __LINE__);
	}

	/* ------------------------------------------------------------------ */
	/* TEST # 8 --- semval = 7, sem_op[0] = -8, sem_flg = 0               */
	/*          --- semval < |semop| && ! IPC_NOWAIT, caller sleeps       */
	/* call #1  --- semval = 7, sem_op[0] =  2, sem_flg = 0               */
	/*          --- semop > 0 THEN (semval + sem_op) = 9                  */
	/*           --- "child" is awaken via call #1.                       */
	/*         THE FOLLOWING SHOULD SHOW semval = 1                       */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(7, -8, "0", "Sleep (until resource becomes available)");
	/*
	 * Child process
	 */
	if ((pid = fork()) == (pid_t)0) {
		semoparray [0].sem_num = 0;
		semoparray [0].sem_op  = -8;
		semoparray [0].sem_flg = 0;
		if (semop (semid, semoparray, 1) < 0)
			sys_error ("semop failed", __LINE__);
		exit(0);
	} else if (pid < 0) {
		sys_error ("fork failed", __LINE__);
	}
	semoparray [0].sem_num = 0;
	semoparray [0].sem_op  = 2;
	semoparray [0].sem_flg = 0;

	/*
	 * Wait for child process's semaphore request before proceeding...
	 */
        while (!semctl (semid, 0, GETNCNT, arg))
                sleep (1);

	if (semop (semid, semoparray, 1) < 0)
		sys_error ("semop failed", __LINE__);

	waitpid (pid, &status, 0);	/* Wait for child to complete */
	if (WEXITSTATUS (status))
		sys_error ("child process terminated abnormally", __LINE__);

	expected_value = 1;
	arg.val = 0;
	if ((val = semctl (semid, 0, GETVAL, arg)) < 0)
		sys_error ("semctl (GETVAL) failed", __LINE__);
	if (val != expected_value)
		error ("incorrect semaphore value", __LINE__);

	/* ------------------------------------------------------------------ */
	/* TEST # 9  --- semval = 7, sem_op[0] = -8, sem_flg = 0              */
	/*           --- semval < |semop| && ! IPC_NOWAIT, caller sleeps      */
	/*           --- "child" is awaken via a signal.  AFTER AWAKENING,    */
	/*           --- semval > |sem_op| THEN (semval - |sem_op|) = 1       */
	/*         THE FOLLOWING SHOULD SHOW semval = 7                       */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(7, -8, "0", "Sleep (until signaled)");
	/*
	 * Child process
	 */
	if ((pid = fork()) == (pid_t)0) {
		semoparray [0].sem_num = 1;
		semoparray [0].sem_op  = -8;
		semoparray [0].sem_flg = 0;

		if (semop (semid, semoparray, 1) >= 0)
			error ("semop did not return EINTR", __LINE__);
		else
			if (errno != EINTR) {
				printf ("semop returned: %d\n", errno);
				sys_error ("semop failed", __LINE__);
			}
		exit (0);
	} else if (pid < (pid_t)0) {
		sys_error ("fork failed", __LINE__);
	}

	/*
	 * Wait for child process's semaphore request before proceeding...
	 */
        while (!semctl (semid, 1, GETNCNT, arg))
                sleep (1);

	kill (pid, SIGUSR1);

	waitpid (pid, &status, 0);	/* Wait for child to complete */
	if (WEXITSTATUS (status))
		sys_error ("child process terminated abnormally", __LINE__);

	expected_value = 7;
	arg.val = 0;
	if ((val = semctl (semid, 1, GETVAL, arg)) < 0)
		sys_error ("semctl (GETVAL) failed", __LINE__);
	if (val != expected_value)
		error ("incorrect semaphore value", __LINE__);

	/* ------------------------------------------------------------------ */
	/* TEST # 10 --- semval = 1, sem_op[3] = -3, sem_flg = 0              */
	/*           --- semval < |semop| && ! IPC_NOWAIT, caller sleeps      */
	/* call #1   --- semval = 1, sem_op[3] =  5, sem_flg = 0              */
	/*           --- sem_op > 0 THEN (semval + sem_op) = 6                */
	/* call #2   --- semval = 6, sem_op[3] =  5, sem_flg = SEM_UN         */
	/*           --- sem_op > 0 && SEM_UN, THEN                           */
	/*           ---               THEN (semval + sem_op) = 11            */
	/*           --- "child" is awaken via call #2.                       */
	/*           --- semval < |semop (-3)| THEN semval = 8                */
	/*         THE FOLLOWING SHOULD SHOW semval = 8                       */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(1, 5, "SEM_UNDO", "Sleep (until resource becomes available)");
	/*
	 * Child process
	 */
	if ((pid = fork()) == (pid_t)0) {
		semoparray [0].sem_num = 0;
		semoparray [0].sem_op  = -3;
		semoparray [0].sem_flg = 0;

		if (semop (semid, semoparray, 1) < 0)
			sys_error ("semop failed", __LINE__);
		exit (0);
	} else if (pid < (pid_t)0) {
		sys_error ("fork failed", __LINE__);
	}

	/*
	 * Wait for child process's semaphore request before proceeding...
	 */
        while (!semctl (semid, 0, GETNCNT, arg))
                sleep (1);

	semoparray [0].sem_num = 0;
	semoparray [0].sem_op  = 5;
	semoparray [0].sem_flg = 0;
	if (semop (semid, semoparray, 1) < 0)
		sys_error ("semop failed", __LINE__);

	semoparray [0].sem_num = 0;
	semoparray [0].sem_op  = 5;
	semoparray [0].sem_flg = SEM_UNDO;
	if (semop (semid, semoparray, 1) < 0)
		sys_error ("semop failed", __LINE__);

	waitpid (pid, &status, 0);	/* Wait for child to complete */
	if (WEXITSTATUS (status))
		sys_error ("child process terminated abnormally", __LINE__);

	expected_value = 8;
	arg.val = 0;
	if ((val = semctl (semid, 0, GETVAL, arg)) < 0)
		sys_error ("semctl (GETVAL) failed", __LINE__);
	if (val != expected_value)
		error ("incorrect semaphore value", __LINE__);

	/* ------------------------------------------------------------------ */
	/* TEST # 11 --- semval = 7, sem_op[3] = -8, sem_flg = 0              */
	/*           --- semval < |semop| && ! IPC_NOWAIT, caller sleeps      */
	/*           --- "child" is awaken via removal of semaphores          */
	/*         THE FOLLOWING SHOULD SHOW now be destroyed                 */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(7, -8, "0", "Sleep (until semaphores are removed)");
	/*
	 * Child process
	 */
	if ((pid = fork()) == (pid_t)0) {
		semoparray [0].sem_num = 2;
		semoparray [0].sem_op  = -8;
		semoparray [0].sem_flg = 0;

		if (semop (semid, semoparray, 1) >= 0)
			error ("semop did not return ERMID", __LINE__);
		else
			if (errno != EIDRM) {
				printf ("semop returned: %d\n", errno);
				sys_error ("semop failed", __LINE__);
			}
		exit (0);
	} else if (pid < (pid_t)0) {
		sys_error ("fork failed", __LINE__);
	}

	/*
	 * Wait for child process's semaphore request before deleting the
	 * semaphores...
	 */
        while (!semctl (semid, 2, GETNCNT, arg))
	  sleep (1);

	arg.val = 0;
	if (semctl (semid, 0, IPC_RMID, arg) < 0)
		sys_error ("semctl (IPC_RMID) failed", __LINE__);

	waitpid (pid, &status, 0);	/* Wait for child to complete */
	if (WEXITSTATUS (status))
		sys_error ("child process terminated abnormally", __LINE__);

	/* ------------------------------------------------------------------ */
	/*        IPC_RMID DESTROYED THE SEMAPHORE STRUCTURES.                */
	/*        THEREFORE:  REBUILD A SEMAPHORE STRUCTURE SET.              */
	/* ------------------------------------------------------------------ */
	/*
	 * Create the semaphores...
	 */
	if ((semid = semget (IPC_PRIVATE, nsems, IPC_CREAT|mode)) < 0)
		sys_error ("semget (IPC_PRIVATE) failed", __LINE__);
		   
	/*
	 * Set the semaphore uid, gid and mode
	 */
	arg.buf = (struct semid_ds *) calloc (1, sizeof (struct semid_ds));
	if(!arg.buf)
		error("calloc failed", __LINE__);
	arg.buf->sem_perm.uid = uid;
	arg.buf->sem_perm.gid = gid;
	arg.buf->sem_perm.mode = mode;
	if (semctl (semid, 0, IPC_SET, arg) < 0)
		sys_error ("semctl failed", __LINE__);

	/*
	 * Verify that semaphore uid, gid and mode were set correctly
	 */
	if (semctl (semid, 0, IPC_STAT, arg) < 0)
		sys_error ("semctl (IPC_STAT) failed", __LINE__);
	if (arg.buf->sem_perm.uid != uid)
		error ("semctl: uid was not set", __LINE__);
	if (arg.buf->sem_perm.gid != gid)
		error ("semctl: gid was not set", __LINE__);
	if ((arg.buf->sem_perm.mode & 0777) != mode)
		error ("semctl: mode was not set", __LINE__);
	if (arg.buf->sem_nsems != nsems)
		error ("semctl: nsems (number of semaphores) was not set",
			__LINE__);
	SAFE_FREE(arg.buf);

        arg.array = malloc(sizeof(int) * nsems);
	if(!arg.array)
		error("malloc failed", __LINE__);
	for (i = 0; i < nsems; i++)
		arg.array [i] = 9;
	if (semctl (semid, 0, SETALL, arg) < 0)
		sys_error ("semctl (SETALL) failed", __LINE__);
	SAFE_FREE(arg.array);


	/* ------------------------------------------------------------------ */
	/* TEST # 12 --- semval = 9, sem_op = -1, sem_flg = SEM_UN          */
	/*           --- semval > |sem_op| THEN (semval - |sem_op|) = 8       */
	/*           ---                   ALSO (semadj = semadj + sem_op)    */
	/*         THE FOLLOWING SHOULD SHOW semval = 8                       */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(9, -1, "SEM_UNDO", "Obtain resource");
	for (i = 0; i < nsems; i++) {
		semoparray [i].sem_num = i;
		semoparray [i].sem_op  = -1;
		semoparray [i].sem_flg = SEM_UNDO;
	}
	if (semop (semid, semoparray, nsems) < 0)
		sys_error ("semop failed", __LINE__);

	expected_value = 8;
	for (i = 0; i < nsems; i++) {
		arg.val = 0;
		if ((val = semctl (semid, 0, GETVAL, arg)) < 0)
			sys_error ("semctl (GETVAL) failed", __LINE__);
		if (val != expected_value)
			error ("incorrect semaphore value", __LINE__);
	}

	/* ------------------------------------------------------------------ */
	/* TEST # 13 --- semval = 8, sem_op = -8, sem_flg = SEM_UN          */
	/*           --- semval = |sem_op| THEN (semval - |sem_op|) = 0       */
	/*           ---                   ALSO (semadj = semadj + sem_op)    */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(8, -8, "SEM_UNDO", "Obtain resource");
	for (i = 0; i < nsems; i++) {
		semoparray [i].sem_num = i;
		semoparray [i].sem_op  = -8;
		semoparray [i].sem_flg = SEM_UNDO;
	}
	if (semop (semid, semoparray, nsems) < 0)
		sys_error ("semop failed", __LINE__);

	expected_value = 0;
	for (i = 0; i < nsems; i++) {
		arg.val = 0;
		if ((val = semctl (semid, 0, GETVAL, arg)) < 0)
			sys_error ("semctl (GETVAL) failed", __LINE__);
		if (val != expected_value)
			error ("incorrect semaphore value", __LINE__);
	}

        arg.array = malloc(sizeof(int) * nsems);
	if(!arg.array)
		error("malloc failed", __LINE__);
	for (i = 0; i < nsems; i++)
		arg.array [i] = 9;
	if (semctl (semid, 0, SETALL, arg) < 0)
		sys_error ("semctl (SETALL) failed", __LINE__);
	SAFE_FREE(arg.array);

	/* ------------------------------------------------------------------ */
	/* TEST # 14 --- semval = 9, sem_op[4] = 0, sem_flg = 0               */
	/*           --- semval != 0 && ! IPC_NOWAIT     caller sleeps        */
	/*           ---                   ALSO  ++semzcnt                    */
	/*           --- "child" is awaken via a signal.                      */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(9, 0, "0", "Sleep (until signaled)");
	/*
	 * Child process
	 */
	if ((pid = fork()) == 0) {
		semoparray [0].sem_num = 0;
		semoparray [0].sem_op  = 0;
		semoparray [0].sem_flg = 0;

		if (semop (semid, semoparray, 1) >= 0)
			error ("semop did not return EINTR", __LINE__);
		else
			if (errno != EINTR) {
				printf ("semop returned: %d\n", errno);
				sys_error ("semop failed", __LINE__);
			}
		exit (0);
	} else if (pid < 0) {
		sys_error ("fork failed", __LINE__);
	}

	/*
	 * Wait for child process's semaphore request before proceeding...
	 */
        while (!semctl (semid, 0, GETZCNT, arg))
                sleep (1);

	kill (pid, SIGUSR1);

	waitpid (pid, &status, 0);	/* Wait for child to complete */
	if (WEXITSTATUS (status))
		sys_error ("child process terminated abnormally", __LINE__);

	expected_value = 9;
	arg.val = 0;
	if ((val = semctl (semid, 0, GETVAL, arg)) < 0)
		sys_error ("semctl (GETVAL) failed", __LINE__);
	if (val != expected_value)
		error ("incorrect semaphore value", __LINE__);

	/* ------------------------------------------------------------------ */
	/* TEST # 15 --- semval = 9, sem_op[0] = 0, sem_flg = 0               */
	/*           --- semval != 0 && ! IPC_NOWAIT     caller sleeps     */
	/*           ---                   ALSO  ++semzcnt                    */
	/* call #1   --- semval = 9, sem_op[0] = -9, sem_flg = 0              */
	/*           ---    THEN (semval - |sem_op|) = 0 and  --semzcnt       */
	/*           --- "child" is awaken via call #1.                       */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(9, 0, "0", "Sleep (until resource becomes available)");
	/*
	 * Child process
	 */
	if ((pid = fork()) == (pid_t)0) {
		semoparray [0].sem_num = 0;
		semoparray [0].sem_op  = 0;
		semoparray [0].sem_flg = 0;
		if (semop (semid, semoparray, 1) < 0)
			sys_error ("semop failed", __LINE__);
		exit(0);
	} else if (pid < (pid_t)0) {
		sys_error ("fork failed", __LINE__);
	}

	/*
	 * Wait for child process's semaphore request before proceeding...
	 */
        while (!semctl (semid, 0, GETZCNT, arg))
                sleep (1);

	semoparray [0].sem_num = 0;
	semoparray [0].sem_op  = -9;
	semoparray [0].sem_flg = 0;

	if (semop (semid, semoparray, 1) < 0)
		sys_error ("semop failed", __LINE__);

	waitpid (pid, &status, 0);	/* Wait for child to complete */
	if (WEXITSTATUS (status))
		sys_error ("child process terminated abnormally", __LINE__);

	expected_value = 0;
	arg.val = 0;
	if ((val = semctl (semid, 0, GETVAL, arg)) < 0)
		sys_error ("semctl (GETVAL) failed", __LINE__);
	if (val != expected_value)
		error ("incorrect semaphore value", __LINE__);

	/* ------------------------------------------------------------------ */
	/* TEST # 16 --- semval = 4, sem_op[4] = 0, sem_flg = 0               */
	/*           --- semval != 0 && ! IPC_NOWAIT     caller sleeps     */
	/*           ---                   ALSO  ++semzcnt                    */
	/*           --- "child" is awaken via removal of semaphores          */
	/* ------------------------------------------------------------------ */
	SEMOP_TABLE(4, 0, "0", "Sleep (until semaphores are removed)");
	/*
	 * Child process
	 */
	arg.val = 4;
	if (semctl (semid, 4, SETVAL, arg) < 0)
		sys_error ("semctl (SETALL) failed", __LINE__);

	if ((pid = fork()) == (pid_t)0) {
		semoparray [0].sem_num = 4;
		semoparray [0].sem_op  = -8;
		semoparray [0].sem_flg = 0;

		if (semop (semid, semoparray, 1) >= 0)
			error ("semop did not return ERMID", __LINE__);
		else
			if (errno != EIDRM) {
				printf ("semop returned: %d\n", errno);
				sys_error ("semop failed", __LINE__);
			}
		exit (0);
	} else if (pid < 0) {
		sys_error ("fork failed", __LINE__);
	}

	/*
	 * Wait for child process's semaphore request before proceeding...
	 */
        while (!semctl (semid, 4, GETNCNT, arg))
                sleep (1);

	arg.val = 0;
	if (semctl (semid, 0, IPC_RMID, arg) < 0)
		sys_error ("semctl (IPC_RMDI) failed", __LINE__);

	waitpid (pid, &status, 0);	/* Wait for child to complete */
	if (WEXITSTATUS (status))
		sys_error ("child process terminated abnormally", __LINE__);
}


/*---------------------------------------------------------------------+
|                             parse_args ()                            |
| ==================================================================== |
|                                                                      |
| Function:  Parse the command line arguments & initialize global      |
|            variables.                                                |
|                                                                      |
| Updates:   (command line options)                                    |
|                                                                      |
|            [-p] nproc: number of child processes                     |
|                                                                      |
+---------------------------------------------------------------------*/
void parse_args (int argc, char **argv)
{
	int	opt;
	int	errflag = 0;
	char	*program_name = *argv;
	extern char 	*optarg;	/* Command line option */

	while ((opt = getopt(argc, argv, "s:p:")) != EOF) {
		switch (opt) {
			case 's':
				nsems = atoi (optarg);
				break;
			case 'p':
				nprocs = atoi (optarg);
				break;
			default:
				errflag++;
				break;
		}
	}
	if (nsems >= MAX_SEMAPHORES) {
		errflag++;
		fprintf (stderr, "ERROR: nsems must be less than %d\n",
			MAX_SEMAPHORES);
	}
	if (nprocs >= MAX_CHILDREN) {
		errflag++;
		fprintf (stderr, "ERROR: nproc must be less than %d\n",
			MAX_CHILDREN);
	}

	if (errflag) {
		fprintf (stderr, USAGE, program_name);
		exit (2);
	}
}


/*---------------------------------------------------------------------+
|                          setup_signal_handler ()                     |
| ==================================================================== |
|                                                                      |
| Function:  Sets up signal handler for SIGUSR1 signal                 |
|                                                                      |
+---------------------------------------------------------------------*/
static void setup_signal_handler ()
{
        struct sigaction sigact;

        sigact.sa_flags = 0;
        sigfillset (&sigact.sa_mask);

        /*
         * Establish the signal handler for SIGUSR1
         */
        sigact.sa_handler = (void (*)(int)) catch;
        if (sigaction (SIGUSR1, &sigact, NULL) < 0)
                sys_error ("sigaction failed", __LINE__);
}


/*---------------------------------------------------------------------+
|                             catch ()                                 |
| ==================================================================== |
|                                                                      |
| Function:  Signal catching function for SIGUSR1                      |
|                                                                      |
+---------------------------------------------------------------------*/
static void catch (int sig)
{
	char	err_msg [256];
	pid_t	pid = getpid ();

	if (sig == SIGUSR1) {
		if (pid == parent_pid)
			printf ("\t\t\t\t    <<< caught signal (SIGUSR1, %d) >>>\n",
				sig);
	} else {
		sprintf (err_msg, "caught unexpected signal (%d)", sig);
		error (err_msg, __LINE__);
	}
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
	fprintf (stderr, "ERROR pid %d [line: %d] %s\n", errpid, line, msg);
	exit (-1);
}
