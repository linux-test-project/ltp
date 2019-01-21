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
#include "posixtest.h"

#define NUMSTOPS 10

static volatile int child_stopped;
static volatile int child_continued;
static volatile int notification;

void handler(int signo LTP_ATTRIBUTE_UNUSED, siginfo_t *info,
	void *context LTP_ATTRIBUTE_UNUSED)
{
	if (!info)
		return;

	notification = info->si_code;

	switch (notification) {
	case CLD_STOPPED:
		printf("Child has been stopped\n");
		child_stopped++;
		break;
	case CLD_CONTINUED:
		printf("Child has been continued\n");
		child_continued++;
		break;
	}
}

void wait_for_notification(int val)
{
	struct timeval tv;

	while (notification != val) {
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		if (!select(0, NULL, NULL, NULL, &tv))
			break;
	}
}

int main(void)
{
	pid_t pid;
	struct sigaction act;
	struct timeval tv;

	act.sa_sigaction = handler;
	act.sa_flags = SA_SIGINFO;
	sigemptyset(&act.sa_mask);
	sigaction(SIGCHLD, &act, 0);

	if ((pid = fork()) == 0) {
		/* child */
		while (1) {
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
			printf("--> Sending SIGSTOP\n");
			notification = 0;
			kill(pid, SIGSTOP);

			/*
			   Don't let the kernel optimize away queued
			   SIGSTOP/SIGCONT signals.
			 */

			wait_for_notification(CLD_STOPPED);

			printf("--> Sending SIGCONT\n");
			notification = 0;
			kill(pid, SIGCONT);
			/*
			   SIGCHLD doesn't queue, make sure CLD_CONTINUED
			   doesn't mask the next CLD_STOPPED
			 */
			wait_for_notification(CLD_CONTINUED);
		}

		kill(pid, SIGKILL);
		waitpid(pid, &s, 0);
	}

	if (child_stopped == NUMSTOPS && child_continued == NUMSTOPS) {
		printf("Test PASSED\n");
		return 0;
	}

	printf("Test FAILED\n");
	return -1;
}
