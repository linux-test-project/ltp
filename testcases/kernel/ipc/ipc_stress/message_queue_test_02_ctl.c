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
|                     message_queue_test_02_ctl                        |
| ==================================================================== |
|                                                                      |
| Description:  Verify and delete message queues                       |
|                                                                      |
| Algorithm:    o  Removes message queues with msgctl (IPC_RMID) if    |
|                  -r option was specified                             |
|                                                                      |
|               o  Prints out message queue stats with                 |
|                  msgctl (IPC_STAT) if -s option was specified        |
|                                                                      |
| System calls: The following system calls are made                    |
|                                                                      |
|               o  ftok () - generates a standard IPC key              |
|               o  msgctl () - provides message control operations     |
|               o  msgget () - gets a message queue identifier         |
|                                                                      |
| Usage:        message_queue_test_02_ctl [-f file] [-i id] {-r | -s}  |
|                                                                      |
| To compile:   cc -o message_queue_test_02_ctl \                      |
|                     message_queue_test_02_ctl.c                      |
|                                                                      |
| Last update:   Ver. 1.2, 2/26/94 14:03:40                           |
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
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#ifdef _LINUX_
// defines struct msgbuf
#define __USE_GNU
#endif
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

/*
 * Defines
 *
 * BUF_SIZE: size of message buffer...
 */
#define DEFAULT_PROJECT_NAME 	"/tmp/message_queue_test"
#define DEFAULT_PROJECT_ID	20
#define USAGE	"\nUsage: %s [-f project_name ] [-i project_id ] {-r | -s}\n\n"

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
 * project_name: Unique path used to create key (ftok)
 * project_id:   Unique number used to create key (ftok)
 */
char	*project_name = DEFAULT_PROJECT_NAME;
char	project_id = DEFAULT_PROJECT_ID;
int	rflg = 0;
int	sflg = 0;

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
	key_t	key;			/* Unique key */
	int	msqid;			/* Message queue identifier */
	struct msqid_ds buf;		/* Message queue info */

	/*
	 * Parse command line options
	 */
	parse_args (argc, argv);

	if ((key = ftok (project_name, project_id)) < 0)
		sys_error ("ftok failed", __LINE__);

	if ((msqid = msgget (key, S_IRUSR|S_IWUSR)) < 0)
		sys_error ("msgget failed", __LINE__);

	if (sflg) {
		if (msgctl (msqid, IPC_STAT, &buf) < 0)
			sys_error ("msgctl (IPC_STAT) failed", __LINE__);

		printf ("uid	%d\n",  buf.msg_perm.uid);
		printf ("gid	%d\n",  buf.msg_perm.gid);
		printf ("cuid	%d\n",  buf.msg_perm.cuid);
		printf ("cgid	%d\n",  buf.msg_perm.cgid);
		printf ("mode	%d\n",  buf.msg_perm.mode);
#ifdef _LINUX_
		printf ("seq	%ld\n",  (long int)buf.msg_perm.__seq);
		printf ("key	0x%x\n",  buf.msg_perm.__key);
#else
		printf ("seq	%d\n",  buf.msg_perm.seq);
		printf ("key	%\n",  buf.msg_perm.key);
#endif
		printf ("cbytes	%ld\n",  buf.msg_cbytes);
		printf ("qnum	%ld\n",  buf.msg_qnum);
		printf ("qbytes	%ld\n",  buf.msg_qbytes);
		printf ("lspid	%d\n",  buf.msg_lspid);
		printf ("lrpid	%d\n",  buf.msg_lrpid);
		printf ("stime	%ld\n", buf.msg_stime);
		printf ("rtime	%ld\n", buf.msg_rtime);
		printf ("rtime	%ld\n", buf.msg_ctime);
	}
	if (rflg) {
		if (msgctl (msqid, IPC_RMID, 0) < 0)
			sys_error ("msgctl (IPC_RMID) failed", __LINE__);
	}
	exit(0);
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
|            [-s]        print out message queue info                  |
|                                                                      |
|            [-r]        remove message queue                          |
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
	while ((opt = getopt(argc, argv, "rsf:i:")) != EOF) {
		switch (opt) {
			case 's':	/* queue status */
				sflg++;
				break;
			case 'r':	/* remove queue */
				rflg++;
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
	/*
	 * Can only specifiy one or the other
	 */
	if ((!sflg && !rflg) || (sflg && rflg))
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