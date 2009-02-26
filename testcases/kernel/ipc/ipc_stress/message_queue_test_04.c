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
|                        message_queue_test_04                         |
| ==================================================================== |
|                                                                      |
| Description:  Create a message queue to send messages between two    |
|               processes.                                             |
|                                                                      |
| Algorithm:    o  Generate a key value given the project name and id  |
|                                                                      |
|               o  Get a unique message queue id                       |
|                                                                      |
|               o  Print out message queue id                          |
|                                                                      |
| System calls: The following system calls are made                    |
|                                                                      |
|               o  msgget () - gets a message queue identifier         |
|                                                                      |
| Usage:        message_queue_test_04 [-v] [-l logfile ]               |
|                                                                      |
| To compile:   cc -o message_queue_test_04 message_queue_test_04.c    |
|                                                                      |
| Last update:   Ver. 1.2, 2/26/94 14:03:55                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     111993  DJK   Initial version for AIX 4.1                 |
|    1.2     022694  DJK   Moved to Prod directory                     |
|                                                                      |
+---------------------------------------------------------------------*/


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#ifdef _LINUX_
// defines struct msgbuf
#define __USE_GNU
#endif
#include <sys/msg.h>
#include <sys/types.h>
#include <stdlib.h>

/*
 * Defines
 *
 * MAX_MSGS: maximum number of messages per queue (8192)
 */
#define DEFAULT_PROJECT_NAME 	"/tmp/message_queue_test"
#define DEFAULT_PROJECT_ID	20
#define MAX_MSGS		8192
#define USAGE	"\nUsage: %s [-v] [-l logfile ]\n\n"


/*
 * Function prototypes
 *
 * parse_args (): Parse command line arguments
 * sys_error (): System error message function
 * error (): Error message function
 */
static void parse_args (int, char **);
static void sys_error (const char *, int);
static void error (const char *, int);


/*
 * Global variables
 *
 * log_filename: Name of log file
 */
int	verbose	= 0;
int	logit	= 0;
FILE	*logfile;
char	*log_filename = NULL;


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
	struct msqid_ds info;	/* Message queue info */
	struct msgbuf buf;	/* Message queue buffer */
	int	mode = 0777;	/* Default mode bits */
	int	msqid;		/* Message queue identifier */
	size_t 	max_bytes;	/* Num bytes sent to message queue */
	size_t 	msg_size;	/* Num bytes sent to message queue */
	unsigned long	bytes_sent;	/* Num bytes sent to message queue */

	/*
	 * Parse command line options
	 */
	parse_args (argc, argv);
	if (logit) {
		if ((logfile = fopen (log_filename, "w")) == NULL)
			sys_error ("msgget failed", __LINE__);
	}

	/*
	 * Print out program header
	 */
	printf ("%s: IPC Message Queue TestSuite program\n\n", *argv);
	if (logit)
		fprintf (logfile, "%s: IPC Message Queue TestSuite program\n\n", *argv);

	/*
	 * Obtain a unique message queue identifier using msgget()
	 */
	if ((msqid = msgget (IPC_PRIVATE, IPC_CREAT|mode)) < 0)
		sys_error ("msgget failed", __LINE__);

	if (verbose)
		printf ("\tCreated message queue: %d\n\n", msqid);
	if (logit)
		fprintf (logfile, "\tCreated message queue: %d\n\n", msqid);


	/*
	 * Determine message queue limits
	 *
	 * Determine the maximum number of bytes that the message
	 * queue will hold.  Then determine the message size
	 * (Max num of bytes per queue / maximum num of messages per queue)
	 */
	if (msgctl (msqid, IPC_STAT, &info) < 0)
		sys_error ("msgctl (IPC_STAT) failed", __LINE__);

	max_bytes = info.msg_qbytes;

	/*
	 * this has been changed because of defect 227707 related to floating point
	 * problem, but here is not the right place to test floating point...
	 * msg_size  = (size_t) (0.5 + ((float) max_bytes / MAX_MSGS));
	 */
	msg_size  = (size_t)((max_bytes + MAX_MSGS - 1) / MAX_MSGS);

	if (verbose) {
		printf ("\tMax num of bytes per queue:  %ld\n", (long)max_bytes);
		printf ("\tMax messages per queue:      %d\n",  MAX_MSGS);
		printf ("\tCorresponding message size:  %ld\n\n", (long)msg_size);
	}
	if (logit) {
		fprintf (logfile, "\tMax num of bytes per queue:  %ld\n",  (long)max_bytes);
		fprintf (logfile, "\tMax messages per queue:      %d\n",  MAX_MSGS);
		fprintf (logfile, "\tCorresponding message size:  %ld\n\n", (long)msg_size);
	}

	/*
	 * Fill up the message queue
	 *
	 * Send bytes to the message queue until it fills up
	 */
	//	buf = (struct msgbuf *) calloc (msg_size + sizeof(struct msgbuf), sizeof (char));

	buf.mtype = 1L;

	bytes_sent = 0;
	while (bytes_sent < max_bytes - msg_size) {
		if (msgsnd (msqid, &buf, msg_size, 0) < 0)
			sys_error ("msgsnd failed", __LINE__);
		bytes_sent += msg_size;
		//usleep(5000);
		if (verbose) {
			printf ("\r\tBytes sent: %ld", (long)bytes_sent);
			fflush(stdout);
		  }
	}
	if (verbose) puts ("\n");
	if (logit) fprintf (logfile, "\tBytes sent: %ld\n", (long)bytes_sent);
	//free (buf);

	/*
	 * Remove the message queue
	 */
	if (msgctl (msqid, IPC_RMID, 0) < 0)
		sys_error ("msgctl (IPC_RMID) failed", __LINE__);
	if (verbose)
		printf ("\n\tRemoved message queue: %d\n", msqid);
	if (logit)
		fprintf (logfile, "\n\tRemoved message queue: %d\n", msqid);

	/* Program completed successfully -- exit */
	printf ("\nsuccessful!\n");
	if (logit) {
		fprintf (logfile, "\nsuccessful!\n");
		fclose (logfile);
	}


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
|            [-v] verbose                                              |
|                                                                      |
|            [-l] logfile: log file name                               |
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
	while ((opt = getopt(argc, argv, "vl:")) != EOF) {
		switch (opt) {
			case 'v':	/* verbose */
				verbose++;
				break;
			case 'l':	/* log file */
				logit++;
				log_filename = optarg;
				break;
			default:
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
	if (logit)
		fprintf (logfile, "ERROR [line: %d] %s\n", line, msg);
	exit (-1);
}
