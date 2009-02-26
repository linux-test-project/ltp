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
|                           shmem_test_03                              |
| ==================================================================== |
|                                                                      |
| Description:  Verify shared memory mapping of /dev/zero with         |
|               exclusive writes and shared reads using semaphores     |
|               as the synchronization method.                         |
|                                                                      |
| Algorithm:    o  Obtain three shared memory segments by mapping      |
|                  /dev/zero, one for random data created by the       |
|                  parent, another for the childs checksums, and the   |
|                  last for the read count (number of child processes  |
|                  reading the data).                                  |
|               o  Spawn N child processes                             |
|               o  Parent:                                             |
|                     - obtain write lock (exclusive) on data          |
|                     - fill shared memory segment with data           |
|                     - compute data checksum                          |
|                     - release lock                                   |
|               o  Child:                                              |
|                     - obtain read lock (shared) on data              |
|                     - compute data checksum                          |
|                     - release lock                                   |
|                     - store checksum in other shared memory segment  |
|               o  Parent:                                             |
|                     - wait for child proceeses to complete           |
|                     - compare child's checksum (from shared memory)  |
|                       with that of the parent                        |
|                                                                      |
| System calls: The following system calls are tested:                 |
|                                                                      |
|               mmap () - Maps a file-system object into memory        |
|               semget () - Gets a set of semaphores                   |
|               semctl () - Controls semaphore operations              |
|               semop () - Preforms semaphore operations               |
|                                                                      |
| Usage:        shmem_test_03 [-c num_children] [-s shmem_size]        |
|                                                                      |
| To compile:   cc -o shmem_test_03 shmem_test_03.c                    |
|                                                                      |
| Last update:   Ver. 1.2, 2/8/94 00:08:45                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     012093  DJK   Initial version for AIX 4.1                 |
|    1.2     020794  DJK   Moved to "prod" directory                   |
|                                                                      |
+---------------------------------------------------------------------*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Defines
 *
 * USAGE: usage statement
 */
#define MAX_CHILDREN		400
#define DEFAULT_NUM_CHILDREN	2
#define DEFAULT_SHMEM_SIZE	100000
#define USAGE	"\nUsage: %s [-c num_children] [-s shmem_size]\n\n" \
		"\t-c num_children   number of child processes to spawn\n" \
		"\t-s shmem_size     size of shared memory segment (bytes)\n" \
		"\t                  (must be less than 256MB!)\n\n"

/*
 * Function prototypes
 *
 * create_semaphores (): Create semaphores for synchorizing memory accesses
 * delete_semaphores (): Delete the semaphores
 * lock_resource (): Obtains the resource (shared memory segment)
 * unlock_resource (): Releases the resource (shared memory segment)
 * parse_args (): Parse command line arguments
 * setup_signal_handlers (): Setup the signal catching function
 * handler (): Signal catching function
 * child (): Child process
 * sys_error (): System error message function
 * error (): Error message function
 * cleanup (): Releases semaphores & kills child processes
 */
static void create_semaphores ();
static void delete_semaphores ();
static void lock_resource (int);
static void unlock_resource (int);
static void parse_args (int, char **);
static void setup_signal_handlers ();
static void handler (int, int, struct sigcontext *);
static void child (int, unsigned char *);
static void sys_error (const char *, int);
static void error (const char *, int);
static void cleanup ();

/*
 * Global Variables:
 *
 * checksum: Array of checksums computed by child processes
 * parent_pid: Process id of the parent
 * pid: Array of child process id's
 * semid: System wide unique shared memory identifier
 * num_children: number of child processes to spawn
 * buffer_size: size of "scratch" shared memory segment
 * read_count: number of child processes reading data
 */
enum { READ_COUNT, WRITE };		/* semaphore constants */
int	*read_count;

unsigned long *checksum; 	/* shared memory segment address */
pid_t	parent_pid;		/* process id of parent */
pid_t	pid [MAX_CHILDREN];	/* child processes process id's */
int	semid;			/* semaphore id */
int     num_children = DEFAULT_NUM_CHILDREN;
int 	buffer_size = DEFAULT_SHMEM_SIZE;

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
	int	fd;		/* Misc file descriptor  */
	int	i; 		/* Misc loop index */
	int	shmem_size;	/* Size (in bytes) of shared memory segment */
	int	status;		/* Child processes exit status */
	unsigned char *ptr;		/* Misc pointer */
	unsigned char data = 0;	/* Value written into shared memory segment */
	unsigned char *shmptr;	/* Shared memory segment address */
	unsigned long cksum;	/* Shared memory segment checksum */

	/*
	 * Parse command line arguments and print out program header
	 */
	parse_args (argc, argv);
	printf ("%s: IPC Shared Memory TestSuite program\n", *argv);

	/*
	 * Setup the signal handlers (in case user aborts program).
	 *
	 * Create the semaphores to insure exclusive writes to the
	 * shared memory segment.
	 *
	 * Save the parent process id and initialize the array of child
	 * process ids.
	 */
	setup_signal_handlers ();
	create_semaphores ();

	parent_pid = getpid ();
	for (i=0; i<num_children; i++)
		pid [i] = (pid_t)0;

	/*
	 * Create a shared memory segment for storing the read count
	 * (number of child processes reading shared data)
	 * After creating the shared memory segment, initialize it.
	 */
	if ((fd = open ("/dev/zero", O_RDWR)) < 0)
		sys_error ("open failed", __LINE__);
	if ((read_count = (int *)
		mmap (0, sizeof (int), PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, 0)) < 0)
		sys_error ("mmap failed", __LINE__);
	close (fd);
	*read_count = 0;

	/*
	 * Create a shared memory segment for storing the child
	 * processes checksums by memory mapping /dev/zero.
	 * After creating the shared memory segment, initialize it.
	 */
	if ((fd = open ("/dev/zero", O_RDWR)) < 0)
		sys_error ("open failed", __LINE__);
	shmem_size = sizeof (unsigned long) * num_children;
	if ((checksum = (unsigned long *)
		mmap (0, shmem_size, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, 0)) < 0)
		sys_error ("mmap failed", __LINE__);
	close (fd);

	for (i=0; i < num_children; i++)
		*(checksum + (sizeof (unsigned long) * i)) = 0;

	/*
	 * Create the "scratch" shared memory segment for storing
	 * a series of values by memory mapping /dev/zero.
	 */
	if ((fd = open ("/dev/zero", O_RDWR)) < 0)
		sys_error ("open failed", __LINE__);

	printf ("\n\tGet shared memory segment (%d bytes)\n", buffer_size);
	if ((shmptr = mmap (0, buffer_size, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0)) < 0)
		sys_error ("mmap failed", __LINE__);
	close (fd);

	/*
	 * Obtain an exclusive "write" lock on the shared memory data
	 * segment -- get lock now to insure the parent process gets
	 * first access to the segment.
	 */
	lock_resource (WRITE);

	/*
	 * Spawn all N child processes.  Each child process will compute
	 * the checksum of the shared memory segment and will store
	 * the results in the other shared memory segment accessible
	 * by the parent.
	 */
	printf ("\n\tSpawning %d child processes ... \n", num_children);
	for (i=0; i<num_children; i++) {

		if ((pid [i] = fork()) == (pid_t)0) {
			child (i, shmptr);
			exit (0);
		} else if (pid [i] < (pid_t)0)
			sys_error ("fork failed", __LINE__);
	}

	/*
	 * Fill the "scratch" shared memory segment up with data and
	 * compute the segments checksum.  Release "write" lock after
	 * completing so that the child processes may begin to read the
	 * data.
	 */
	printf ("\n\tParent: calculate shared memory segment checksum\n");
	cksum = data = 0;

	for (ptr=shmptr; ptr < (shmptr+buffer_size); ptr++) {
		*ptr = (data++) % (UCHAR_MAX + 1);
		cksum += *ptr;
	}
	printf ("\t        shared memory checksum %08lx\n", cksum);
	unlock_resource (WRITE);

	/*
	 * Wait for the child processes to compute the checksums and complete.
	 * After the processes complete, check their exit status to insure
	 * that they ran to completion and then verify the corresponding
	 * checksum.
	 */
	for (i=0; i<num_children; i++) {
		waitpid (pid [i], &status, 0);

		if (!WIFEXITED (status))
			sys_error ("child process terminated abnormally",
				__LINE__);
		if (cksum != *(checksum + (sizeof (unsigned long) * i))) {
			printf ("checksum [%d] = %08lx\n", i, checksum [i]);
			error ("checksums do not match", __LINE__);
		}
	}
	printf ("\n\tParent: children calculated segment successfully\n");

	/*
	 * Program completed successfully, cleanup semaphores and exit.
	 */
	delete_semaphores ();
	printf ("\nsuccessful!\n");

	return (0);
}


/*---------------------------------------------------------------------+
|                               child ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Waits for read access to the shared memory segment,       |
|            computes the shared memory segments checksum and releases |
|            the read lock.  Then stores the checksum.                 |
|                                                                      |
| Updates:   checksum - shared memory segment containing checksums     |
|                       computed by child processes                    |
|                                                                      |
+---------------------------------------------------------------------*/
static void child (int num, unsigned char *shmptr)
{
	unsigned long cksum = 0;	/* Shared memory regions checksum */
	int	i;			/* Misc index */

	/*
	 * Wait for a READ_COUNT lock on the shared memory segment, then
	 * compute the checksum and release the READ_COUNT lock.     
	 */
	lock_resource (READ_COUNT);
	(*read_count)++;
	if (*read_count == 1)
		lock_resource (WRITE);
	unlock_resource (READ_COUNT);

	for (i=0; i<buffer_size; i++)
		cksum += *shmptr++;

	lock_resource (READ_COUNT);
	(*read_count)--;
	if (*read_count == 0)
		unlock_resource (WRITE);
	unlock_resource (READ_COUNT);

	/*
	 * Store the resulting checksum and print out a message
	 */
	checksum [num] = cksum;
	*(checksum + (sizeof (unsigned long) * num)) = cksum;
	printf ("\t\tchild (%02d): checksum %08lx\n", num,
		*(checksum + (sizeof (unsigned long) * num)));
}


/*---------------------------------------------------------------------+
|                          create_semaphores ()                        |
| ==================================================================== |
|                                                                      |
| Function:  Creates two semaphores:                                   |
|                                                                      |
|            READ_COUNT: shared read semaphore                         |
|            WRITE:      exclusive write semaphore                     |
|                                                                      |
| Updates:   semid - system wide semaphore identifier                  |
|                                                                      |
+---------------------------------------------------------------------*/
static void create_semaphores ()
{
	int	nsems = 2;	/* Number of semaphores to create */

	/*
	 * Create two system unique semaphores.
	 */
	if ((semid = semget (IPC_PRIVATE, nsems, IPC_CREAT | 0666)) < 0)
		sys_error ("semget failed", __LINE__);

	arg.val = 1;
	if (semctl (semid, WRITE, SETVAL, arg) < 0)
		sys_error ("semctl (SETVAL) failed", __LINE__);
	if (semctl (semid, READ_COUNT, SETVAL, arg) < 0)
		sys_error ("semctl (SETVAL) failed", __LINE__);
}


/*---------------------------------------------------------------------+
|                          delete_semaphores ()                        |
| ==================================================================== |
|                                                                      |
| Function:  Deletes the two READ/WRITE semaphores.                    |
|                                                                      |
+---------------------------------------------------------------------*/
static void delete_semaphores ()
{
	int	nsems = 2;

	/*
	 * Delete both READ_COUNT and WRITE semaphores.
	 */

	arg.val = 0;
	if (semctl (semid, nsems, IPC_RMID, arg) < 0)
		sys_error ("semctl (IPC_RMID) failed", __LINE__);
}


/*---------------------------------------------------------------------+
|                          lock_resource ()                            |
| ==================================================================== |
|                                                                      |
| Function:  Obtains a READ/WRITE semaphore lock.                      |
|                                                                      |
+---------------------------------------------------------------------*/
static void lock_resource (int semaphore)
{
	struct sembuf	buf;

	buf.sem_op = -1;		/* Obtain resource */
	buf.sem_num = semaphore;
	buf.sem_flg = 0;

	if (semop (semid, &buf, 1) < 0)
		sys_error ("semop (LOCK) failed", __LINE__);
}


/*---------------------------------------------------------------------+
|                          unlock_resource ()                          |
| ==================================================================== |
|                                                                      |
| Function:  Releases a READ/WRITE semaphore lock.                     |
|                                                                      |
+---------------------------------------------------------------------*/
static void unlock_resource (int semaphore)
{
	struct sembuf	buf;

	buf.sem_op = 1;			/* Release resource */
	buf.sem_num = semaphore;
	buf.sem_flg = 0;

	if (semop (semid, &buf, 1) < 0)
		sys_error ("semop (UNLOCK) failed", __LINE__);
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
|            [-s] size: shared memory segment size                     |
|                                                                      |
|            [-c] num_children: number of child processes              |
|                                                                      |
+---------------------------------------------------------------------*/
void parse_args (int argc, char **argv)
{
	int	i;
	int	errflag = 0;
	char	*program_name = *argv;
	extern char 	*optarg;	/* Command line option */

	while ((i = getopt(argc, argv, "c:s:?")) != EOF) {
		switch (i) {
			case 'c':
				num_children = atoi (optarg);
				break;
			case 's':
				buffer_size = atoi (optarg);
				break;
			case '?':
				errflag++;
				break;
		}
	}
	if (num_children >= MAX_CHILDREN) {
		errflag++;
		fprintf (stderr, "ERROR: num_children must be less than %d\n",
			MAX_CHILDREN);
	}

	if (errflag) {
		fprintf (stderr, USAGE, program_name);
		exit (2);
	}
}


/*---------------------------------------------------------------------+
|                          setup_handler ()                            |
| ==================================================================== |
|                                                                      |
| Function:  Setup the signal handler for SIGINT.                      |
|                                                                      |
+---------------------------------------------------------------------*/
void setup_signal_handlers ()
{
	struct sigaction invec;

	invec.sa_handler = (void (*)(int)) handler;
	sigemptyset (&invec.sa_mask);
	invec.sa_flags = 0;

	if (sigaction (SIGINT, &invec, (struct sigaction *) NULL) < 0)
		sys_error ("sigaction failed", __LINE__);

}


/*---------------------------------------------------------------------+
|                             handler ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Signal catching function for SIGINT signal                |
|                                                                      |
|            o  SIGINT:  Parent process calls cleanup, child processes |
|                        simply exit                                   |
|                                                                      |
|            o  Other:   Print message and abort program...            |
|                                                                      |
+---------------------------------------------------------------------*/
void handler (int sig, int code, struct sigcontext *scp)
{
	char 	msg [100];	/* Buffer for error message */

	if (sig == SIGINT) {
		if (getpid () == parent_pid) {

			fprintf (stderr, "Received SIGINT -- cleaning up...\n");
			fflush (stderr);

			cleanup ();
		}
		else
			exit (-1);
	} else {
		sprintf (msg, "Received an unexpected signal (%d)", sig);
		error (msg, __LINE__);
	}
}


/*---------------------------------------------------------------------+
|                             cleanup ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Closes all of the pipes, kills all of the child           |
|            processes and exits the program...                        |
|                                                                      |
+---------------------------------------------------------------------*/
void cleanup ()
{
	int i;

	if (getpid () == parent_pid) {
		delete_semaphores ();
		for (i=0; i<num_children; i++) {
			if (pid [i] > (pid_t)0 && kill (pid [i], SIGKILL) < 0)
				sys_error ("signal failed", __LINE__);
		}
	}

	exit (-1);
}


/*---------------------------------------------------------------------+
|                             sys_error ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Creates system error message and calls error ()           |
|                                                                      |
+---------------------------------------------------------------------*/
void sys_error (const char *msg, int line)
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
void error (const char *msg, int line)
{
	fprintf (stderr, "ERROR [line: %d] %s\n", line, msg);
	cleanup ();
}
