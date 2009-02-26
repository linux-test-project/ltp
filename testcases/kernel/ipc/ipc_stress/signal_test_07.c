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
|                            signal_test_07                            |
| ==================================================================== |
|                                                                      |
| Description:  Signal stress - send many signals to the process and   |
|               verify that it receives every one                      |
|                                                                      |
| Algorithm:    o  Block all signals from interrupting the process     |
|               o  Send MAX signals to the current process             |
|               o  Sleep for a short time                              |
|               o  Verify that signals are pending                     |
|               o  Change the signal mask and suspend execution until  |
|                  a signal interrupts the process                     |
|               o  Verify that at least one signal was received        |
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
| Usage:        signal_test_07                                         |
|                                                                      |
| To compile:   cc -o signal_test_07 signal_test_07                    |
|                                                                      |
| Last update:   Ver. 1.3, 7/7/94 16:18:19                           |
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/wait.h>
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

int signals_received = 0;

#define MAXSIG 	1024*1024		/* Max interrupts */
#define MAXTIME	2			/* Time out (minutes) */


/*---------------------------------------------------------------------+
|                               main ()                                |
| ==================================================================== |
|                                                                      |
| Function:  Main program  (see prolog for more details)               |
|                                                                      |
+---------------------------------------------------------------------*/
int main (int argc, char **argv)
{
	int	timeout = MAXTIME*60;	/* Timeout value */
	int	i;			/* Loop index */
	char 	msg [256];		/* Buffer for error message */

	/* Print out program header */
	printf ("%s: IPC TestSuite program\n\n", *argv);
	fflush (stdout);
   
	/* Set up our signal handler */
	init_sig ();

	/*
	 * Send MAXSIG signals to the process
	 *
	 * Using raise, send MAX signals to the process.  Then loop until
	 * every signal is caught by the signal handler (or the timer expires).
	 */
	printf ("\tSend MAX (%d) signals to the process...\n", MAXSIG);
	fflush (stdout);
	for (i=0; i<MAXSIG; i++)
		raise (SIGUSR1);

	while (signals_received < MAXSIG && --timeout)
		sleep (1);

	if (timeout == 0) {
		sprintf (msg, "failed to received %d signals in %d minutes\n",
			MAXSIG, MAXTIME);
		error (msg, __LINE__);
	}

	/*
	 * Received ALL of the sent signals!  Exit with success
	 */
	printf ("\n\tReceived EVERY signal!\n");

	printf ("\nsuccessful!\n");
	return (0);
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
	char msg [256];		/* Buffer for error message */

	if (signal == SIGUSR1) {
		signals_received++;
	} else if (signal == SIGUSR2) {
		printf ("\tcaught signal (%d)\n", signal);
	} else {
		sprintf (msg, "caught an unexpected signal (%d)", signal);
		error (msg, __LINE__);
	}
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
