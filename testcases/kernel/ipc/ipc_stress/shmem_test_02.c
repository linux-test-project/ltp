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
|                           shmem_test_02                              |
| ==================================================================== |
|                                                                      |
| Description:  Verify anonymous shared memory mapping, with exclusive |
|               writes and shared reads                                |
|                                                                      |
| Algorithm:    o  Obtain two shared memory segments using             |
|                  mmap (MAP_ANON), one for random data created by the |
|                  the parent, and another for the childs checksums    |
|               o  Spawn N child processes                             |
|               o  Parent:                                             |
|                     - obtain write lock on data                      |
|                     - fill shared memory segment with data           |
|                     - compute data checksum                          |
|                     - release lock                                   |
|               o  Child:                                              |
|                     - obtain read lock on data                       |
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
|                                                                      |
| Usage:        shmem_test_02 [-c num_children] [-s shmem_size]        |
|                                                                      |
| To compile:   cc -o shmem_test_02 shmem_test_02.c -lbsd              |
|                                                                      |
|**********************************************************************|
|******!!!!!! LINUX PORT - must be using kernel >= 2.4.3  !!!!!!*******|
|**********************************************************************|
|                                                                      |
| Last update:   Ver. 1.2, 2/8/94 00:08:39                             |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     111593  DJK   Initial version for AIX 4.1                 |
|    1.2     020794  DJK   Moved to "prod" directory                   |
|    1.3     060601  VHM   "ported" to linux                           |
|                                                                      |
+---------------------------------------------------------------------*/

#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>

/* Defines
 *
 * USAGE: usage statement
 */
#define LOCK_FILE "lockfile"
#define NLOOPS		20
#define	SHMEM_MODE	(SHM_R | SHM_W)
#define USAGE	"\nUsage: %s [-c num_children] [-s shmem_size]\n\n" \
		"\t-c num_children   number of child processes to spawn\n" \
		"\t-s shmem_size     size of shared memory segment (bytes)\n" \
		"\t                  (must be less than 256MB!)\n\n"

#define GOTHERE printf("got to line %d\n", __LINE__);

/*
 * Function prototypes
 *
 * parse_args (): Parse command line arguments
 * sys_error (): System error message function
 * error (): Error message function
 */

struct data {
	int	who;
	int	value;
};

void cleanup ();
void handler (int, int, struct sigcontext *);
void setup_signal_handlers ();
static void child (int, unsigned char *);
static int create_lock_file (char *);
static void unlock_file (int);
static void write_lock (int);
static void read_lock (int);
void parse_args (int, char **);
void sys_error (const char *, int);
void error (const char *, int);
void loop (int, char);
void tell (int, char *);

enum { READ, WRITE };		/* Pipe read & write end indices */
enum { PARENT, CHILD };		/* Pipe read & write end indices */

#define MAX_CHILDREN		400
#define BUFFER_SIZE		50
#define DEFAULT_NUM_CHILDREN	2
#define DEFAULT_SHMEM_SIZE	100000

int     num_children = DEFAULT_NUM_CHILDREN;
int 	buffer_size = DEFAULT_SHMEM_SIZE;

unsigned long *checksum; 	/* Shared memory segment address */
int	lockfd;
pid_t	parent_pid;
pid_t	pid [MAX_CHILDREN];

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
	unsigned char	*shmptr,	/* Shared memory segment address */
		value = 0;	/* Value written into shared memory segment */
	int	i;
	unsigned char 	*ptr;
	int	status;
	int	shmem_size;
	unsigned long cksum;

	lockfd = create_lock_file (LOCK_FILE);
	setup_signal_handlers ();

	/*
	 * Parse command line arguments and print out program header
	 */
	parse_args (argc, argv);
	printf ("%s: IPC Shared Memory TestSuite program\n", *argv);

	parent_pid = getpid ();
	for (i=0; i<num_children; i++)
		pid [i] = (pid_t)0;

	/*
	 * Get chunk of shared memory for storing num_children checksum
	 */
	shmem_size = sizeof (unsigned long) * num_children;
        if ((checksum = (unsigned long *)
		mmap (0, shmem_size, PROT_READ | PROT_WRITE,
		      MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED )
		sys_error ("mmap failed", __LINE__);

	for (i=0; i < num_children; i++)
		*(checksum + (sizeof (unsigned long) * i)) = 0;

	/*
	 * Get chunk of memory for writing scratch data
	 */
	printf ("\n\tGet shared memory segment (%d bytes)\n", buffer_size);
	if ((shmptr = mmap (0, buffer_size, PROT_READ | PROT_WRITE,
		MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED)
		sys_error ("mmap failed", __LINE__);

	/*
	 * Parent:
	 *
	 * Fill buffer with data..
	 */
	cksum = value = 0;

	printf ("\n\tParent: calculate shared memory segment checksum\n");
	write_lock (lockfd);
	for (ptr=shmptr; ptr < (shmptr+buffer_size); ptr++) {
                *ptr = (value++) % (UCHAR_MAX + 1);
		cksum += *ptr;
	}
	unlock_file (lockfd);
	printf ("\t        shared memory checksum %08lx\n", cksum);

	printf ("\n\tSpawning %d child processes ... \n", num_children);
	for (i=0; i<num_children; i++) {

		if ((pid [i] = fork()) == (pid_t)0) {
			child (i, shmptr);
			exit (0);
		} else if (pid [i] < (pid_t)0)
			sys_error ("fork failed", __LINE__);
	}

	/*
	 * Wait for child processes to compute checksum and complete...
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
	 * Program completed successfully -- exit
	 */
	printf ("\nsuccessful!\n");

	return (0);
}

static void child (int num, unsigned char *shmptr)
{
	unsigned long cksum = 0;
	int	i;

	read_lock (lockfd);
	for (i=0; i<buffer_size; i++)
		cksum += *shmptr++;
	unlock_file (lockfd);

	*(checksum + (sizeof (unsigned long) * num)) = cksum;
	printf ("\t\tchild (%02d): checksum %08lx\n", num,
		*(checksum + (sizeof (unsigned long) * num)));
}

static void write_lock (int fd)
{
	if (lockf (fd, F_LOCK, 0) < 0)
		sys_error ("lockf (LOCK) failed", __LINE__);
}

static void read_lock (int fd)
{
	if (lockf (fd, F_TEST, 0) < 0)
		sys_error ("lockf (LOCK) failed", __LINE__);
}

static void unlock_file (int fd)
{
	if (lockf (fd, F_ULOCK, 0) < 0)
		sys_error ("lockf (UNLOCK) failed", __LINE__);
}

/*---------------------------------------------------------------------+
|                          setup_handler ()                            |
| ==================================================================== |
|                                                                      |
| Function:  Setup the signal handler for SIGPIPE.                     |
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
| Function:  Signal catching function for SIGPIPE signal.              |
|                                                                      |
|            o  SIGPIPE: Print message and abort program...            |
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
		for (i=0; i<num_children; i++) {
			if (pid [i] > (pid_t)0 && kill (pid [i], SIGKILL) < 0)
				sys_error ("signal failed", __LINE__);
		}
	}

	exit (-1);
}

static int create_lock_file (char *lock_name)
{
	int	fd;

        if ((fd = open (lock_name, O_RDWR)) < 0) {
		if ((fd = open (lock_name, O_RDWR|O_CREAT|O_EXCL, 0666)) < 0) {
			perror ("cannot create lock file");
			exit (-1);
		}
        }
	return (fd);
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

	if (errflag) {
		fprintf (stderr, USAGE, program_name);
		exit (2);
	}
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