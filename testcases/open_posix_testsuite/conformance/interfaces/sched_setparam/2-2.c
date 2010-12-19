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
 * priorities for the SCHED_RR policy.
 *
 * NO OTHER REALTIME PROCESS SHOULD RUN WHEN RUNNING THIS TEST.
 *
 * There is no portable way to get the number of CPUs but the test should work
 * for most of UNIX system (including but not limited to: Linux, Solaris, AIX,
 * HPUX, *BSD).
 * Steps:
 *  1. Get the number of CPUs.
 *  2. Set the scheduling policy to SCHED_RR with a mean priority.
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

#include <sched.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

#ifdef BSD
# include <sys/types.h>
# include <sys/param.h>
# include <sys/sysctl.h>
#endif

#ifdef HPUX
# include <sys/param.h>
# include <sys/pstat.h>
#endif

#define NB_LOOP         20000000
#define NB_LOOP_CHILD  200000000 /* shall be much greater than NB_LOOP */

#define ACCEPTABLE_RATIO 2.0

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int nb_child;   /* Number of child processes == number of CPUs */
int count = 0;
int the_pipe[2];

/* Get the number of CPUs */
int get_ncpu() {
	int ncpu = -1;

	/* This syscall is not POSIX but it should work on many system */
#ifdef _SC_NPROCESSORS_ONLN
	ncpu = sysconf(_SC_NPROCESSORS_ONLN);
#else
# ifdef BSD
	int mib[2];
	size_t len = sizeof(ncpu);
	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	sysctl(mib, 2, &ncpu, &len, NULL, 0);
# else
#  ifdef HPUX
	struct pst_dynamic psd;
	pstat_getdynamic(&psd, sizeof(psd), 1, 0);
	ncpu = (int)psd.psd_proc_cnt;
#  endif /* HPUX */
# endif /* BSD */
#endif /* _SC_NPROCESSORS_ONLN */

	return ncpu;
}

void child_process(int id) {
	int i;
	struct sched_param param;

	if (id == nb_child-1) {
		param.sched_priority = sched_get_priority_min(SCHED_RR);
		sched_setparam(getpid(), &param);
	}

	for (i=0; i<NB_LOOP_CHILD; i++) {
		count ++;
	}
}

void sigterm_handler(int signum) {
	close(STDOUT);
	close(the_pipe[0]);
	dup2(the_pipe[1],STDOUT);
	close(the_pipe[1]);

	printf("*%i*", count);
	fflush(stdout);

	exit(0);
}

int main() {
        int child_count, i;
	struct sched_param param;
	int *child_pid;
	float ratio;

	nb_child = get_ncpu();
	if (nb_child == -1) {
		printf("Can not get the number of CPUs of your machine.\n");
		return PTS_UNRESOLVED;
	}
	child_pid = malloc(nb_child);
	if (child_pid == NULL) {
		printf("malloc failed\n");
		return PTS_UNRESOLVED;
	}
	param.sched_priority = (sched_get_priority_min(SCHED_RR) +
				 sched_get_priority_max(SCHED_RR)) / 2;

	if (sched_setscheduler(getpid(), SCHED_RR, &param) == -1) {
		if (errno == EPERM) {
			printf("This process does not have the permission to set its own scheduling policy.\nTry to launch this test as root\n");
		} else {
			perror("An error occurs when calling sched_setscheduler()");
		}
		return PTS_UNRESOLVED;
	}

	if (signal(SIGTERM, sigterm_handler) == SIG_ERR) {
		perror("An error occurs when calling signal()");
		return PTS_UNRESOLVED;
        }

	pipe(the_pipe);

	for (i=0; i<nb_child; i++) {
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

	param.sched_priority = sched_get_priority_max(SCHED_RR);
	if (sched_setparam(0, &param) != 0) {
		perror("An error occurs when calling sched_setparam()");
		return PTS_UNRESOLVED;
	}

	close(STDIN);
	close(the_pipe[1]);
	dup2(the_pipe[0],STDIN);
	close(the_pipe[0]);

	for (i=0; i<NB_LOOP; i++) {
		count++;
	}

	if (kill(child_pid[nb_child-1], SIGTERM) != 0) {
		perror("An error occurs when calling kill()");
		return PTS_UNRESOLVED;
	}

	param.sched_priority = sched_get_priority_min(SCHED_RR);
	if (sched_setparam(0, &param) != 0) {
		perror("An error occurs when calling sched_setparam()");
		return PTS_UNRESOLVED;
	}

	while (scanf("*%i*",&child_count) == 0)
		sched_yield();

	for (i = 0; i < (nb_child-1); i++) {
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
	} else if (ratio <= (1/ACCEPTABLE_RATIO)) {
		printf("Higher numerical values for the priority represent the lower priorities.\n");
		return PTS_FAIL;
	} else {
		printf("The difference between the processes is not representative.\n");
		return PTS_UNRESOLVED;
	}

}