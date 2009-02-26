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
|                       message_queue_test_01                          |
| ==================================================================== |
|                                                                      |
| Description:  Create a message queue to send messages between two    |
|               processes.                                             |
|                                                                      |
| Algorithm:    o  Parent                                              |
|                  - Create a pipe                                     |
|                  - Spawn a child process                             |
|                  - Create unique message queue identifier and send   |
|                    it to child process via pipe                      |
|                                                                      |
|               o  Child                                               |
|                  - Wait for message queue identifier from parent     |
|                  - Send data to parent via message queue and exit    |
|                                                                      |
|               o  Parent                                              |
|                  - Receive message from child process                |
|                  - Remove message queue                              |
|                  - Wait for child process to exit                    |
|                                                                      |
| System calls: The following system calls are made                    |
|                                                                      |
|               msgget() - Gets a message queue identifier             |
|               msgrcv() - Reads a message from a queue                |
|               msgctl() - Provides message control operations         |
|                                                                      |
| Usage:        message_queue_test_01                                  |
|                                                                      |
| To compile:   cc -o message_queue_test_01 message_queue_test_01.     |
|                                                                      |
| Last update:   Ver. 1.2, 2/26/94 14:03:30                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     050689  CTU   Initial version                             |
|    1.2     111993  DJK   Modify for AIX version 4.1                  |
|    1.3     022693  DJK   Move to Prod directory                      |
|                                                                      |
+---------------------------------------------------------------------*/


#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#ifdef _LINUX_
// defines struct msgbuf
#define __USE_GNU
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

#define key_tt int
#define	FIRST_MSG	0
#define BUF_SIZE        256

#define SAFE_FREE(p) { if(p) { free(p); (p)=NULL; } }

/*
 * Function prototypes
 *
 * sys_error (): System error message function
 * error (): Error message function
 */
static void sys_error (const char *, int);
static void error (const char *, int);
static void child (int []);
enum { READ, WRITE };


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
	key_tt	msqid;			/* message queue id */
	struct msgbuf	*buf;
	pid_t	pid;
	int	mode = 0666;
	int	fd [2];

        /*
         * Print out program header
         */
        printf ("%s: IPC Message Queue TestSuite program\n", *argv);
	fflush (stdout);
	pipe (fd);

	if ((pid = fork()) == 0) {
		child (fd);
		exit (0);
	} else if (pid < (pid_t)0)
		sys_error ("fork failed", __LINE__);

	/*
	 * Create a NEW unique message queue identifier.
	 */
	if ((msqid = msgget (IPC_PRIVATE,  mode|IPC_CREAT|IPC_EXCL)) < 0)
		sys_error ("msgget failed", __LINE__);
	printf ("\n\tCreate message queue, id: 0x%8.8x\n", msqid);
	fflush (stdout);

	/*
	 * Send the newly created message queue identifier to the child
	 * process via pipes.  Close pipes after sending identifier.
	 */
	close (fd [READ]);
	if (write (fd [WRITE], &msqid, sizeof (key_tt)) < 0)
		sys_error ("write failed", __LINE__);
	close (fd [WRITE]);

	/*
	 * Read the data from the message queue
	 */
	buf = (struct msgbuf *)calloc ((size_t)(sizeof(struct msgbuf) + BUF_SIZE),
		sizeof (char));
	if(!buf)
		sys_error ("calloc failed", __LINE__);

	if (msgrcv (msqid, (void *)buf, (size_t)BUF_SIZE, FIRST_MSG, 0) < 0) {
		SAFE_FREE(buf);
		sys_error ("msgsnd failed", __LINE__);
	}
	printf ("\n\tParent: received message: %s\n", buf->mtext);
	fflush (stdout);
	SAFE_FREE(buf);

	/*
	 * Remove the message queue from the system
	 */
	printf ("\n\tRemove the message queue\n");
	fflush (stdout);
	if (msgctl (msqid, IPC_RMID, 0) < 0)
		sys_error ("msgctl (IPC_RMID) failed", __LINE__);

        printf ("\nsuccessful!\n");
	fflush (stdout);
	return (0);
}


/*---------------------------------------------------------------------+
|                               child                                  |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
static void child (int fd[])
{
	key_tt	msqid;			/* message queue id */
	size_t	nbytes;
	struct msgbuf	*buf;

	/*
	 * Read the message queue identifier from the parent through
	 * the pipe.  Close pipe after reading identifier.
	 */
	close (fd [WRITE]);
	if (read (fd [READ], &msqid, sizeof (key_tt)) < 0)
		sys_error ("read failed", __LINE__);
	close (fd [READ]);
        printf ("\n\tChild:  received message queue id: %d\n", msqid);
	fflush (stdout);

	/*
	 * Put data on the message queue
	 */
	buf = (struct msgbuf *)calloc ((size_t)(sizeof(struct msgbuf) + BUF_SIZE),
		sizeof (char));
	if(!buf)
		sys_error ("calloc failed", __LINE__);

	buf->mtype = 1;
	sprintf (buf->mtext, "\"message queue transmission test....\"");
	nbytes =  strlen (buf->mtext) + 1;

	printf ("\n\tChild:  sending message:  %s\n", buf->mtext);
	fflush (stdout);
	if (msgsnd (msqid, buf, nbytes, 0) < 0) {
		SAFE_FREE(buf);
		sys_error ("msgsnd failed", __LINE__);
	}
	SAFE_FREE(buf);
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
