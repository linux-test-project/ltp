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
|                           signal_test_01                             |
| ==================================================================== |
|                                                                      |
| Description:  Simplistic test to verify the signal system function   |
|               calls:                                                 |
|                                                                      |
| Algorithm:    o  Setup a signal-catching function for every possible |
|                  signal                                              |
|               o  Send signals to the process and verify that they    |
|                  were received by the signal-catching function       |
|               o  Block a few signals by changing the process signal  |
|                  mask.  Send signals to the process and verify that  |
|                  they indeed were blocked                            |
|               o  Add additional signals to the process signal mask.  |
|                  Verify that they were blocked too                   |
|               o  Change the process signal mask to unblock one       |
|                  signal and suspend execution of the process until   |
|                  the signal is received.  Verify that the unblocked  |
|                  signal is received                                  |
|                                                                      |
| System calls: The following system calls are tested:                 |
|                                                                      |
|               sigstack () - Sets signal stack context                |
|               sigsetmask () - Sets the current signal mask           |
|               sigblock () - Sets the current signal mask             |
|               sigpause () - Automically changes the set of blocked   |
|                             signals and waits for a signal           |
|               sigvec () - Specify the action to take upon delivery   |
|                           of a signal.                               |
|               kill () - Sends a signal to a process                  |
|                                                                      |
| Usage:        signal_test_01                                         |
|                                                                      |
| To compile:   cc -o signal_test_01 signal_test_01.c                  |
|                                                                      |
| Last update:   Ver. 1.3, 4 June 2001                                 |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     050689  CTU   Initial version                             |
|    0.2     112293  DJK   Rewrite for AIX version 4.1                 |
|    1.2     020794  DJK   Move to "prod" directory                    |
|    1.3     060401  VHM   Port to work in linux                       |
|                                                                      |
+---------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _LINUX_
#  define __USE_XOPEN
#endif
#include <signal.h>
#include <unistd.h>

/* Macro for specifying signal masks */
#define STACKSIZE SIGSTKSZ

/* Define an alternative stack for processing signals */
char stackarray [STACKSIZE];

#include "signals.h"

#ifdef _LINUX_
stack_t stack = {
    ss_sp: stackarray+STACKSIZE,    // stack pointer
    ss_flags: 0, //SS_ONSTACK,      // flags
    ss_size: STACKSIZE              // size
};
stack_t *oldstack;

// SIGMAX is defined in the AIX headers to be 63 - 64 seems to work in linux??
// however, signals 32, 33, and 34 are not valid
# define SIGMAX 64

#else // ! _LINUX_

struct sigstack stack = {
	stackarray+STACKSIZE, 		/* signal stack pointer */
	(_ONSTACK & ~ _OLDSTYLE)	/* current status */
};
#endif

/* Function prototypes */
void handler (int);			/* signal catching function */
void init_sig_vec ();			/* setup signal handler for signals */
void reset_valid_sig ();		/* reset valid_sig array */
void sys_error (const char *, int);	/* system error message function */
void error (const char *, int);		/* error message function */


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
    sigset_t mask,		/* Initial process signal mask */
	     newmask, 		/* Second process signal mask */
	     oldmask;		/* Signal mask returned by sigblock */
    pid_t pid = getpid ();	/* Process ID (of this process) */
   
   
	/* Print out program header */
	printf ("%s: IPC Signals TestSuite program\n\n", *argv);
   
	/*
	 * Establish signal handler for each signal & reset "valid signals"
	 * array, and setup alternative stack for processing signals
	 */
	init_sig_vec ();
	reset_valid_sig ();
#ifdef _LINUX_
	// sigstack function is obsolete, use sigaltstack instead
	if (sigaltstack (&stack, NULL) < 0)
	        sys_error ("sigaltstack failed", __LINE__);
#else
	if (sigstack (&stack, (struct sigstack *)0) < 0)
		sys_error ("sigstack failed", __LINE__);
#endif
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
	valid_sig [SIGILL]  = 1;
	valid_sig [SIGALRM] = 1;
	valid_sig [SIGIOT]  = 1;

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
	 * First create the process signal mask by ORing together the
	 * signal values.
	 *
	 * Then change the process signal mask with sigsetmask ().
	 *
	 * Verify that the desired signals are blocked from interrupting the
	 * process, by sending both blocked and unblocked signals to the
	 * process. Only the unblocked signals should interrupt the process.
	 */
	printf ("\n\tBlock SIGILL, SIGALRM, SIGIOT signals, " \
		"and resend signals + others\n");
	sigemptyset (&mask);
	sigaddset (&mask, SIGILL);
	sigaddset (&mask, SIGALRM);
	sigaddset (&mask, SIGIOT);
#ifdef _LINUX_
	sigprocmask(SIG_SETMASK, &mask, NULL);
#else
	if (sigsetmask (mask) < 0)
		sys_error ("setsigmask failed", __LINE__);
#endif
	valid_sig [SIGFPE]  = 1;
	valid_sig [SIGTERM] = 1;
	valid_sig [SIGINT]  = 1;

	kill (pid, SIGILL);
	kill (pid, SIGALRM);
	kill (pid, SIGIOT);
	kill (pid, SIGFPE);
	kill (pid, SIGTERM);
	kill (pid, SIGINT);
	
	if (valid_sig [SIGFPE])
		error ("failed to receive SIGFPE signal!", __LINE__);
	if (valid_sig [SIGTERM])
		error ("failed to receive SIGTERM signal!", __LINE__);
	if (valid_sig [SIGINT])
		error ("failed to receive SIGINT signal!", __LINE__);

	/*
	 * Block additional SIGFPE, SIGTERM & SIGINT signals:
	 *
	 * Create a signal mask containing the additional signals to block.
	 *
	 * Change the process signal mask to block the additional signals
	 * with the sigprocmask () function.
	 *
	 * Verify that all of the desired signals are now blocked from
	 * interrupting the process.  None of the specified signals should
	 * interrupt the process until the process signal mask is changed.
	 */
	printf ("\n\tBlock rest of signals\n");
        sigemptyset (&newmask);
        sigaddset (&newmask, SIGFPE);
        sigaddset (&newmask, SIGTERM);
        sigaddset (&newmask, SIGINT);
	sigemptyset (&oldmask);
	if ( sigprocmask (SIG_BLOCK, &newmask, &oldmask) < 0) {
		perror ("sigprocmask failed");
		exit (-1);
	}
	if (memcmp (&mask, &oldmask, sizeof(mask)) != 0)
		error ("value returned by sigblock () does not match the " \
			"old signal mask", __LINE__);

	kill (pid, SIGILL);
	kill (pid, SIGALRM);
	kill (pid, SIGIOT);
	kill (pid, SIGFPE);
	kill (pid, SIGTERM);
	kill (pid, SIGINT);

	/* Wait two seconds just to make sure that none of the specified
	 * signals interrupt the process (They should all be blocked).
	 */
	sleep (2);

	/* Change the process signal mask:
	 *
	 * Now allow the SIGINT signal to interrupt the process.
	 * Thus by using sigpause (), force the process to suspend
	 * execution until delivery of an unblocked signal (here SIGINT).
	 *
	 * Additionally, verify that the SIGINT signal was received.
	 */
	valid_sig [SIGINT] = 1;

	printf ("\n\tChange signal mask & wait until signal interrupts process\n");
	if (sigpause (SIGINT) != -1 || errno != 4)
		sys_error ("sigpause failed", __LINE__);

	if (valid_sig [SIGINT])
		error ("failed to receive SIGINT signal!", __LINE__);

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
	char	errmsg [256];
	int	i;

#ifdef _LINUX_
	static struct sigaction invec;

	for (i=1; i<=SIGMAX; i++) {
       		if ((i == SIGKILL) || (i == SIGSTOP) || ((i>=32)&&(i<=34))) continue;
		invec.sa_handler = handler;
		//sigaction.sa_mask = 0;
		invec.sa_flags = 0;

		if (sigaction (i, &invec, (struct sigaction *)0)) {
		        sprintf (errmsg, "init_sig_vec: sigaction failed on signal (%d)", i);
		        perror(errmsg);
			sys_error (errmsg, __LINE__);
		}
	}
#else
	static struct sigvec invec;

	for (i=1; i<=SIGMAX; i++) {

		/* Cannot catch or ignore the following signals */
# ifdef _IA64  /* SIGWAITING NOT supported, RESERVED */
		if ((i == SIGKILL) || (i == SIGSTOP) ||
		    (i == SIGCONT) || (i == SIGWAITING)) continue;
# else
                if ((i == SIGKILL) || (i == SIGSTOP) || (i == SIGCONT)) continue;
# endif

		invec.sv_handler = handler;
		//invec.sv_mask = SA_NOMASK;
# if defined  _IA64
		invec.sv_flags = 1;
# else
		invec.sv_onstack = 1;
# endif
		if (sigvec (i, &invec, (struct sigvec *)0)) {
		        sprintf (errmsg, "init_sig_vec: sigvec failed on signal (%d)", i);
		        perror(errmsg);
			sys_error (errmsg, __LINE__);
		}
	}
#endif //ifdef _LINUX_
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
void handler (int sig)
{
	char errmsg [256];

	/* Check to insure that expected signal was received */
	if (valid_sig [sig]) {
		valid_sig [sig] = 0;
		printf ("\treceived signal: (%s)\n", signames[sig]);
	} else {
		sprintf (errmsg, "unexpected signal (%d,%s)", sig, (sig<32)?signames[sig]:"unknown signal");
		error (errmsg, __LINE__);
	}
}


/*---------------------------------------------------------------------+
|                         reset_valid_sig ()                           |
| ==================================================================== |
|                                                                      |
| Function:  Reset the valid "signal" array                            |
|                                                                      |
| Returns:   Nothing                                                   |
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
  char derr[256];
  sprintf (derr, "ERROR [line: %d] %s\n", line, msg);
  perror(derr);
  exit (-1);
}
