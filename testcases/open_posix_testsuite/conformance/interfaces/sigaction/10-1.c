/*
  Test assertion #10 by verifying that SIGCHLD signals are sent to a parent
  when their children are stopped.
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
#include <sys/types.h>
#include <unistd.h>

#define NUMSTOPS 10

int child_stopped = 0;
int waiting = 1;

void handler(int signo, siginfo_t *info, void *context) 
{
	if (info && info->si_code == CLD_STOPPED) {
		printf("Child has been stopped\n");
		waiting = 0;
		child_stopped++;
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

		for (i = 0; i < NUMSTOPS; i++) {
			waiting = 1;

			printf("--> Sending SIGSTOP\n");
			kill(pid, SIGSTOP);

			/*
			  Don't let the kernel optimize away queued
			  SIGSTOP/SIGCONT signals.
			*/
			while (waiting) {
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				if (!select(0, NULL, NULL, NULL, &tv))
				  break;
			}


			printf("--> Sending SIGCONT\n");
			kill(pid, SIGCONT);
		}
		
		/* POSIX specifies default action to be abnormal termination */
		kill(pid, SIGHUP);
		waitpid(pid, &s, 0);
	}

	if (child_stopped == NUMSTOPS) {
		printf("Test PASSED\n");
		return 0;
	}

	printf("Test FAILED\n");
	return -1;
}

