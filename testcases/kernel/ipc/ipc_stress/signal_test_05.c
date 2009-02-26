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
|                            signal_test_05                            |
| ==================================================================== |
|                                                                      |
| Description:  Verify that signals can be ignored by the receiving    |
|               process.                                               |
|                                                                      |
| Algorithm:    o  Spawn a child                                       |
|                                                                      |
|               (parent)                                               |
|               o  Use sigaction (SIG_IGN) to ignore all catchable     |
|                  signals from child, except for sigchild             |
|               o  Wait until sigchld signal is received               |
|               o  Verify that the sigchild interrupt was received     |
|               o  Additionally check to verify that child process     |
|                  exited successfully                                 |
|                                                                      |
|               (child)                                                |
|               o  Send all catchable signals to the parent            |
|               o  exit (implicitly send sigchild signal to parent)    |
|                                                                      |
|                                                                      |
| System calls: The following system calls are tested:                 |
|                                                                      |
|               sigaction () - Specify the action to take upon         |
|                              delivery of a signal                    |
|               wait () - Waits for a child process to stop or         |
|                         terminate                                    |
|               kill () - Sends a signal to a process                  |
|                                                                      |
| Usage:        signal_test_05                                         |
|                                                                      |
| To compile:   cc -o signal_test_05 signal_test_05                    |
|                                                                      |
| Last update:   Ver. 1.2, 2/7/94 23:24:05                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     050689  CTU   Initial version                             |
|    0.2     112993  DJK   Rewrite for AIX version 4.1                 |
|    1.2     020794  DJK   Move to "prod" directory                    |
|                                                                      |
+---------------------------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#ifdef _LINUX_
#  define __USE_XOPEN
#  include <sys/types.h>
#  include <sys/stat.h>
#endif
#include <unistd.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _LINUX_
// bits/signum.h defines _NSIG as 64
#define SIGMAX 64
#endif

/* Function prototypes */
void ignore_signals ();
void child (int);
void handler (int, int, struct sigcontext *);
void sys_error (const char *, int);
void error (const char *, int);


/* Flag set upon receiving SIGCHLD signal */
int sigchld_flag = 0;


/*---------------------------------------------------------------------+
|                               main ()                                |
| ==================================================================== |
|                                                                      |
| Function:  Main program  (see prolog for more details)               |
|                                                                      |
+---------------------------------------------------------------------*/
int main (int argc, char **argv)
{

	pid_t	pid = getpid ();	/* Process ID (of parent process) */
	int	status;			/* Child's exit status */

	/* Print out program header */
	printf ("%s: IPC TestSuite program\n\n", *argv);
   
	/* Set up our signal handler */
	ignore_signals ();

	/*
	 * Spawn a child process & have the child process send all of the
	 * catchable signals to the parent.
	 */
	printf ("\n\tSpawning child process\n");
	switch (fork ()) {
		case -1: /* Unable to create child process */
			sys_error ("fork failed", __LINE__);

		case 0: /* Child process */
			child (pid);

		default: /* Parent process */
			break;
	}

	/*
	 * Wait for the child process to complete
	 *
	 * Suspend execution of the parent process until either a signal
	 * that is not blocked or ignored, or until the child process
	 * completes. 
	 *
	 * Use the POSIX macro to insure that the child process exited
	 * normally.
	 *
	 * Check to insure that the SIGCHLD signal was caught.
	 */
	wait (&status);

	if (!WIFEXITED (status))
		error ("child process exited abnormally", __LINE__);

	if (sigchld_flag != 1)
		error ("child process failed to send SIGCHLD signal", __LINE__);

	printf ("\n\tChild process exited successfully\n");

	/* Program completed successfully -- exit */
	printf ("\nsuccessful!\n");

	return 0;
}


/*---------------------------------------------------------------------+
|                               child ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Child process                                             |
|            o  Send all catchable signals (except sigchild) to parent |
|            o  exit & implicitly send sigchild signal to parent       |
|                                                                      |
| Parms:     pid - process id of parent process                        |
|                                                                      |
+---------------------------------------------------------------------*/
void child (pid_t pid)
{
	int i;

	/* Send one of each of the possible signals to the process */
	printf ("\n\tChild: sending ALL signals to parent!\n");
	for (i=1; i< (SIGMAX + 1); i++) {

		/* Cannot catch or ignore the following signals */
#ifdef _LINUX_
		if ((i == SIGKILL) || (i == SIGSTOP) || ((i>=32)&&(i<=34))) continue;
#else
		if (i == SIGKILL || i == SIGSTOP || i == SIGCONT) continue;
#endif

		/* Skip sigchild too */
		if (i == SIGCHLD) continue;
#ifdef _IA64
		/* RESERVED - DON'T USE */
		if ( i == SIGWAITING) continue;
#endif

		printf ("\tSending (%d)\n", i);

		if (kill (pid, i) < 0)
			sys_error ("kill failed", __LINE__);
	}

	/* Exit & implicitly send a sigchild interrupt to the parent process */
	exit (0);
}


/*---------------------------------------------------------------------+
|                               handler ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Signal handler                                            |
|            o  Send all catchable signals (except sigchild) to parent |
|                                                                      |
| Parms:     signal - signal number caught                             |
|                                                                      |
| Returns:   Aborts if an unexpected signal is caught                  |
|                                                                      |
+---------------------------------------------------------------------*/
void handler (int signal, int code, struct sigcontext *scp)
{
	char msg [100];

	if (signal == SIGCHLD) {
		printf ("\tcaught SIGCHLD(%d) signal\n", signal);
		sigchld_flag = 1;
	} else {
		sprintf (msg, "caught an unexpected signal (%d)", signal);
		error (msg, __LINE__);
	}
}


/*---------------------------------------------------------------------+
|                           ignore_signals ()                          |
| ==================================================================== |
|                                                                      |
| Function:  Force all signals to be ignored, except for the following |
|            signals:                                                  |
|                                                                      |
|            (Sent when child process stops or exits)                  |
|                                                                      |
|              o  SIGCHLD (20)                                         |
|                                                                      |
|            (Theses cannot be caught or ignored)                      |
|                                                                      |
|              o  SIGKILL (9)                                          |
|              o  SIGSTOP (17)                                         |
|              o  SIGCONT (19)                                         |
|                                                                      |
| Returns:   n/a                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
void ignore_signals ()
{
	struct sigaction action;
	char	msg [100];
	int i;

	for (i=1; i< (SIGMAX + 1); i++) {

		/* Cannot catch or ignore the following signals: */
#ifdef _LINUX_
       		if ((i == SIGKILL) || (i == SIGSTOP) || ((i>=32)&&(i<=34))) continue;
#else
		if (i == SIGKILL || i == SIGSTOP || i == SIGCONT) continue;
#endif

		/* Do not ignore SIGCHILD signal */
		if (i == SIGCHLD) continue;

		action.sa_handler = SIG_IGN;
		sigfillset (&action.sa_mask);
		action.sa_flags = SA_RESTART;

		if (sigaction (i, &action, (struct sigaction *) NULL) < 0) {
		        perror("ignore_signals: sigaction");
			sprintf (msg, "sigaction failed on signal %d", i);
			error (msg, __LINE__);
		}
	}

	/* Setup signal handler for SIGCHLD signal */
	action.sa_handler = (void (*)(int)) handler;
	sigfillset (&action.sa_mask);
	action.sa_flags = SA_RESTART;
	if (sigaction (SIGCHLD, &action, (struct sigaction *) NULL) < 0)
		sys_error ("sigaction (SIGCHLD) failed", __LINE__);
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
