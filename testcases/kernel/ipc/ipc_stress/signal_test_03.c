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
|                           signal_test_03                             |
| ==================================================================== |
|                                                                      |
| Description:  Block a critical section from receiving signals.       |
|                                                                      |
| Algorithm:    o  Setup a signal-catching function                    |
|               o  Beginning of Critical section -- Set the process    |
|                  signal mask to block the SIGILL signal              |
|               o  Send SIGILL signal to the process to insure that    |
|                  the signal is blocked during the critical section   |
|               o  Preform the critical section code (merely sleep     |
|                  for testing purposes)                               |
|               o  Verify that the signal was blocked                  |
|               o  End of Critical section -- unblock the signal and   |
|                  suspend he process signal mask to unblock one       |
|                  signal and suspend execution of the process until   |
|                  the signal is received.  Verify that the unblocked  |
|                  signal is received.                                 |
|                                                                      |
| System calls: The following system calls are tested:                 |
|                                                                      |
|               sigsetmask () - Sets the current signal mask           |
|               sigblock () - Sets the current signal mask             |
|               sigvec () - Specify the action to take upon delivery   |
|                           of a signal.                               |
|               raise () - Sends a signal to the executing program     |
|                                                                      |
| Usage:        signal_test_03                                         |
|                                                                      |
| To compile:   cc -o signal_test_03 signal_test_03                    |
|                                                                      |
| Last update:   Ver. 1.2, 2/7/94 23:23:34                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     050689  CTU   Initial version                             |
|    1.2     112293  DJK   Rewrite for AIX version 4.1                 |
|                                                                      |
+---------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MASK(sig)  (1 << ((sig) - 1))
#define MAXTIME	2			/* MAX timeout (minutes) */

#ifdef _LINUX_
// bits/signum.h defines _NSIG as 64
#define SIGMAX 64
#endif

#include "signals.h"

/* Function prototypes */
void handler (int);
void init_sig_vec ();
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
	int	timeout = MAXTIME*60;	/* Number sec to wait for signal */

	/* Print out program header */
	printf ("%s: IPC Signals TestSuite program\n\n", *argv);

	/* Set up our signal handlers */
	init_sig_vec ();

	/*
	 * Critical section - block SIGILL signal
	 * 
	 * Block the SIGILL interrupt from interrupting the process
	 * with the sigprocmask () system function call.
	 * 
	 * Send the SIGILL interrupt to the process in an attempt to
	 * disrupt the critial section -- the signal should be blocked.
	 * Wait one second to insure that the signal has plenty of time
	 * to reach the process. 
	 */
#ifdef _LINUX_
	sigset_t mask;
	sigemptyset (&mask);
	sigaddset (&mask, SIGILL);
	sigprocmask (SIG_BLOCK, &mask, NULL);
#else
	if (sigblock ( MASK (SIGILL) ) < 0)
		sys_error ("sigblock failed", __LINE__);
#endif

	printf ("\t(BEGIN) Critial section\n");

	/* Critial section */
	sleep (1);

	/*
	 * End of critical section - ensure SIGILL signal was not received
	 *
	 * Check to insure that the signal handler has not caught any signals,
	 * and then unblock all of the signals with the sigsetmask system
	 * function call. 
	 */
	if (signals_received > 0)
		error ("received an unexpected signal during the critical section",
			__LINE__);

	printf ("\n\t(END) Critial section\n");

#ifdef _LINUX_
	sigemptyset (&mask);
	sigprocmask (SIG_SETMASK, &mask, NULL);
#else
	if (sigsetmask (0) < 0)
		sys_error ("sigsetmask failed", __LINE__);
#endif
	raise (SIGILL);

	/*
	 * Upon unblocking the signals, should receive the SIGILL signal.
	 * Verify that it indeed is caught.
	 */
	while (signals_received == 0 && --timeout)
	  {
	        printf(".");
		fflush(stdout);
		sleep (1);
	  }

	if (timeout == 0)
		error ("failed to receive SIGILL signal after unblocking signals",
			__LINE__);

	/* Program completed successfully -- exit */
	printf ("\nsuccessful!\n");
	return (0);
}


/*---------------------------------------------------------------------+
|                           init_sig_vec ()                            |
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
| Returns:   Nothing                                                   |
|                                                                      |
+---------------------------------------------------------------------*/
void init_sig_vec ()
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
|                             handler ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Signal catching function.  This function is called each   |
|            time a non-blocked signal is received by the process.     |
|                                                                      |
|            Increment the global variable (signals_received) for      |
|            each received signal.                                     |
|                                                                      |
| Returns:   Nothing                                                   |
|                                                                      |
+---------------------------------------------------------------------*/
void handler (int signal)
{
	if (signal == SIGILL) signals_received++;
	printf ("\treceived signal: (%s)\n", signames[signal]);
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
