/*

 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by: rusty.lynch REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

  Test assertion #16 by verifying that select does not return -1 with 
  errno set to EINT if a handler for the SIGINT signal is setup with
  the SA_RESTART flag set.
 *
 * 12/18/02 - Adding in include of sys/time.h per
 *            rodrigc REMOVE-THIS AT attbi DOT com input that it needs
 *            to be included whenever the timeval struct is used.
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

int wakeup = 0;

void handler(int signo)
{
	printf("Caught SIGINT\n");
	wakeup++;
}

int main()
{
	int filedes[2];
	pid_t pid;
	char b;

	if (pipe(filedes) == -1) {
		perror("pipe");
		return -1;
	}

	if ((pid = fork()) == 0) {
		/* child */
		struct sigaction act;
		
		act.sa_handler = handler;
		act.sa_flags = SA_RESTART;
		sigemptyset(&act.sa_mask);
		sigaction(SIGINT,  &act, 0);     
		
		write(filedes[1], &b, 1); /* sync with parent */

		while(wakeup == 0) {
			struct timeval tv;
			
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			if (select(0, NULL, NULL, NULL, &tv)== -1 && 
			    errno == EINTR) {
				perror("select");
				return PTS_FAIL;
			}
		}
		
		return PTS_PASS;
	} else {
		/* parent */
		int s;
		
		read(filedes[0], &b, 1);  /* sync with child */

		kill(pid, SIGINT);
		waitpid(pid, &s, 0);

		if (!WEXITSTATUS(s)) {
			printf("Test PASSED\n");
			return PTS_PASS;
		}
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
}

