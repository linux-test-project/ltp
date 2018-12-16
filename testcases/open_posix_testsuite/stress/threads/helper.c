/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
 * Created by: Sebastien Decugis

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 */

/*
 * The purpose of this file is to provide a monitor process
 * for the stress/threads/ * / *.c testcases in the OPTS.
 *
 * The goal is:
 * -> if the testcase returns, the monitor returns the error code.
 * -> after a specified timeout, the monitor let the stress test terminate

 * This allows for the stress tests to be run in an automatic maneer
 * with a script such as:
#!/bin/sh

#monitor the system
vmstat -n 120 180 >monitor.txt 2>&1 &

#run the tests
for TS in `ls -1 *.c`;
do <compile $TS>;
if [ $? -eq 0 ];
then <run in background:>
     helper 6 $TS.exe &
fi
done

#wait for the end
while [ "`ps -e | grep helper`" ];
do sleep 30;
done

*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <assert.h>
#include <time.h>

pid_t child;
int timeout;

/* Note that there could be a race between
the moment the stress test terminates and
when the timeout expires. As this is highly
improbable, we don't care... */

void *timer(void *arg)
{
	int ret = 0;

	unsigned remaining = timeout * 3600;
	do {
		remaining = sleep(remaining);
	} while (remaining);
	ret = kill(child, SIGUSR1);
	if (ret != 0) {
		perror("Failed to kill the stress test");
		exit(2);
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int ret;
	pthread_t th;
	pid_t chk;
	int status;
	char *ts = "[??:??:??]";
	struct tm *now;
	time_t nw;

	/* check args */
	if (argc < 3) {
		printf("\nUsage: \n");
		printf("  $ %s n exe arglist\n", argv[0]);
		printf("\nWhere:\n");
		printf("  n       is the timeout duration in hours,\n");
		printf("  exe     is the stress test executable to monitor,\n");
		printf
		    ("  arglist is the arguments to be passed to executable.\n\n");
		return 2;
	}

	timeout = atoi(argv[1]);
	if (timeout < 1) {
		fprintf(stderr,
			"Invalid timeout value \"%s\". Timeout must be a positive integer.\n",
			argv[1]);
		return 2;
	}

	/* create the timer thread */
	ret = pthread_create(&th, NULL, timer, NULL);
	if (ret != 0) {
		perror("Failed to create the timeout thread\n");
		return 2;
	}

	/* Create the new process for the stress test */
	child = fork();

	if (child == (pid_t) - 1) {
		perror("Failed to create a new process");
		exit(2);
	}

	/* The child process executes the test */
	if (child == (pid_t) 0) {

		/* Execute the command */
		ret = execvp(argv[2], &argv[2]);
		if (ret == -1) {
			/* Application was not launched */
			perror("Unable to run child application");
			return 2;
		}
		assert(0);
		perror("Should not see me");
		return 2;
	}

	/* The parent: */

	/* wait for the child process to terminate */
	chk = waitpid(child, &status, 0);
	if (chk != child) {
		perror("Got the wrong process image status");
		return 2;
	}

	/* Cancel the timer thread in case the process returned by itself */
	(void)pthread_cancel(th);

	ret = pthread_join(th, NULL);
	if (ret != 0) {
		perror("Unable to join the timer thread");
		return 2;
	}

	/* return */
	nw = time(NULL);
	now = localtime(&nw);
	if (now == NULL)
		printf(ts);
	else
		printf("[%2.2d:%2.2d:%2.2d]", now->tm_hour, now->tm_min,
		       now->tm_sec);
	if (!WIFEXITED(status)) {
		printf("The stress sample did not exit\n");
		if (WIFSIGNALED(status)) {
			printf("It was killed with signal %i\n",
			       WTERMSIG(status));
		} else {
			printf("and it was not killed...\n");
		}
		exit(1);
	}
	if (WEXITSTATUS(status) == 0) {
		printf("Test %s PASSED\n", argv[2]);
	} else {
		printf("Test %s: returned %d\n", argv[2], WEXITSTATUS(status));
	}
	exit(WEXITSTATUS(status));
}
