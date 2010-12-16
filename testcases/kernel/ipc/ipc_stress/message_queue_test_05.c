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
|                       message_queue_test_05                          |
| ==================================================================== |
|                                                                      |
| Description:  Creates N message queues.  Creates three and deletes   |
|               one until N is reached.                                |
|                                                                      |
| Algorithm:    o  Loop until N message queues have been created       |
|                  - Create three message queues                       |
|                  - Delete one message queue                          |
|                                                                      |
|               o  Delete all of the message queues                    |
|                                                                      |
| System calls: The following system calls are made                    |
|                                                                      |
|               o  msgctl () - provides message control operations     |
|               o  msgget () - gets a message queue identifier         |
|                                                                      |
| Usage:        message_queue_test_05 [-n num_queues] [-d]             |
|                                                                      |
|               where: [-n] number of message queues to create         |
|                      [-d] debug option                               |
|                                                                      |
| To compile:   cc -o message_queue_test_05 message_queue_test_05.     |
|                                                                      |
| Last update:   Ver. 1.2, 2/26/94 14:10:17                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     032789  CTU   Initial version                             |
|    1.2     111993  DJK   Modify for AIX version 4.1                  |
|    1.3     022694  DJK   Moved to Prod directory                     |
|                                                                      |
+---------------------------------------------------------------------*/

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#ifdef _LINUX_
// defines struct msgbuf
#define __USE_GNU
#endif
#include <sys/msg.h>
#include <sys/types.h>

/*
 * Defines
 *
 * MAX_MESSAGE_QUEUES: maximum number of message queues to create
 *
 * DEFAULT_MESSAGE_QUEUES: default number of message queues to create
 * unless specified with [-n] command line option
 *
 * USAGE: usage statement macro
 */

#ifdef _LINUX_
#define	MAX_MESSAGE_QUEUES 128
#define	DEFAULT_MESSAGE_QUEUES 10
#else
#define	MAX_MESSAGE_QUEUES 4096
#define	DEFAULT_MESSAGE_QUEUES 1000
#endif

#define USAGE	"\nUsage: %s [-n num_message_queues] [-d]\n\n"

/*
 * Function prototypes
 *
 * parse_args (): Parse command line arguments
 * sys_error (): System error message function
 * error (): Error message function
 * cleanup (): Cleanup function for the test
 */
static void parse_args (int, char **);
static void sys_error (const char *, int);
static void error (const char *, int);
static void cleanup (int qnum);
/*
 * Global variables
 *
 * max_queues: maximum number of message queues allowed (system limit)
 * msqid_array: array containing the unique message queue identifiers
 * debug: debugging flag, set with [-d] command line option
 */
int	max_queues = DEFAULT_MESSAGE_QUEUES;
int	msqid_array [MAX_MESSAGE_QUEUES];
int	debug = 0;

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
	int	nqueues = 0;	/* Current number of message queues */
	int	i;		/* Loop index */
	int	mode = 0777;	/* Misc mode bits */

	/*
	 * Parse command line arguments and print out program header
	 */
	parse_args (argc, argv);
	printf ("%s: IPC Message Queue TestSuite program\n", *argv);

	printf ("\n\tCreating %d message queues ...\n", max_queues);
	while (nqueues < max_queues) {

		for (i=0; i<3; i++) {
			if (debug) printf ("\tcreating queue [%d]\n", nqueues);
			if ((msqid_array [nqueues++]
				= msgget (IPC_PRIVATE, IPC_CREAT|mode)) < 0)
			{
				cleanup(nqueues);
				sys_error ("msgget failed", __LINE__);
			}

			if (nqueues > MAX_MESSAGE_QUEUES)
				break;
		}

		/*
		 * Delete the last message queue...
		 */
		if (msgctl (msqid_array [--nqueues], IPC_RMID, 0) < 0)
			sys_error ("msgctl (IPC_RMID) failed", __LINE__);
		if (debug) printf ("\tremoved queue  [%d]\n", nqueues);
	}
	printf ("\n\tAll message queues created successfully\n");

        cleanup(nqueues);
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
|            [-n] num:   number of message queues to create            |
|                                                                      |
+---------------------------------------------------------------------*/
static void parse_args (int argc, char **argv)
{
	int	opt;
	int	errflag = 0;
	char	*program_name = *argv;
	extern char 	*optarg;	/* Command line option */

	/*
	 * Parse command line options.
	 */
	while ((opt = getopt(argc, argv, "n:d")) != EOF) {
		switch (opt) {
			case 'n':	/* number of queues to create */
				max_queues = atoi (optarg);
				break;
			case 'd':	/* debug */
				debug++;
				break;
			default:
				errflag++;
				break;
		}
	}
	if (max_queues > MAX_MESSAGE_QUEUES)
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

/*---------------------------------------------------------------------+
|                              cleanup()                               |
| ==================================================================== |
| cleanup() - performs all message queues cleanup for this test at     |
|             completion or premature exit.                            |
|             Remove the temporary message queues created.             |
|                                                                      |
+---------------------------------------------------------------------*/
void cleanup(int qnum)
{
        /*
         *  Remove the allocated message queues.
         */
        while (qnum > 0) {
		if (msqid_array [--qnum] < 0)
			continue;
                if (msgctl (msqid_array [qnum], IPC_RMID, 0) < 0)
                        sys_error ("msgctl (IPC_RMID) failed", __LINE__);
                if (debug) printf ("\tremoved queue  [%d]\n", qnum);
        }
}