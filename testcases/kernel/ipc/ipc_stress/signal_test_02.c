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
|                          signal_test_02                              |
| ==================================================================== |
|                                                                      |
| Description:  Simplistic test to verify the signal system function   |
|               calls:                                                 |
|                                                                      |
|               o  Setup a signal-catching function for every possible |
|                  signal.                                             |
|               o  Send signals to the process and verify that they    |
|                  were received by the signal-catching function.      |
|               o  Block a few signals by changing the process signal  |
|                  mask.  Send signals to the process and verify that  |
|                  they indeed were blocked.                           |
|               o  Add additional signals to the process signal mask.  |
|                  Verify that they were blocked too.                  |
|               o  Change the process signal mask to unblock one       |
|                  signal and suspend execution of the process until   |
|                  the signal is received.  Verify that the unblocked  |
|                  signal is received.                                 |
|                                                                      |
| System calls: The following system calls are tested:                 |
|                                                                      |
|               sigprocmask () - Sets the current signal mask          |
|               sigemptyset () - Creates and manipulates signal masks  |
|               sigfillset () - Creates and manipulates signal masks   |
|               sigaddset () - Creates and manipulates signal masks    |
|               sigdelset () - Creates and manipulates signal masks    |
|               sigsuspend () - Atomically changes the set of blocked  |
|                               signals and waits for a signal         |
|               sigaction () - Specifies the action to take upon       |
|                              delivery of a signal                    |
|               kill () - Sends a signal to a process                  |
|                                                                      |
| Usage:        signal_test_03                                         |
|                                                                      |
| To compile:   cc -o signal_test_03 signal_test_03                    |
|                                                                      |
| Last update:   Ver. 1.2, 2/7/94 23:17:48                             |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     050689  CTU   Initial version                             |
|    0.2     112293  DJK   Rewrite for AIX version 4.1                 |
|    1.2     020794  DJK   Move to "prod" directory                    |
|    1.3     060501  VHM   Port to work in linux                       |
|                                                                      |
+---------------------------------------------------------------------*/

#define SIGMAX 64 /* What should this number really be? _NSIG from bits/signum.h maybe? */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

/* Macro for specifying signal masks */
#define MASK(sig)  (1 << ((sig) - 1))

#include "signals.h"

/* Function prototypes */
void init_sig ();
void handler (int sig);//, int code, struct sigcontext *);
void reset_valid_sig ();
void sys_error (const char *, int);
void error (const char *, int);

/* Define an array for verifying received signals */
int valid_sig [ SIGMAX + 1 ];

/*---------------------------------------------------------------------+
|                               main ()                                |
| ==================================================================== |
|                                                                      |
| Function:  Main program  (see prolog for more details)               |
|                                                                      |
+---------------------------------------------------------------------*/
int main (int argc, char **argv)
{
	sigset_t	setsig, 		/* Initial signal mask */
			newsetsig;		/* Second signal mask */
	pid_t		pid = getpid ();	/* Process ID (of this process) */

	/* Print out program header */
	printf ("%s: IPC TestSuite program\n\n", *argv);

	/*
	 * Establish signal handler for each signal & reset "valid signals"
	 * array
	 */
	init_sig ();
	reset_valid_sig ();

	sigemptyset(&setsig);
	if (sigprocmask (SIG_SETMASK, &setsig, (sigset_t *) NULL) < 0)
		sys_error ("sigprocmask failed", __LINE__);

	/*
	 * Send SIGILL, SIGALRM & SIGIOT signals to this process:
	 *
	 * First indicate which signals the signal handler should expect
	 * by setting the corresponding valid_sig[] array fields.
	 *
	 * Then send the signals to this process.
	 *
	 * And finally verify that the signals were caught by the signal
	 * handler by checking to see if the corresponding valid_sig[] array
	 * fields were reset.
	 */
	printf ("\tSend SIGILL, SIGALRM, SIGIOT signals to process\n");
	valid_sig [SIGILL] = 1;
	valid_sig [SIGALRM] = 1;
	valid_sig [SIGIOT] = 1;

	kill (pid, SIGILL);
	kill (pid, SIGALRM);
	kill (pid, SIGIOT);

	if (valid_sig [SIGILL])
		error ("failed to receive SIGILL signal!", __LINE__);
	if (valid_sig [SIGALRM])
		error ("failed to receive SIGALRM signal!", __LINE__);
	if (valid_sig [SIGIOT])
		error ("failed to receive SIGIOT signal!", __LINE__);

	/*
	 * Block SIGILL, SIGALRM & SIGIOT signals:
	 *
	 * First initialize the signal set so that all signals are excluded,
	 * then individually add the signals to block to the signal set.
	 *
	 * Then change the process signal mask with sigprocmask (SIG_SETMASK).
	 *
	 * Verify that the desired signals are blocked from interrupting the
	 * process, by sending both blocked and unblocked signals to the
	 * process.  Only the unblocked signals should interrupt the process.
	 */
	printf ("\n\tBlock SIGILL, SIGALRM, SIGIOT signals, " \
		"and resend signals + others\n");
	sigemptyset(&setsig);

	if (sigaddset (&setsig, SIGIOT) < 0)
		sys_error ("sigaddset (SIGIOT) failed", __LINE__);
	if (sigaddset (&setsig, SIGILL) < 0)
		sys_error ("sigaddset (SIGILL) failed", __LINE__);
	if (sigaddset (&setsig, SIGALRM) < 0)
		sys_error ("sigaddset (SIGALRM) failed", __LINE__);

	if (sigprocmask (SIG_SETMASK, &setsig, (sigset_t *) NULL) < 0)
		sys_error ("sigaddset (SIGALRM) failed", __LINE__);

	valid_sig [SIGFPE] = 1;
	valid_sig [SIGTERM] = 1;
	valid_sig [SIGINT] = 1;

	kill (pid, SIGILL);
	kill (pid, SIGALRM);
	kill (pid, SIGIOT);
	kill (pid, SIGFPE);
	kill (pid, SIGTERM);
	kill (pid, SIGINT);

	if (valid_sig [SIGFPE])
		sys_error ("failed to receive SIGFPE signal!", __LINE__);
	if (valid_sig [SIGTERM])
		sys_error ("failed to receive SIGTERM signal!", __LINE__);
	if (valid_sig [SIGINT])
		sys_error ("failed to receive SIGINT signal!", __LINE__);

	/*
	 * Block additional SIGFPE, SIGTERM & SIGINT signals:
	 *
	 * Create an other signal set to contain the additional signals to block
	 * and add the signals to block to the signal set.
	 *
	 * Change the process signal mask to block the additional signals
	 * with the sigprocmask (SIG_BLOCK) function.
	 *
	 * Verify that all of the desired signals are now blocked from
	 * interrupting the process.  None of the specified signals should
	 * interrupt the process until the process signal mask is changed.
	 */
	printf ("\n\tBlock rest of signals\n");
	sigemptyset (&newsetsig);

	sigaddset (&newsetsig, SIGFPE);
	sigaddset (&newsetsig, SIGTERM);
	sigaddset (&newsetsig, SIGINT);

	if (sigprocmask (SIG_BLOCK, &newsetsig, &setsig) < 0)
		sys_error ("sigprocmask failed", __LINE__);

	kill (pid, SIGILL);
	kill (pid, SIGALRM);
	kill (pid, SIGIOT);
	kill (pid, SIGFPE);
	kill (pid, SIGTERM);
	kill (pid, SIGINT);

	/*
	 * Wait two seconds just to make sure that none of the specified
	 * signals interrupt the process (They should all be blocked).
	 */
	sleep (2);

	/*
	 * Change the process signal mask:
	 *
	 * Now specifiy a new process signal set to allow the SIGINT signal
	 * to interrupt the process.  Create the signal set by initializing
	 * the signal set with sigfillset () so that all signals are included
	 * in the signal set, then remove the SIGINT signal from the set with
	 * sigdelset ().
	 *
	 * Force the  process to suspend execution until delivery of an
	 * unblocked signal (SIGINT in this case) with sigsuspend ().
	 *
	 * Additionally, verify that the SIGINT signal was received.
	 */
	valid_sig [SIGINT] = 1;

	printf ("\n\tChange signal mask & wait until signal interrupts process\n");
	if (sigfillset (&setsig) < 0)
		sys_error ("sigfillset failed", __LINE__);
	if (sigdelset (&setsig, SIGINT) < 0)
		sys_error ("sigdelset failed", __LINE__);
	if (sigsuspend(&setsig) != -1 || errno != 4)
		sys_error ("sigsuspend failed", __LINE__);

	if (valid_sig [SIGINT])
		error ("failed to receive SIGIOT signal!", __LINE__);

	/* Program completed successfully -- exit */
	printf ("\nsuccessful!\n");

	return (0);
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
|                             handler ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Signal catching function.  As specified in init_sig_vec() |
|            this function is automatically called each time a signal  |
|            is received by the process.                               |
|                                                                      |
|            Once receiving the signal, verify that the corresponding  |
|            signal was expected.                                      |
|                                                                      |
| Returns:   Aborts program if an unexpected signal was received.      |
|                                                                      |
+---------------------------------------------------------------------*/
void handler (int sig)//, int code, struct sigcontext *scp)
{
	char 	msg [256];

	/* Check to insure that expected signal was received */
	if (valid_sig [sig]) {
		valid_sig [sig] = 0;
		printf ("\treceived signal: (%s)\n", signames[sig]);
	} else {
		sprintf (msg, "unexpected signal (%d,%s)", sig, (sig<32)?signames[sig]:"unknown signal");
		error (msg, __LINE__);
	}
}

/*---------------------------------------------------------------------+
|                         reset_valid_sig ()                           |
| ==================================================================== |
|                                                                      |
| Function:  Reset the valid "signal" array                            |
|                                                                      |
| Returns:   n/a                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
void reset_valid_sig ()
{
	int i;

	for (i=0; i< (SIGMAX + 1); i++)
		valid_sig [i] = 0;
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