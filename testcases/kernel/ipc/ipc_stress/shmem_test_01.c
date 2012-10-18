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
|                           shmem_test_01                              |
| ==================================================================== |
|                                                                      |
| Description:  Simplistic test to verify the shmem system function    |
|               calls.                                                 |
|                                                                      |
|                                                                      |
| Algorithm:    o  Obtain a unique shared memory identifier with       |
|                  shmget ()                                           |
|               o  Map the shared memory segment to the current        |
|                  process with shmat ()                               |
|               o  Index through the shared memory segment             |
|               o  Release the shared memory segment with shmctl ()    |
|                                                                      |
| System calls: The following system calls are tested:                 |
|                                                                      |
|               shmget () - Gets shared memory segments                |
|               shmat () - Controls shared memory operations           |
|               shmctl () - Attaches a shared memory segment or mapped |
|                           file to the current process                |
|                                                                      |
| Usage:        shmem_test_01                                          |
|                                                                      |
| To compile:   cc -o shmem_test_01 shmem_test_01.c                    |
|                                                                      |
| Last update:   Ver. 1.2, 2/8/94 00:08:30                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     111593  DJK   Initial version for AIX 4.1                 |
|    1.2     020794  DJK   Moved to "prod" directory                   |
|                                                                      |
+---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/shm.h>

/* Defines
 *
 * MAX_SHMEM_SIZE: maximum shared memory segment size of 256MB
 * (reference 3.2.5 man pages)
 *
 * DEFAULT_SHMEM_SIZE: default shared memory size, unless specified with
 * -s command line option
 *
 * SHMEM_MODE: shared memory access permissions (permit process to read
 * and write access)
 *
 * USAGE: usage statement
 */
#define MAX_SHMEM_SIZE		256*1024*1024
#define DEFAULT_SHMEM_SIZE	1024*1024
#define	SHMEM_MODE		(SHM_R | SHM_W)
#define USAGE	"\nUsage: %s [-s shmem_size]\n\n" \
		"\t-s shmem_size  size of shared memory segment (bytes)\n" \
		"\t               (must be less than 256MB!)\n\n"

/*
 * Function prototypes
 *
 * parse_args (): Parse command line arguments
 * sys_error (): System error message function
 * error (): Error message function
 */
void parse_args (int, char **);
void sys_error (const char *, int);
void error (const char *, int);

/*
 * Global variables
 *
 * shmem_size: shared memory segment size (in bytes)
 */
int shmem_size = DEFAULT_SHMEM_SIZE;

/*---------------------------------------------------------------------+
|                               main                                   |
| ==================================================================== |
|                                                                      |
|                                                                      |
| Function:  Main program  (see prolog for more details)               |
|                                                                      |
| Returns:   (0)  Successful completion                                |
|            (-1) Error occurred                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int main (int argc, char **argv)
{
	int	shmid;		/* (Unique) Shared memory identifier */
	char	*shmptr,	/* Shared memory segment address */
		*ptr,		/* Index into shared memory segment */
		value = 0;	/* Value written into shared memory segment */

	/*
	 * Parse command line arguments and print out program header
	 */
	parse_args (argc, argv);
	printf ("%s: IPC Shared Memory TestSuite program\n", *argv);

	/*
	 * Obtain a unique shared memory identifier with shmget ().
	 * Attach the shared memory segment to the process with shmat (),
	 * index through the shared memory segment, and then release the
	 * shared memory segment with shmctl ().
	 */
	printf ("\n\tGet shared memory segment (%d bytes)\n", shmem_size);
	if ((shmid = shmget (IPC_PRIVATE, shmem_size, SHMEM_MODE)) < 0)
		sys_error ("shmget failed", __LINE__);

	printf ("\n\tAttach shared memory segment to process\n");
	if ((shmptr = shmat (shmid, 0, 0)) < 0)
		sys_error ("shmat failed", __LINE__);

	printf ("\n\tIndex through shared memory segment ...\n");
	for (ptr=shmptr; ptr < (shmptr + shmem_size); ptr++)
		*ptr = value++;

	printf ("\n\tRelease shared memory\n");
	if (shmctl (shmid, IPC_RMID, 0) < 0)
		sys_error ("shmctl failed", __LINE__);

	/*
	 * Program completed successfully -- exit
	 */
	printf ("\nsuccessful!\n");

	return (0);
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
+---------------------------------------------------------------------*/
void parse_args (int argc, char **argv)
{
	int	i;
	int	errflag = 0;
	char	*program_name = *argv;
	extern char 	*optarg;	/* Command line option */

	while ((i = getopt(argc, argv, "s:?")) != EOF) {
		switch (i) {
			case 's':
				shmem_size = atoi (optarg);
				break;
			case '?':
				errflag++;
				break;
		}
	}

	if (shmem_size < 1 || shmem_size > MAX_SHMEM_SIZE)
		errflag++;

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
	exit (-1);
}
