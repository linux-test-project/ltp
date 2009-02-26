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
|                     message_queue_test_02_snd                        |
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
|               o  ftok () - generates a standard IPC key              |
|               o  msgget () - gets a message queue identifier         |
|                                                                      |
| Usage:        message_queue_test_02_snd [-f file] [-i id]            |
|                                         [-m msg | -b bytes]          |
|                                                                      |
| To compile:   cc -o message_queue_test_02_snd \                      |
|                     message_queue_test_02_snd.c                      |
|                                                                      |
| Last update:   Ver. 1.2, 2/26/94 14:03:51                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     032789  CST   Initial version                             |
|    1.2     111993  DJK   Modify for AIX version 4.1                  |
|    1.3     022694  DJK   Moved to Prod directory                     |
|                                                                      |
+---------------------------------------------------------------------*/


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#ifdef _LINUX_
# include <sys/stat.h>
// defines struct msgbuf
# define __USE_GNU
#endif
#include <sys/msg.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

/*
 * Defines
 *
 * BUF_SIZE: size of message buffer...
 */
#define BUF_SIZE		256
#define DEFAULT_MESSAGE 	"<< Message Queue test default message >>"
//#define DEFAULT_MESSAGE 	""
#define DEFAULT_PROJECT_NAME 	"/tmp/message_queue_test"
#define DEFAULT_PROJECT_ID	20
#define USAGE	"\nUsage: %s [-f project_name ] [-i project_id ] \n" \
	        "          {-m msg | -b bytes} \n\n"


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
 * message:	 Message to store in queue
 * project_name: Unique path used to create key (ftok)
 * project_id:   Unique number used to create key (ftok)
 */
size_t	bytes = 0;
char	*message = DEFAULT_MESSAGE;
char	*project_name = DEFAULT_PROJECT_NAME;
char	project_id = DEFAULT_PROJECT_ID;

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
	struct msgbuf *buf;		/* Message buffer */
	key_t	key;			/* Unique key */
	int	msqid;			/* Message queue identifier */
	size_t	size;			/* Size of message buffer */

	/*
	 * Parse command line options
	 */
	parse_args (argc, argv);

	if ((key = ftok (project_name, project_id)) < 0)
		sys_error ("ftok failed", __LINE__);

	if ((msqid = msgget (key, S_IRUSR|S_IWUSR)) < 0)
		sys_error ("msgget failed", __LINE__);

	if (bytes) {
		buf = (struct msgbuf *) calloc (bytes + sizeof(struct msgbuf),
			sizeof (char));
		buf->mtype = 1L;

		if (msgsnd (msqid, buf, bytes, 0) < 0)
			sys_error ("msgsnd failed", __LINE__);

		free (buf);

	} else {
		size = strlen (message) + 1;
		buf = (struct msgbuf *) calloc (size + sizeof(struct msgbuf),
			sizeof (char));

		buf->mtype = 1L;
		strcpy (buf->mtext, message);

		if (msgsnd (msqid, buf, size, 0) < 0)
			sys_error ("msgsnd failed", __LINE__);

		free (buf);
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
|            [-f] file:  project file                                  |
|                                                                      |
|            [-i] num:   project id                                    |
|                                                                      |
|            [-m] msg:   message to transmit                           |
|                                                                      |
|            [-b] byes:  number of bytes to send                       |
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
	while ((opt = getopt(argc, argv, "b:m:f:i:")) != EOF) {
		switch (opt) {
			case 'b':	/* number of bytes to send */
				bytes = atoi (optarg);
				break;
			case 'm':	/* message to send */
				message = optarg;
				break;
			case 'f':	/* project file */
				project_name = optarg;
				break;
			case 'i':	/* project id */
				project_id = *optarg;
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
	exit (-1);
}
