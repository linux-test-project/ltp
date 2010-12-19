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
|                            signal_test_04                            |
| ==================================================================== |
|                                                                      |
| Description:  Verify default signal actions (for SIGSTOP & SIGCONT)  |
|               by spawning a child process and sending the signals    |
|               to the child.  Meanwhile, checking to insure that the  |
|               child process takes the default action for each signal |
|               received.                                              |
|                                                                      |
| Algorithm:    o  Use sigaction (SIG_DFL) to take the default action  |
|                  for all signals except for SIGCHLD, SIGUSR1         |
|                  SIGUSR2                                             |
|                                                                      |
|               o  Spawn a child                                       |
|                                                                      |
|               (parent)                                               |
|               o  Ensure that child process is running by waiting     |
|                  for the child process to send a SIGUSR1 signal      |
|               o  Send a SIGSTOP signal to stop the execution of the  |
|                  child process                                       |
|               o  Verify that the child process received the SIGSTOP  |
|                  signal by waiting for the childs SIGCHLD signal     |
|               o  Send a SIGCONT signal to resume the execution of    |
|                  child process.                                      |
|               o  Verify that the child process resumed execution by  |
|                  waiting for the child to send another SIGUSR1       |
|                  signal                                              |
|               o  Kill child process with SIGUSR2 signal..            |
|                                                                      |
|               (child)                                                |
|               o  Take default action for each received signal        |
|               o  Continuously send SIGUSR1 signals to the parent     |
|                  process                                             |
|                                                                      |
|                                                                      |
| System calls: The following system calls are tested:                 |
|                                                                      |
|               sigaction () - specify the action to take upon         |
|                              delivery of a signal                    |
|               wait () - waits for a child process to stop or         |
|                         terminate                                    |
|               kill () - sends a signal to a process                  |
|                                                                      |
| Usage:        signal_test_04                                         |
|                                                                      |
| To compile:   cc -o signal_test_04 signal_test_04                    |
|                                                                      |
| Last update:   Ver. 1.3, 4/8/94 13:07:38                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     112993  DJK   Initial program                             |
|    1.2     020794  DJK   Move to "prod" directory                    |
|                                                                      |
+---------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#ifdef _LINUX_
#  define __USE_XOPEN
#endif
#include <unistd.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <errno.h>

#ifdef _LINUX_
// bits/signum.h defines _NSIG as 64
#define SIGMAX 64
#endif

/* Function prototypes */
void signal_init ();
void child (int);
void handler (int, int, struct sigcontext *);
void sys_error (const char *, int);
void error (const char *, int);

/* Flag set upon receiving SIGCHLD signal */
volatile int sigchld_flag = 0;
volatile int sigusr1_count = 0;

/*---------------------------------------------------------------------+
|                               main ()                                |
| ==================================================================== |
|                                                                      |
| Function:  Main program  (see prolog for more details)               |
|                                                                      |
+---------------------------------------------------------------------*/
int main (int argc, char **argv)
{
	pid_t 	pid = getpid (),	/* Process ID (of parent process) */
		cpid;			/* Process ID (of child process) */
	int	sigusr1_interrupts;	/* xxx */
	int	status;			/* Child's exit status */
	int	timeout = 10;		/* How long to wait for SIGCHILD */
	char	msg [100];		/* Buffer for error message */

	/* Print out program header */
	printf ("%s: IPC TestSuite program\n\n", *argv);
	fflush (stdout);

	/* Set up our signal handler */
	signal_init ();

	/*
	 * Spawn a child process & have the child process send SIGUSR1
	 * signals to the parent.
	 */
	switch (cpid = fork ()) {
		case -1: /* Unable to create child process */
			sys_error ("fork failed", __LINE__);
			exit (-1);

		case 0: /* Child process */
			child (pid);

		default: /* Parent process */
			break;
	}

	/*
	 * Interrupt the child process
	 *
	 * Send SIGSTOP signal to child process (child process will
	 * stop upon receiving the signal).
	 *
	 * Wait for the child process to receive the SIGSTOP signal
	 * and send a corresponding SIGCLD interrupt to the parent.
	 *
	 * Send SIGKILL signal to child process (child process
	 * will exit upon receiving the signal).
	 */
	printf ("\tWait for SIGUSR1 signal from child process\n");
#ifdef _IA64
	while (sigusr1_count < 1) sleep(1);
#else
	while (sigusr1_count < 1) ;
#endif
	printf ("\tReceived SIGUSR1 (30)\n");
	sigusr1_interrupts = sigusr1_count;

	printf ("\n\tStop the child process\n");
	kill (cpid, SIGSTOP);

	printf ("\n\tWait for SIGCHLD signal from stopped child process\n");
	alarm (timeout);
	while (sigchld_flag < 1) ;
	printf ("\tReceived SIGCHLD (20)\n");

	printf ("\n\tResume child process & wait for it to send SIGUSR1 signal\n");
	kill (cpid, SIGCONT);

#ifdef _IA64
	while (sigusr1_interrupts == sigusr1_count) sleep(1) ;
#else
	while (sigusr1_interrupts == sigusr1_count) ;
#endif
	printf ("\tReceived SIGUSR1 (20)\n");

	printf ("\n\tNow kill the child process with SIGUSR2 signal\n");
	kill (cpid, SIGUSR2);

	/*
	 * Wait for the child process to abort
	 *
	 * Suspend execution of the parent process until the SIGCHLD signal
	 * is received upon termination of the child process.
	 *
	 * Check the child process's exit status (with POSIX macros) to
	 * verify that the child process terminated due to the SIGKILL signal
	 * from the parent.
	 *
	 * Additionally verify that the number of SIGCHLD signals received
	 * has increased.
	 */
	printf ("\n\tWait for SIGCHLD signal from killed child process\n");

	wait (&status);

	if (WIFSIGNALED (status)) {
		if (WTERMSIG (status) != SIGUSR2) {
			sprintf (msg, "child process was killed with signal (%d)",
				WTERMSIG (status));
			error (msg, __LINE__);
		}
	} else {
		error ("child process did not receive SIGUSR2 signal", __LINE__);
	}
	if (sigchld_flag < 2)
		error ("child process failed to send SIGCHLD signal", __LINE__);

	printf ("\tReceived SIGCHLD (30)\n");

	/* Program completed successfully -- exit */
	printf ("\nsuccessful!\n");

	return 0;
}

/*---------------------------------------------------------------------+
|                               child ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Child process -- loop until killed.                       |
|            o  continuously send SIGUSR1 interrupts to parent         |
|                                                                      |
+---------------------------------------------------------------------*/
void child (pid_t pid)
{
	while (1) {
		sleep (2);
#ifdef _IA64
		if (kill (pid, SIGUSR1) < 0)
			perror("child: kill");
#else
		kill (pid, SIGUSR1);
#endif
	}

	error ("child process failed to terminate correctly", __LINE__);
}

/*---------------------------------------------------------------------+
|                               handler ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Signal handler                                            |
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
		sigchld_flag++;
	} else if (signal == SIGUSR1) {
		sigusr1_count++;
	} else if (signal == SIGALRM) {
		sprintf (msg, "Timed out waiting for SIGCHLD signal from child, defect # 124551");
		error (msg, __LINE__);
	} else {
		sprintf (msg, "caught an unexpected signal (%d)", signal);
		error (msg, __LINE__);
	}
}

/*---------------------------------------------------------------------+
|                           signal_init ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Take default action for all signals except for SIGCHLD.   |
|            Particularily interrested in the following:               |
|                                                                      |
|              o  SIGSTOP (17) - default action: stop                  |
|              o  SIGCONT (19) - default action: stop                  |
|              o  SIGKILL (09) - default action: abort process         |
|                                                                      |
|            Process signal handler upon receiving the following:      |
|            (Sent when child process stops or exits)                  |
|                                                                      |
|              o  SIGCHLD (20)                                         |
|                                                                      |
| Returns:   n/a                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
void signal_init ()
{
	struct sigaction action;
	char	msg [100];
	int	i;

	for (i=1; i<=SIGMAX; i++) {

		/* Do not ignore SIGCHILD signal */
#ifdef _IA64    /* SIGWAITING not supported, RESERVED */
		if (i == SIGCHLD || i == SIGALRM ||
		    i == SIGUSR1 || i == SIGWAITING) continue;
#else
#  ifdef _LINUX_
       		if ((i == SIGKILL) || (i == SIGSTOP) || ((i>=32)&&(i<=34))) continue;
#  else
                if ((i == SIGKILL) || (i == SIGSTOP) || (i == SIGCONT)) continue;
#  endif
#endif

		action.sa_handler = SIG_DFL;
		sigfillset (&action.sa_mask);
		action.sa_flags = SA_RESTART;

		if (sigaction (i, &action, (struct sigaction *) NULL) < 0) {
			sprintf (msg, "sigaction failed on signal %d", i);
			error (msg, __LINE__);
		}
	}

	/* Setup signal handler for SIGCHLD & SIGUSR1 signals */
	action.sa_handler = (void (*)(int)) handler;
	sigfillset (&action.sa_mask);

	/* changing */
	/*action.sa_flags = 0;*/
	action.sa_flags = SA_RESTART;

	if (sigaction (SIGCHLD, &action, (struct sigaction *) NULL) < 0)
		sys_error ("sigaction (SIGCHLD) failed", __LINE__);

	/* changing */
	action.sa_flags = SA_RESTART;
	/* end of changing */

	if (sigaction (SIGUSR1, &action, (struct sigaction *) NULL) < 0)
		sys_error ("sigaction (SIGUSR1) failed", __LINE__);

	/* Setup signal handler for SIGALRM */
	action.sa_handler = (void (*)(int)) handler;
	if (sigaction (SIGALRM, &action, (struct sigaction *) NULL) < 0)
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
#ifdef _IA64
	fflush(stderr);
#endif
	exit (-1);
}