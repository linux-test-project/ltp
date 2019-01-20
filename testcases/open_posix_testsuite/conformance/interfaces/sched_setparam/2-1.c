/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the higher numerical values for the priority represent the higher
 * priorities for the SCHED_FIFO policy.
 *
 * NO OTHER REALTIME PROCESS SHOULD RUN WHEN RUNNING THIS TEST.
 *
 * There is no portable way to get the number of CPUs but the test should work
 * for most of UNIX system (including but not limited to: Linux, Solaris, AIX,
 * HPUX, *BSD).
 * Steps:
 *  1. Get the number of CPUs.
 *  2. Set the scheduling policy to SCHED_FIFO with a mean priority.
 *  3. Lauch as many processes than CPU. The last process set its own priority
 *     to the min value.
 *  4. Father process set its own priority to the max value.
 *  5. Both children and father increment a counter in a basic loop.
 *  6. The father send SIGTERM to the last child and get its counter. If child
 *     counter is reasonably lower than the fathers one, the test is
 *     succesfull.
 *  7. The father kill all other children.
 *
 */
#include "affinity.h"

#include <sched.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"
#include "ncpu.h"

#define NB_LOOP         20000000
#define NB_LOOP_CHILD  200000000	/* shall be much greater than NB_LOOP */

#define ACCEPTABLE_RATIO 2.0

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int nb_child;			/* Number of child processes == number of CPUs */
int count = 0;
int the_pipe[2];

void child_process(int id)
{
	int i;
	struct sched_param param;

	if (id == nb_child - 1) {
		param.sched_priority = sched_get_priority_min(SCHED_FIFO);
		sched_setparam(getpid(), &param);
	}

	for (i = 0; i < NB_LOOP_CHILD; i++) {
		count++;
	}
}

void sigterm_handler(int signum LTP_ATTRIBUTE_UNUSED)
{
	close(STDOUT);
	close(the_pipe[0]);
	dup2(the_pipe[1], STDOUT);
	close(the_pipe[1]);

	printf("*%i*", count);
	fflush(stdout);

	exit(0);
}

int main(void)
{
	int child_count, i;
	struct sched_param param;
	int *child_pid;
	float ratio;

	/* Only use a single CPU and one child process
	   when set_affinity is availaible.It's because
	   no matter what value of the counter is set to,
	   There is no guarantee that the LOOP of the child
	   can be certainly big enough on any device at any time.
	 */
	int rc = set_affinity_single();
	if (rc) {
		nb_child = get_ncpu();
		if (nb_child == -1) {
			printf("Can not get the number of"
			       "CPUs of your machine.\n");
			return PTS_UNRESOLVED;
		}
	} else {
		nb_child = 1;
	}

	child_pid = malloc(nb_child * sizeof(int));

	param.sched_priority = (sched_get_priority_min(SCHED_FIFO) +
				sched_get_priority_max(SCHED_FIFO)) / 2;

	if (sched_setscheduler(getpid(), SCHED_FIFO, &param) == -1) {
		if (errno == EPERM) {
			printf
			    ("This process does not have the permission to set its own scheduling policy.\nTry to launch this test as root\n");
		} else {
			perror
			    ("An error occurs when calling sched_setscheduler()");
		}
		return PTS_UNRESOLVED;
	}

	if (signal(SIGTERM, sigterm_handler) == SIG_ERR) {
		perror("An error occurs when calling signal()");
		return PTS_UNRESOLVED;
	}

	pipe(the_pipe);

	for (i = 0; i < nb_child; i++) {
		child_pid[i] = fork();
		if (child_pid[i] == -1) {
			perror("An error occurs when calling fork()");
			return PTS_UNRESOLVED;
		} else if (child_pid[i] == 0) {
			child_process(i);

			printf("This code should not be executed.\n");
			return PTS_UNRESOLVED;
		}
	}

	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	if (sched_setparam(0, &param) != 0) {
		perror("An error occurs when calling sched_setparam()");
		return PTS_UNRESOLVED;
	}

	close(STDIN);
	close(the_pipe[1]);
	dup2(the_pipe[0], STDIN);
	close(the_pipe[0]);

	for (i = 0; i < NB_LOOP; i++) {
		count++;
	}

	if (kill(child_pid[nb_child - 1], SIGTERM) != 0) {
		perror("An error occurs when calling kill()");
		return PTS_UNRESOLVED;
	}

	param.sched_priority = sched_get_priority_min(SCHED_FIFO);
	if (sched_setparam(0, &param) != 0) {
		perror("An error occurs when calling sched_setparam()");
		return PTS_UNRESOLVED;
	}

	while (scanf("*%i*", &child_count) == 0)
		sched_yield();

	for (i = 0; i < nb_child - 1; i++) {
		if (kill(child_pid[i], SIGKILL) != 0) {
			perror("An error occurs when calling kill()");
			return PTS_UNRESOLVED;
		}
	}

	if (child_count)
		ratio = (float)count / (float)child_count;

	if (child_count == 0 || ratio >= ACCEPTABLE_RATIO) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (ratio <= (1 / ACCEPTABLE_RATIO)) {
		printf
		    ("Higher numerical values for the priority represent the lower priorities.\n");
		return PTS_FAIL;
	} else {
		printf
		    ("The difference between the processes is not representative.\n");
		return PTS_UNRESOLVED;
	}

}
