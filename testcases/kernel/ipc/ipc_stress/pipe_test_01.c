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
|                            pipe_test_01                              |
| ==================================================================== |
|                                                                      |
| Description:  Simplistic test to verify the pipe system function     |
|               calls                                                  |
|                                                                      |
| Algorithm:    o  Create a pipe                                       |
|               o  Spawn a child process & exec cat on a file.         |
|                  Redirect the command output to the write end of the |
|                  pipe.                                               |
|               o  Spawn another child, redirect the read end of the   |
|                  pipe to stdin & exec wc.                            |
|                                                                      |
|               Equivalent to following shell command:                 |
|                  cat /etc/rc | wc -c                                 |
|                                                                      |
| System calls: The following system calls are tested:                 |
|                                                                      |
|               pipe () - Creates an interprocess channel              |
|               fork () - Creates a new process                        |
|               dup2 () - Controls open file descriptors               |
|               execl () - Executes a file                             |
|               waitpid () - Waits for a child process to stop or      |
|                            terminate                                 |
|                                                                      |
| Usage:        pipe_test_01                                           |
|                                                                      |
| To compile:   cc -o pipe_test_01 pipe_test_01.c                      |
|                                                                      |
| Last update:   Ver. 1.2, 2/13/94 22:42:00                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     032289  CTU   Initial version                             |
|    0.2     010393  DJK   Rewrite for AIX version 4.1                 |
|    1.2     021394  DJK   Move to prod directory                      |
|                                                                      |
+---------------------------------------------------------------------*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

/* Function prototypes */
void sys_error (const char *, int);	/* System error message function */
void error (const char *, int);	/* Error message function */
void setup_handler ();		/* Sets up signal catching function */
void handler (int, int, struct sigcontext *);	/* Signal catching function */

/*---------------------------------------------------------------------+
|                               main ()                                |
| ==================================================================== |
|                                                                      |
| Function:  Main program  (see prolog for more details)               |
|                                                                      |
+---------------------------------------------------------------------*/
int main (int argc, char **argv)
{
	int	pid [2];		/* Child process ids */
	int	fd [2];  		/* Pipe file descriptors */
	int	status;			/* Child's exit status */

	enum { READ, WRITE };		/* Constants */
	enum { childA, childB };

	/*
	 * Setup signal catching function for SIGPIPE in case an
	 * error occurs
	 */
	setup_handler ();

	/*
	 * Create a Pipe for data transfer between the two child
	 * processes.
	 */
	if (pipe (fd) < 0)
			sys_error ("pipe failed", __LINE__);

	/*
	 * Create child process, run command and write info into pipe.
	 *
	 * Close the read end of the pipe and dup the stdout to the write
	 * end of the pipe, so the the output of the exec'd command will
	 * be written into the pipe.  Then exec the cat command.
	 */
	if ((pid [childA] = fork()) < 0)
		sys_error ("fork failed", __LINE__);

	if (pid [childA] == 0) {
		/* Child process */

		close (fd [READ]);

		/* Redirect STDOUT to fd [WRITE] */
		if (fd [WRITE] != STDOUT_FILENO) {
			if (dup2 (fd [WRITE], STDOUT_FILENO) != STDOUT_FILENO)
				sys_error ("dup2 failed", __LINE__);
		}
		close (fd [WRITE]);

/* Vernon Mauery 6/1/2001 changed path and file to work will more flavors of unix */
		execl ("/bin/cat", "cat", "/etc/inittab", NULL);
		sys_error ("execl failed (should not reach this line) ", __LINE__);
	}

	/*
	 * Create another child process and run command on data passed though
	 * the pipe.
	 *
	 * Close the write end of the pipe and dup the read end of the pipe
	 * to stdin, so that the input of the exec'd command will come
	 * from the pipe.  Then exec the wc command.
	 */
	if ((pid [childB] = fork()) < 0)
		sys_error ("fork failed", __LINE__);

	if (pid [childB] == 0) {
		/* Child process */
		close (fd [WRITE]);

		if (fd [READ] != STDIN_FILENO) {
			if (dup2 (fd [READ], STDIN_FILENO) != STDIN_FILENO)
				sys_error ("dup2 failed", __LINE__);
		}
		close (fd [READ]);

		execl ("/usr/bin/wc","wc","-c",NULL);
		sys_error ("execl failed (should not reach this line) ", __LINE__);
	}

	/*
	 * Close both ends of the pipe and wait for the child processes
	 * to complete.
	 */
	close (fd [READ]);
	close (fd [WRITE]);

	waitpid (pid [childA], &status, 0);
	if (!WIFEXITED (status))
		sys_error ("child process A terminated abnormally", __LINE__);

	waitpid (pid [childB], &status, 0);
	if (!WIFEXITED (status))
		sys_error ("child process B terminated abnormally", __LINE__);

	/* Program completed successfully -- exit */
	return (0);
}

/*---------------------------------------------------------------------+
|                          setup_handler ()                            |
| ==================================================================== |
|                                                                      |
| Function:  Setup the signal handler for SIGPIPE.                     |
|                                                                      |
+---------------------------------------------------------------------*/
void setup_handler ()
{
	struct sigaction invec;

	invec.sa_handler = (void (*)(int)) handler;
	sigemptyset (&invec.sa_mask);
	invec.sa_flags = 0;

	if (sigaction (SIGPIPE, &invec, (struct sigaction *) NULL) < 0)
		sys_error ("sigaction failed", __LINE__);
}

/*---------------------------------------------------------------------+
|                             handler ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Signal catching function for SIGPIPE signal.              |
|                                                                      |
| Returns:   Aborts program & prints message upon receiving signal.    |
|                                                                      |
+---------------------------------------------------------------------*/
void handler (int sig, int code, struct sigcontext *scp)
{
	error ("wrote to pipe with closed read end", __LINE__);
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