/*
  Test assertion #11 by verifying that SIGCHLD signals are sent to a parent
  when their children are continued after being stopped.

  NOTE: This is only required to work if the XSI options are implemented.
 * 12/18/02 - Adding in include of sys/time.h per
 *            rodrigc REMOVE-THIS AT attbi DOT com input that it needs
 *            to be included whenever the timeval struct is used.
 *
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>

#define NUMSTOPS 2

int child_continued = 0;
int waiting = 1;

void handler(int signo, siginfo_t *info, void *context) 
{
	if (info && info->si_code == CLD_CONTINUED) {
		printf("Child has been stopped\n");
		waiting = 0;
		child_continued++;
	}
}


int main()
{
	pid_t pid;
	struct sigaction act;
	struct timeval tv;

	act.sa_sigaction = handler;
	act.sa_flags = SA_SIGINFO;
	sigemptyset(&act.sa_mask);
	sigaction(SIGCHLD,  &act, 0);     

	if ((pid = fork()) == 0) {
		/* child */
		while(1) {
			/* wait forever, or until we are 
			   interrupted by a signal */
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			select(0, NULL, NULL, NULL, &tv);
		}
		return 0;
	} else {
		/* parent */
		int s; 		
		int i;

		/* delay to allow child to get into select call */
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		select(0, NULL, NULL, NULL, &tv);

		for (i = 0; i < NUMSTOPS; i++) {
			struct timeval tv; 
			printf("--> Sending SIGSTOP\n");
			kill(pid, SIGSTOP);

			/*
			  Don't let the kernel optimize away queued
			  SIGSTOP/SIGCONT signals.
			*/
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			select(0, NULL, NULL, NULL, &tv);

			printf("--> Sending SIGCONT\n");
			waiting = 1;
			kill(pid, SIGCONT);
			while (waiting) {
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				if (!select(0, NULL, NULL, NULL, &tv))
					break;
			}

		}
		
		/* POSIX specifies default action to be abnormal termination */
		kill(pid, SIGHUP);
		waitpid(pid, &s, 0);
	}

	if (child_continued == NUMSTOPS) {
		printf("Test PASSED\n");
		return 0;
	}

	printf("Test FAILED (%i Continues)\n", child_continued);
	return -1;
}

