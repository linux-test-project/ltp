/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 * Test that the process that is the head of the highest priority list preempt
 * the process calling sched_setparam() when the calling process sets its own
 * priority lower than that of one or more other non-empty process lists.
 *
 * There is no portable way to get the number of CPUs but the test should work
 * for most of UNIX system (including but not limited to: Linux, Solaris, AIX,
 * HPUX, *BSD).
 * This test used shared memory.
 * Steps:
 *   1. Create a share memory segment.
 *   2. Change the policy to SCHED_FIFO.
 *   3. Create nb_cpu children processes which shall have the same priority
 *      and policy than the father (cf specs of fork).
 *   4. Call sched_setparam with a priority smaller than those of children.
 *   5. Check if the shared value has been changed by a child process. If not,
 *      the test fail.
 */

#include <sched.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
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

static int nb_cpu;
static int *shmptr;

static int get_ncpu(void)
{
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

static void child_process(void)
{
	alarm(2);

	while (1) {
		(*shmptr)++;
		sched_yield();
	}
}

static void kill_children(int *child_pid)
{
	int i;

	for (i = 0; i < nb_cpu; i++) {
		kill(child_pid[i], SIGTERM);
	}
}

int main(void)
{
        int *child_pid, oldcount, newcount, shm_id, i, j;
	struct sched_param param;
	key_t key;

	nb_cpu = get_ncpu();
	
	if (nb_cpu == -1) {
		printf("Can not get the number of CPUs of your machines.\n");
		return PTS_UNRESOLVED;
	}

	child_pid = malloc(nb_cpu * sizeof(int));

	key = ftok("conformance/interfaces/sched_setparam/10-1.c",1234);
	shm_id = shmget(key, sizeof(int), IPC_CREAT|0600);
	if (shm_id < 0) {
		perror("An error occurs when calling shmget()");
		return PTS_UNRESOLVED;
	}

	shmptr = shmat(shm_id, 0, 0);
	if (shmptr == (void*)-1) {
		perror("An error occurs when calling shmat()");
		return PTS_UNRESOLVED;
	}
	*shmptr = 0;

	param.sched_priority = sched_get_priority_max(SCHED_FIFO);

	if (sched_setscheduler(getpid(), SCHED_FIFO, &param) != 0) {
		if (errno == EPERM) {
			printf("This process does not have the permission to set its own scheduling "
			       "parameter.\nTry to launch this test as root\n");
		} else {
			perror("An error occurs when calling sched_setscheduler()");
		}
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < nb_cpu; i++) {
		child_pid[i] = fork();
		if (child_pid[i] == -1) {
			perror("An error occurs when calling fork()");
			for (j=0; j<i; j++) {
				kill(child_pid[j], SIGTERM);
			}
			return PTS_UNRESOLVED;
		} else if (child_pid[i] == 0) {

			child_process();

			printf("This code should not be executed.\n");
			return PTS_UNRESOLVED;
		}
	}

	sleep(1);

	param.sched_priority--;

	oldcount = *shmptr;
	if (sched_setparam(getpid(), &param) != 0) {
		perror("An error occurs when calling sched_setparam()");
		kill_children(child_pid);
		return PTS_UNRESOLVED;
	}
	newcount = *shmptr;

	if (newcount == oldcount) {
		printf("The calling process does not relinquish the processor\n");
		kill_children(child_pid);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	kill_children(child_pid);
	return PTS_PASS;
}
