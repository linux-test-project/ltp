/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

   Test that sigaddset() will add all defined signal numbers to a signal
   set.
   Test steps:
   1)  Initialize an empty signal set.
   For each signal number:
     2)  Add the signal to the empty signal set.
     3)  Verify that the signal is a member of the signal set.
 */
#include <stdio.h>
#include <signal.h>

#define SIGNALLISTSIZE 21

int main()
{
	sigset_t signalset;
	int signallist[SIGNALLISTSIZE];
	int i;
	int f=0;

	signallist[0] = SIGABRT;
	signallist[1] = SIGALRM;
	signallist[2] = SIGBUS;
	signallist[3] = SIGCHLD;
	signallist[4] = SIGCONT;
	signallist[5] = SIGFPE;
	signallist[6] = SIGHUP;
	signallist[7] = SIGILL;
	signallist[8] = SIGINT;
	signallist[9] = SIGKILL;
	signallist[10] = SIGPIPE;
	signallist[11] = SIGQUIT;
	signallist[12] = SIGSEGV;
	signallist[13] = SIGSTOP;
	signallist[14] = SIGTERM;
	signallist[15] = SIGTSTP;
	signallist[16] = SIGTTIN;
	signallist[17] = SIGTTOU;
	signallist[18] = SIGUSR1;
	signallist[19] = SIGUSR2;
	signallist[20] = SIGURG;
	if (sigemptyset(&signalset) == -1) {
		perror("sigemptyset failed -- test aborting\n");
		return -1;
	}

	for (i=0; i<SIGNALLISTSIZE; i++) {
		if (sigaddset(&signalset, signallist[i]) == 0) {
			if (sigismember(&signalset, signallist[i]) == 1) {
				// signal added
			} else {
				f=-1;
				printf("Signal - %d\n", signallist[i]);
			}
		} else {
			f=-1;
			printf("Signal - %d\n", signallist[i]);
		}
	}
	
	if (0 == f) {
		printf("All signals added\n");
		return 0;
	} else {
		printf("Some signals not added -- see above\n");
		return -1;
	}
}
