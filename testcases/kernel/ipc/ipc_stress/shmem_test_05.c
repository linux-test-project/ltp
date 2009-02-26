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
|                           shmem_test_05.c                            |
| ==================================================================== |
|                                                                      |
| Title:        SHMBLA is treated as 0x1000                            |
|                                                                      |
| Purpose:      simultaneous attachment of more than ten shared        |
|               memory regions to a process at adresses that are       |
|		multiple of the value SHMBLA . EXTSHM=ON is set ,      |
|               the value of SHMLBA is treated as 0x1000 internally    |
|                                                                      |
| Description:  Simplistic test to verify that a process can obtain    |
|               11 shared memory regions in the adress space from      |
|               0x30000000 to 0x3FFFFFFF .                             |
|                                                                      |
|                                                                      |
| Algorithm:    *  from 1 up to 11                                     |
|               {                                                      |
|               o  Create a key to uniquely identify the shared segment|
|		   using ftok()	subroutine.			       |
|               o  Obtain a unique shared memory identifier with       |
|                  shmget ()                                           |
|               o  Map the shared memory segment to the current        |
|                  process with shmat ()                               |
|               o  Index through the shared memory segment             |
|               }                                                      |
|               *  from 1 up to 11                                     |
|               {                                                      |
|               o  Detach from the segment with shmdt()                |
|               o  Release the shared memory segment with shmctl ()    |
|               }                                                      |
|                                                                      |
| System calls: The following system calls are tested:                 |
|                                                                      |
|               ftok()                                                 |
|               shmget ()                                              |
|               shmat ()                                               |
|               shmdt ()                                               |
|               shmctl ()                                              |
|                                                                      |
| Usage:        shmem_test_05                                          |
|                                                                      |
| To compile:   cc -O -g  shmem_test_05.c -o shmem_test_05 -lbsd       |
|                                                                      |
| Last update:                                                         |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     010997  J.Malcles  initial version for 4.2.G              |
|                                                                      |
|                                                                      |
+---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/shm.h>

#include <sys/types.h>
#include <sys/ipc.h>

#include <sys/stat.h>
off_t offsets[] = {
0x30000000,
0x30200000,
0x30400000,
0x30600000,
0x30800000,
0x30A00000,
0x30C00000,
0x30D00000,
0x30F00000,
0x31000000,
0x32000000
};
int offsets_cnt = sizeof (offsets) /sizeof (offsets[0]);
/* Defines
 *
 * MAX_SHMEM_SIZE: maximum shared memory segment size of 256MB
 *
 * MAX_SHMEM_NUMBER: maximum number of simultaneous attached shared memory
 * regions
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
#define MAX_SHMEM_NUMBER	11
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
	off_t offset;
	int i;

	int shmid[MAX_SHMEM_NUMBER];	/* (Unique) Shared memory identifier */

	char *shmptr[MAX_SHMEM_NUMBER];	/* Shared memory segment address */
	char	*ptr;		/* Index into shared memory segment */

	int value = 0;	/* Value written into shared memory segment */

	key_t mykey[MAX_SHMEM_NUMBER];
	char * null_file = "/dev/null";
	char  proj[MAX_SHMEM_NUMBER] = {
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'A',
		'B',
		'C',
		'E'
	};


	/*
	 * Parse command line arguments and print out program header
	 */
	parse_args (argc, argv);
	printf ("%s: IPC Shared Memory TestSuite program\n", *argv);
   
	for (i=0; i<offsets_cnt; i++) {
        /*
         * Create a key to uniquely identify the shared segment
         */

	mykey[i] = ftok(null_file,proj[i]);

	printf ("\n\tmykey to uniquely identify the shared memory segment 0x%x\n",mykey[i]);


	printf ("\n\tGet shared memory segment (%d bytes)\n", shmem_size);
        /*
         * Obtain a unique shared memory identifier with shmget ().
         */

/*	if ((shmid[i]= shmget(mykey[i], shmem_size, IPC_CREAT | 0666 )) < 0) */
	if ((shmid[i]= shmget (IPC_PRIVATE, shmem_size,
				IPC_CREAT | IPC_EXCL | 0666 )) < 0)
		sys_error ("shmget failed", __LINE__);


	printf ("\n\tAttach shared memory segment to process\n");
        /*
         * Attach the shared memory segment to the process
         */

                offset = offsets[i];

#ifndef __64BIT__
         printf ("\n\toffset of the shared memory segment 0x%lx\n",offset);
        if ((shmptr[i] = (char *) shmat (shmid[i], (const void *)offset, 0)) == (char *)-1)
#else
         printf ("\n\toffset of the shared memory is determined by the system\n"
);
         if ((shmptr[i] = (char *) shmat (shmid[i], 0 , 0)) == (char *)-1)
#endif
                sys_error ("shmat failed", __LINE__);



	printf ("\n\tIndex through shared memory segment ...\n");

        /*
         * Index through the shared memory segment
         */

	for (ptr=shmptr[i]; ptr < (shmptr[i] + shmem_size); ptr++)
		*ptr = value++;

	}

	printf ("\n\tDetach from the segment using the shmdt subroutine\n");

	printf ("\n\tRelease shared memory\n");

	for (i=0; i<offsets_cnt; i++) {
        /*
         * Detach from the segment
         */

	shmdt( shmptr[i] );

        /*
         * Release shared memory
         */


	if (shmctl (shmid[i], IPC_RMID, 0) < 0)
		sys_error ("shmctl failed", __LINE__);

	}
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
