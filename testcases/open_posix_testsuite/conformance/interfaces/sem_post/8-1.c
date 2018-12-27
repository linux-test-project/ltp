/*
 * Copyright (c) 2003-2004, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 * adam.li@intel.com 2004-03
 *
 * Cleaned up the code and uncommented the lock synchronization.
 * Cyril Hrubis <chrubis@suse.cz> 2011
 */

/*
 * If the Process Scheduling option is supported, the thread to be unblocked
 * shall be chosen in a manner appropriate to the scheduling policies
 * and parameters in effect for the blocked threads.
 * In the case of the schedulers SCHED_FIFO and SCHED_RR, the highest
 * priority waiting thread shall be unblocked, and if there is
 * more than one highest priority thread blocked waiting for the semaphore,
 * then the highest priority thread that has been waiting the
 * longest shall be unblocked.
 * If the Process Scheduling option is not defined, the choice of a thread
 * to unblock is unspecified.
 *
 * Test Steps:
 * Here we test SCHED_FIFO
 * 1. Parent locks a semaphore, it has highest priority P0.
 * 2. It forks 2 child processes 1, 2, as for priority P1, P2, P0 > P2 > P1
 * 3. The children lock the semaphore.
 *    Make sure the two children are waiting.
 * 4. Parent forks another child 3, with priority P3, P3 = P2, it locks
 *    the semaphore too.
 * 5. Parent unlocks the semaphore, make sure the children
 *    wake up in the order of  2 -> 3 -> 1
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "proc.h"

#define TEST "sem_post_8-1"

static char semname[28];
static char semname_1[28];	/* Used to record state */

int set_my_prio(int priority)
{
	struct sched_param sp;
	sp.sched_priority = priority;

	if (sched_setscheduler(0, SCHED_FIFO, &sp) == -1) {
		perror("sched_setscheduler()");
		return -1;
	}

	return 0;
}

int get_my_prio(void)
{
	struct sched_param sp;

	if (sched_getparam(0, &sp) == -1) {
		perror("sched_getparam()");
		return -1;
	}

	return sp.sched_priority;
}

int child_fn(int priority, int id)
{
	sem_t *sem, *sem_1;

	if (set_my_prio(priority))
		exit(-1);

	sem = sem_open(semname, 0);
	if (sem == SEM_FAILED) {
		perror("sem_open(semname)");
		exit(-1);
	}

	sem_1 = sem_open(semname_1, 0);
	if (sem_1 == SEM_FAILED) {
		perror("sem_open(semname_1)");
		exit(-1);
	}

	fprintf(stderr, "child %d try to get lock, prio: %d\n",
		id, get_my_prio());

	sem_wait(sem_1);

	if (sem_wait(sem) == -1) {
		perror("Error at sem_wait");
		fprintf(stderr, "Child %d: Cannot get lock", id);
		exit(-1);
	}

	fprintf(stderr, "child %d got lock\n", id);
	exit(0);
}

int main(void)
{
#ifndef _POSIX_PRIORITY_SCHEDULING
	printf("_POSIX_PRIORITY_SCHEDULING not defined\n");
	return PTS_UNTESTED;
#endif
	sem_t *sem, *sem_1;
	int val;
	int priority;
	pid_t c_1, c_2, c_3, ret_pid;
	int retval = PTS_UNRESOLVED;
	int status;
	struct timespec sync_wait_ts = {0, 100000};

	snprintf(semname, sizeof(semname), "/" TEST "_%d", getpid());

	sem = sem_open(semname, O_CREAT | O_EXCL, 0777, 1);
	if (sem == SEM_FAILED) {
		perror("sem_open(semname)");
		return PTS_UNRESOLVED;
	}

	snprintf(semname_1, sizeof(semname_1), "/" TEST "_%d_1", getpid());

	sem_1 = sem_open(semname_1, O_CREAT | O_EXCL, 0777, 3);
	if (sem_1 == SEM_FAILED) {
		perror("sem_open(semname_1)");
		sem_unlink(semname);
		return PTS_UNRESOLVED;
	}

	/* The parent has highest priority */
	priority = sched_get_priority_min(SCHED_FIFO) + 3;
	if (set_my_prio(priority) == -1) {
		retval = PTS_UNRESOLVED;
		goto clean_up;
	}

	/* Lock Semaphore */
	if (sem_wait(sem) == -1) {
		perror("sem_wait()");
		retval = PTS_UNRESOLVED;
		goto clean_up;
	}

	c_1 = fork();
	switch (c_1) {
	case 0:
		child_fn(priority - 2, 1);
		break;
	case -1:
		perror("fork()");
		retval = PTS_UNRESOLVED;
		goto clean_up;
		break;
	}
	fprintf(stderr, "P: child_1: %d forked\n", c_1);

	c_2 = fork();
	switch (c_2) {
	case 0:
		child_fn(priority - 1, 2);
		break;
	case -1:
		perror("fork()");
		retval = PTS_UNRESOLVED;
		goto clean_up;
		break;
	}
	fprintf(stderr, "P: child_2: %d forked\n", c_2);

	/* Make sure the two children has been waiting */
	do {
		nanosleep(&sync_wait_ts, NULL);
		sem_getvalue(sem_1, &val);
	} while (val != 1);
	tst_process_state_wait3(c_1, 'S', 2);
	tst_process_state_wait3(c_2, 'S', 2);

	c_3 = fork();
	switch (c_3) {
	case 0:
		child_fn(priority - 1, 3);
		break;
	case -1:
		perror("fork()");
		retval = PTS_UNRESOLVED;
		goto clean_up;
		break;
	}
	fprintf(stderr, "P: child_3: %d forked\n", c_3);

	/* Make sure child 3 has been waiting for the lock */
	do {
		nanosleep(&sync_wait_ts, NULL);
		sem_getvalue(sem_1, &val);
	} while (val != 0);
	tst_process_state_wait3(c_3, 'S', 2);

	/* Ok, let's release the lock */
	fprintf(stderr, "P: release lock\n");
	sem_post(sem);
	ret_pid = wait(&status);
	if (ret_pid == c_2 && WIFEXITED(status)
	    && WEXITSTATUS(status) == 0) {
		fprintf(stderr, "P: release lock\n");
		sem_post(sem);
		ret_pid = wait(&status);
		if (ret_pid == c_3 && WIFEXITED(status)
		    && WEXITSTATUS(status) == 0) {
			fprintf(stderr, "P: release lock\n");
			sem_post(sem);
			ret_pid = wait(&status);
			if (ret_pid == c_1 && WIFEXITED(status)
			    && WEXITSTATUS(status) == 0) {
				printf("Test PASSED\n");
				retval = PTS_PASS;
				goto clean_up;
			}
			printf("Test Fail: Expect child_1: %d, got %d\n",
			       c_1, ret_pid);
			retval = PTS_FAIL;
			goto clean_up;
		} else {
			printf("Test Fail: Expect child_3: %d, got %d\n",
			       c_3, ret_pid);
			retval = PTS_FAIL;
			sem_post(sem);
			while ((wait(NULL) > 0)) ;
			goto clean_up;
		}
	} else {
		printf("Test Fail: Expect child_2: %d, got %d\n", c_2, ret_pid);
		retval = PTS_FAIL;
		sem_post(sem);
		sem_post(sem);
		while ((wait(NULL) > 0)) ;
		goto clean_up;
	}

clean_up:
	sem_close(sem);
	sem_close(sem_1);
	sem_unlink(semname);
	sem_unlink(semname_1);
	return retval;
}
