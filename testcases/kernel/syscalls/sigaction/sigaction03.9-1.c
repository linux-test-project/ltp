/*
  Test assertion #9 by verifying that SIGCHLD signals are not sent when 
  the parent has setup a SIGCHLD signal handler with the SA_NOCLDSTOP flag set
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
	act.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
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
			  If we send a bunch of SIGSTOP/SIGCONT
			  signals one after the other then it is 
			  perfectly OK for the OS to not send
			  the SIGSTOP/SIGCONT combination as an 
			  optimization.

			  I can't think of any POSIX method to determine
			  if a process has been stopped, so I'm 
			  going to punt with a one second sleep and
			  assume the child process gets put to sleep
			  within that time period.  This will be problem
			  when this test is run on a really stressed
			  system. (Although since we are sending multiple 
			  SIGSTOP's then maybe in practice this will
			  cause any problems.)
			*/
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			select(0, NULL, NULL, NULL, &tv);

			printf("--> Sending SIGCONT\n");
			kill(pid, SIGCONT);
		}
		
		/* POSIX specifies default action to be abnormal termination */
		kill(pid, SIGHUP);
		waitpid(pid, &s, 0);
	}

	if (child_stopped == 0) {
		printf("Test PASSED\n");
		return 0;
	}

	printf("Test FAILED\n");
	return -1;
}

