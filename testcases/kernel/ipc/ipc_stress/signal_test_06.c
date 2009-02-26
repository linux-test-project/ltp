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
|                            signal_test_06                            |
| ==================================================================== |
|                                                                      |
| Description:  Verifies that when multiple identical signals are sent |
|               to a process, at least one is queued.                  |
|                                                                      |
| Algorithm:    o  Block all signals from interrupting the process     |
|               o  Send MAXSIG signals to the current process          |
|               o  Sleep for a short time                              |
|               o  Verify that signals are pending                     |
|               o  Change the signal mask and suspend execution until  |
|                  a signal interrupts the process                     |
|               o  Verify that at least one signal was received        |
|                                                                      |
| System calls: The following system calls are tested:                 |
|                                                                      |
|               sigprocmask () - Sets the current signal mask          |
|               sigismember () - Creates and manipulates signal masks  |
|               sigfillset () - Creates and manipulates signal masks   |
|               sigpending () - Returns a set of signals that are      |
|                               blocked from delivery                  |
|               sigsuspend () - Automatically changes the set of       |
|                               blocked signals and waits for a signal |
|               raise () - Sends a signal to the executing program     |
|                                                                      |
| Usage:        signal_test_06                                         |
|                                                                      |
| To compile:   cc -o signal_test_06 signal_test_06                    |
|                                                                      |
| Last update:   Ver. 1.2, 2/7/94 23:24:14                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     122193  DJK   Wrote initial test for AIX version 4.1      |
|    1.2     020794  DJK   Move to "prod" directory                    |
|                                                                      |
+---------------------------------------------------------------------*/

#define MAXSIG 1024*1024	/* Number of signals sent to process */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/signal.h>
#include <errno.h>

#ifdef _LINUX_
// bits/signum.h defines _NSIG as 64
#define SIGMAX 64
#endif

/* Function prototypes */
void handler (int, int, struct sigcontext *);
void init_sig ();
void sys_error (const char *, int);
void error (const char *, int);


/* Global variables */
int signals_received = 0;


/*---------------------------------------------------------------------+
|                               main ()                                |
| ==================================================================== |
|                                                                      |
| Function:  Main program  (see prolog for more details)               |
|                                                                      |
+---------------------------------------------------------------------*/
int main (int argc, char **argv)
{
	sigset_t	newmask, 	/* New signal mask */
			oldmask, 	/* Initial signal mask */
			pendmask; 	/* Pending signal mask */
	int		i;		/* Loop index */

	/* Print out program header */
	printf ("%s: IPC TestSuite program\n\n", *argv);
   
	/* Set up our signal handler */
	init_sig ();

	/*
	 * Block ALL signals from interrupting the process
	 */
	printf ("\tBlock all signals from interrupting the process\n");
	if (sigfillset (&newmask) < 0)
		error ("sigfillset failed", __LINE__);
	if (sigprocmask (SIG_SETMASK, &newmask, &oldmask) < 0)
		error ("sigprocmask failed", __LINE__);

	/*
	 * Send MAXSIG signals to the current process -- since ALL of the
	 * signals are blocked, none of the signals should interrupt the
	 * process
	 */
	printf ("\n\tSend MAX (%d) SIGUSR1 signals to the process...\n", MAXSIG);
	for (i=0; i<MAXSIG; i++) raise (SIGUSR1);

	/*
	 *  Sleep for a short time and the check to ensure that a SIGUSR1
	 *  signal is pending
	 */
	sleep (2);

	printf ("\n\tEnsure at least one SIGUSR1 signal is pending\n");
	if (sigpending (&pendmask) < 0)
		error ("sigpending failed", __LINE__);

	if (sigismember (&pendmask, SIGUSR1) == 0)
		error ("sent multiple SIGUSR1 signals to process, " \
			"yet none are pending!", __LINE__);

	/*
	 * Change the signal mask to allow signals to interrupt the process
	 * and then suspend execution until a signal reaches the process
	 *
	 * Then verify that at least one signal was received
	 */
	printf ("\n\tChange signal mask & wait for SIGUSR1 signal\n");
	if (sigsuspend(&oldmask) != -1 || errno != 4)
		error ("sigsuspend failed", __LINE__);

	if (signals_received != 1) {
		printf ("Signals are queued!  Sent %d signals, " \
			"while %d were queued\n",
			MAXSIG, signals_received);
	}

	/* Program completed successfully -- exit */
	printf ("\nsuccessful!\n");
	return (0);
}


/*---------------------------------------------------------------------+
|                               handler ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Catches signals and aborts the program if a signal other  |
|            than SIGUSR1 was received                                 |
|                                                                      |
| Updates:   signals_received - global variable indicating the number  |
|                               of SIGUSR1 signals received            |
|                                                                      |
| Returns:   Aborts if an unexpected signal is caught                  |
|                                                                      |
+---------------------------------------------------------------------*/
void handler (int signal, int code, struct sigcontext *scp)
{
	char msg [256];		/* Buffer for error message */

	if (signal != SIGUSR1) {
		sprintf (msg, "unexpected signal (%d)", signal);
		error (msg, __LINE__);
	}

	printf ("\tcaught SIGUSR1 (%d) signal\n", signal);
	signals_received++;
}


/*---------------------------------------------------------------------+
|                             init_sig ()                              |
| ==================================================================== |
|                                                                      |
| Function:  Initialize the signal vector for ALL possible signals     |
|            (as defined in /usr/include/sys/signal.h) except for      |
|            the following signals which cannot be caught or ignored:  |
|                                                                      |
|              o  SIGKILL (9)                                          |
|              o  SIGSTOP (17)                                         |
|              o  SIGCONT (19)                                         |
|                                                                      |
| Returns:   n/a                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
void init_sig ()
{
	struct sigaction invec;
	char 	msg [256];		/* Buffer for error message */
	int 	i;

	for (i=1; i<=SIGMAX; i++) {

		/* Cannot catch or ignore the following signals */
#ifdef _IA64    /* SIGWAITING not supported, RESERVED */
		if ((i == SIGKILL) || (i == SIGSTOP) ||
		    (i == SIGCONT) || (i == SIGWAITING)) continue;
#else
# ifdef _LINUX_
       		if ((i == SIGKILL) || (i == SIGSTOP) || ((i>=32)&&(i<=34))) continue;
# else
		if (i == SIGKILL || i == SIGSTOP || i == SIGCONT) continue;
# endif
#endif

		invec.sa_handler = (void (*)(int)) handler;
		sigemptyset (&invec.sa_mask);
		invec.sa_flags = 0;

		if (sigaction (i, &invec, (struct sigaction *) NULL) < 0) {
			sprintf (msg, "sigaction failed on signal %d", i);
			error (msg, __LINE__);
		}
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
