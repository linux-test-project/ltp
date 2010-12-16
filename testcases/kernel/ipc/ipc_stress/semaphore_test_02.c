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
|                           semaphore_test_02                          |
| ==================================================================== |
|                                                                      |
| Description:  Verify semget () and semctl () options                 |
|               Also uses semop in linux (semctl(GETPID) requires semop|
|                                                                      |
| Algorithm:    o  Spawn N child processes                             |
|                                                                      |
|               o  Obtain N semaphores with semget (IPC_PRIVATE)       |
|                                                                      |
|               o  Call semctl () with following commands:             |
|                     IPC_SET:  set uid, gid & mode                    |
|                     IPC_STAT: get uid, gid, mode & verify            |
|                     SETVAL:   set each semaphore value individually  |
|                     GETVAL:   get each semaphore value & verify      |
|                #if linux then call semop
|                     GETPID:   get pid of last operation & verify     |
|                     GETNCNT:  get semncnt & verify                   |
|                     GETZCNT:  get semzcnt & verify                   |
|                     SETALL:   set all the semaphores                 |
|                     GETALL:   get all the semaphores & verify values |
|                     IPC_RMID: remove the N semaphores                |
|                                                                      |
| System calls: The following system calls are made                    |
|                                                                      |
|               semget () - Gets a set of semaphores                   |
|               semctl () - Controls semaphore operations              |
|                                                                      |
| Usage:        semaphore_test_02 [-p nprocs]                          |
|                                                                      |
| To compile:   cc -o semaphore_test_02 semaphore_test_02.c            |
|                                                                      |
| Last update:   Ver. 1.2, 2/14/94 00:18:23                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     050689  CTU   Initial version                             |
|    0.2     111993  DJK   Modify for AIX version 4.1                  |
|    1.2     021494  DJK   Moved to "prod" directory                   |
|							               |
|    1.3     Jan-28-02 	Manoj Iyer, IBM Austin TX. manjo@austin.ibm.com|
|			Modified semctl() to work in linux. Also the   |
|			#ifdef _LINUX_ was removed from the code.      |
|								       |
|                                                                      |
+---------------------------------------------------------------------*/

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/param.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

/*
 * Defines
 *
 * NUM_SEMAPHORES: number of semaphores to create
 */
# define MAX_SEMAPHORES	250
# define MAX_CHILDREN		200
# define DEFAULT_NUM_SEMAPHORES	96
# define DEFAULT_NUM_CHILDREN	0

#define USAGE	"\nUsage: %s [-s nsems] [-p nproc]\n\n" \
		"\t-s nsems  number of semaphores (per process)\n\n"	\
		"\t-p nproc  number of child processes to spawn\n\n"

#define SAFE_FREE(p) { if (p) { free(p); (p)=NULL; } }
/*
 * Function prototypes
 *
 * test_commands (): Tests semget () and semctl () commands
 * parse_args (): Parse command line arguments
 * sys_error (): System error message function
 * error (): Error message function
 */
static void test_commands (pid_t);
static void sys_error (const char *, int);
static void error (const char *, int);
static void parse_args (int, char **);

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
int     childpid [MAX_CHILDREN];
int	errors = 0;
pid_t   parent_pid;

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
	int	proc;			/* fork loop index */
	pid_t	pid;			/* Process id */
	int	status;			/* Child's exit status */

        /*
         * Parse command line arguments and print out program header
         */
        parse_args (argc, argv);
        printf ("%s: IPC Semaphore TestSuite program\n", *argv);
	fflush (stdout);
	parent_pid = getpid ();

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
			error ("fork failed", __LINE__);
		else if (pid == 0)
			break;
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

                if (!WIFEXITED (status))
                        error ("child process terminated abnormally",
                                __LINE__);
        }
	if (nprocs > 0)
		printf ("\n\tAll child processes verified commands successfully\n");
        printf ("\nsuccessful!\n");
	return (errors);
}

/*---------------------------------------------------------------------+
|                   test_commands (pid_t pid)                          |
| ==================================================================== |
|                                                                      |
| Function:  test semget() and semctl()                                |
|                                                                      |
+---------------------------------------------------------------------*/
static void test_commands (pid_t pid)
{
	int	i;			/* loop index */
	int	semid;			/* Unique semaphore id */
	int	val;			/* Misc value */
	gid_t	gid = getgid ();	/* User's group id */
	mode_t	mode = 0666;		/* User's mode value */
	uid_t	uid = getuid ();	/* User's user id */
	struct sembuf sops;
	union  semun semunptr;		/* This union has struct semid_ds *buf*/
	/*
	 * Test semget () with IPC_PRIVATE command
	 *
	 * Create nsems semaphores and store the returned unique
	 * semaphore identifier as semid.
	 */
	if (pid == parent_pid)
		printf ("\n\tTesting semctl (IPC_SET) command operation\n");
	if ((semid = semget (IPC_PRIVATE, nsems, IPC_CREAT|0666)) < 0)
		error ("semget failed", __LINE__);

	/*
	 * Test semctl () with IPC_SET command
	 *
	 * Set the uid, gid and mode fields
	 */
	if (pid == parent_pid)
		printf ("\n\tTesting semctl (IPC_SET) command operation\n");

	semunptr.buf = (struct semid_ds *) calloc(1, sizeof (struct semid_ds));
	if (!semunptr.buf)
		error("calloc failed", __LINE__);

	semunptr.buf->sem_perm.uid = uid;
	semunptr.buf->sem_perm.gid = gid;
	semunptr.buf->sem_perm.mode = mode;

	if (semctl (semid, 0, IPC_SET, semunptr) < 0)
		sys_error ("semctl failed", __LINE__);

	/*
	 * Test semctl () with IPC_STAT command
	 *
	 * Get the semid_ds structure and verify it's fields.
	 */
	if (pid == parent_pid)
		printf ("\n\tTesting semctl (IPC_STAT) command operation\n");

	if (semctl (semid, 0, IPC_STAT, semunptr) < 0)
		sys_error ("semctl failed", __LINE__);
	if (semunptr.buf->sem_perm.uid != uid)
		sys_error ("semctl: uid was not set", __LINE__);
	if (semunptr.buf->sem_perm.gid != gid)
		sys_error ("semctl: gid was not set", __LINE__);
	if ((semunptr.buf->sem_perm.mode & 0777) != mode)
		sys_error ("semctl: mode was not set", __LINE__);
	if (semunptr.buf->sem_nsems != nsems)
		sys_error ("semctl: nsems (number of semaphores) was not set",
			__LINE__);
	SAFE_FREE(semunptr.buf);

	/*
	 * Test semctl () with SETVAL command
	 *
	 * Loop through all the semaphores and set the semaphore value
	 * to the loop index (i).
	 */
	if (pid == parent_pid)
		printf ("\n\tTesting semctl (SETVAL) command operation\n");
	for (i = 0; i < nsems; i++) {
	      arg.val=i;
		if (semctl (semid, i, SETVAL, arg) < 0)
			sys_error ("semctl failed", __LINE__);
	}

	/*
	 * Test semctl () with GETVAL command
	 *
	 * Loop through all the semaphores and retrieve the semaphore values
	 * and compare with the expected value, the loop index (i).
	 */
	if (pid == parent_pid)
		printf ("\n\tTesting semctl (GETVAL) command operation\n");
	for (i = 0; i < nsems; i++) {
		if ((val = semctl (semid, i, GETVAL, arg)) < 0)
			sys_error ("semctl failed", __LINE__);
		if (val != i)
			sys_error ("semctl (GETVAL) failed", __LINE__);
	}

	// testing in linux.  before semctl(GETPID) works, we must call semop
	if (pid == parent_pid)
		printf ("\n\tTesting semop (signal and wait) operations\n");
	sops.sem_flg = 0;
	for (i = 0; i < nsems; i++) {
	        sops.sem_num = i;
		sops.sem_op = 1;
	        if ((val = semop (semid, &sops, 1)) < 0)
		        sys_error ("semop signal failed", __LINE__);
		sops.sem_op = -1;
		if ((val = semop (semid, &sops, 1)) < 0)
		        sys_error ("semop wait failed", __LINE__);
	}

	/*
	 * Test semctl () with GETPID command
	 */
	if (pid == parent_pid)
		printf ("\n\tTesting semctl (GETPID) command operation\n");
	for (i = 0; i < nsems; i++) {
		if ((val = semctl (semid, i, GETPID, arg)) < 0)
			sys_error ("semctl failed", __LINE__);
		if (val != pid)
			sys_error ("semctl (GETPID) failed", __LINE__);
	}

	/*
	 * Test semctl () with GETNCNT command
	 *
	 * Get semncnt (the number of processes awaiting semval > currval)
	 * and insure that this value is 0...
	 *
	 * Note: A better test would include forking off a process that
	 *       waits for the semaphore so that semncnt would be nonzero.
	 */
	if (pid == parent_pid)
		printf ("\n\tTesting semctl (GETNCNT) command operation\n");
	for (i = 0; i < nsems; i++) {
		if ((val = semctl (semid, i, GETNCNT, arg)) < 0)
			sys_error ("semctl failed", __LINE__);
		if (val != 0)
			sys_error ("semctl (GETNCNT) returned wrong value",
				__LINE__);
	}

	/*
	 * Test semctl () with GETZCNT command
	 *
	 * Get semzcnt (the number of processes awaiting semval = currval)
	 * and insure that this value is 0...
	 *
	 * Note: A better test would include forking off a process that
	 *       waits for the semaphore so that semzcnt would be nonzero.
	 */
	if (pid == parent_pid)
		printf ("\n\tTesting semctl (GETZCNT) command operation\n");
	for (i = 0; i < nsems; i++) {
		if ((val = semctl (semid, i, GETZCNT, arg)) < 0)
			sys_error ("semctl failed", __LINE__);
		if (val != 0)
			sys_error ("semctl (GETZCNT) returned wrong value",
				__LINE__);
	}

	/*
	 * Test semctl () with SETALL command
	 *
	 * Set all of the semaphore values in the set.
	 */
	arg.array = malloc(sizeof(int) * nsems);
	if (!arg.array)
		error ("malloc failed", __LINE__);
	if (pid == parent_pid)
		printf ("\n\tTesting semctl (SETALL) command operation\n");
	for (i = 0; i < nsems; i++)
		arg.array[i] = i;
	if (semctl (semid, 0, SETALL, arg) < 0)
		sys_error ("semctl failed", __LINE__);

	/*
	 * Test semctl () with GETALL command
	 *
	 * Get all of the semaphore values in the set, and verify that
	 * they are all correct.
	 */
	if (pid == parent_pid)
		printf ("\n\tTesting semctl (GETALL) command operation\n");
	if (semctl (semid, nsems, GETALL, arg) < 0)
		sys_error ("semctl failed", __LINE__);
	for (i = 0; i < nsems; i++) {
		if (arg.array [i] != i)
			sys_error ("semaphore does not match expected value",
				__LINE__);
	}

	/*
	 * Test semctl () with IPC_RMID command
	 *
	 * Remove the semaphores
	 */
	if (pid == parent_pid)
		printf ("\n\tTesting semctl (IPC_RMID) command operation\n");
	if (semctl (semid, nsems, IPC_RMID, arg) < 0)
		sys_error ("semctl failed", __LINE__);
        SAFE_FREE(arg.array);

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
|                             sys_error ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Creates system error message and increments errors	       |
|                                                                      |
+---------------------------------------------------------------------*/
static void sys_error (const char *msg, int line)
{
	char syserr_msg [256];

	sprintf (syserr_msg, "%s: %s\n", msg, strerror (errno));
	fprintf (stderr, "ERROR [line: %d] %s\n", line, syserr_msg);
	errors++;
	/* error (syserr_msg, line); */
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