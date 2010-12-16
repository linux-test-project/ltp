/*
 * Copyright (c) 2003-2004, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 * adam.li@intel.com 2004-03
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

#define TEST "8-1"
#define FUNCTION "sem_wait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

static char semname[28];
static char semname_1[28]; /* Used to record state */

/* set my schedule priority to @priority */
int set_my_prio(int priority)
{
	struct sched_param sp;
	sp.sched_priority = priority;

	/* Set priority */
	if (sched_setscheduler(0, SCHED_FIFO, &sp) == -1)
	{
		perror("Error at sched_setscheduler()\n");
		return -1;
	}
	return 0;
}

int get_my_prio()
{
	struct sched_param sp;
	if (sched_getparam(0, &sp) == -1)
	{
		perror("Error at sched_getparam()\n");
		return -1;
	}
	return sp.sched_priority;
}

int child_fn(int priority, int id)
{
	sem_t *sem, *sem_1;
	if (set_my_prio(priority) == -1)
		exit(-1);
	sem = sem_open(semname, 0);
	if (sem == SEM_FAILED || sem == NULL) {
		perror(ERROR_PREFIX "sem_open: sem");
		return -1;
	}
	sem_1 = sem_open(semname_1, 0);
	if (sem_1 == SEM_FAILED || sem_1 == NULL) {
		perror(ERROR_PREFIX "sem_open: sem_1");
		return -1;
	}
	sem_wait(sem_1);
	fprintf(stderr, "child %d try to get lock, prio: %d\n",
			id, get_my_prio());
	if (sem_wait(sem) == -1)
	{
		perror("Error at sem_wait");
		fprintf(stderr, "Child %d: Cannot get lock", id);
		exit(-1);
	}
	fprintf(stderr, "child %d got lock\n", id);
	exit(0);
}

int main()
{
#ifndef _POSIX_PRIORITY_SCHEDULING
	printf("_POSIX_PRIORITY_SCHEDULING not defined\n");
	return PTS_UNTESTED;
#endif
	sem_t *sem, *sem_1;
	int val = 3; /* for sem_1 to track the child state */
	int priority;
	pid_t c_1, c_2, c_3, ret_pid;
	int retval = PTS_UNRESOLVED;
	int status;

	snprintf(semname, 20, "/" TEST "_%d", getpid());
	/* Initial value of Semaphore is 1 */
	sem = sem_open(semname, O_CREAT, 0777, 1);
	if (sem == SEM_FAILED || sem == NULL) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	snprintf(semname_1, 20, "/" TEST "_%d_1", getpid());
	sem_1 = sem_open(semname_1, O_CREAT, 0777, val);
	if (sem_1 == SEM_FAILED || sem_1 == NULL) {
		perror(ERROR_PREFIX "sem_open: sem_1");
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
		perror(ERROR_PREFIX "sem_wait");
		retval = PTS_UNRESOLVED;
		goto clean_up;
	}

	c_1 = fork();
	if (c_1 == 0)
	{
		/* Child 1, should block */
		child_fn(priority - 2, 1);
	}
	else if (c_1 < 0)
	{
		perror("Error at fork");
		retval = PTS_UNRESOLVED;
		goto clean_up;
	}
	fprintf(stderr, "P: child_1:%d forked\n", c_1);

	c_2 = fork();
	if (c_2 == 0)
	{
		/* Child 2 */
		child_fn(priority - 1, 2);
	}
	else if (c_2 < 0)
	{
		perror("Error at fork");
		retval = PTS_UNRESOLVED;
		goto clean_up;
	}
	fprintf(stderr, "P: child_2: %d forked\n", c_2);

	/* Make sure the two children has been waiting */
	/*do {
		sleep(1);
		sem_getvalue(sem_1, &val);
		//printf("val = %d\n", val);
	} while (val != 1);
	*/
	c_3 = fork();
	if (c_3 == 0)
	{
		/* Child 3 */
		child_fn(priority - 1, 3);
	}
	fprintf(stderr, "P: child_3: %d forked\n", c_3);

	/* Make sure child 3 has been waiting for the lock */
	/*do {
		sleep(1);
		sem_getvalue(sem_1, &val);
		//printf("val = %d\n", val);
	} while (val != 0);
	*/

	/* Synchronization required before release the lock */
	sleep(1);
	/* Ok, let's release the lock */
	fprintf(stderr, "P: release lock\n");
	sem_post(sem);
	ret_pid = wait(&status);
	if (ret_pid == c_2 && WIFEXITED(status)
				&& WEXITSTATUS(status) == 0)
	{
		fprintf(stderr, "P: release lock\n");
		sem_post(sem);
		ret_pid = wait(&status);
		if (ret_pid == c_3 && WIFEXITED(status)
				&& WEXITSTATUS(status) == 0)
		{
			fprintf(stderr, "P: release lock\n");
			sem_post(sem);
			ret_pid = wait(&status);
			if (ret_pid == c_1 && WIFEXITED(status)
				&& WEXITSTATUS(status) == 0)
			{
				printf("Test Pass\n");
				retval = PTS_PASS;
				goto clean_up;
			}
			printf("Test Fail: Expect child_1: %d, got %d\n",
				c_1, ret_pid);
			retval = PTS_FAIL;
			goto clean_up;
		}
		else
		{
			printf("Test Fail: Expect child_3: %d, got %d\n",
			c_3, ret_pid);
			retval = PTS_FAIL;
			sem_post(sem);
			while ((wait(NULL) > 0));
			goto clean_up;
		}
	}
	else
	{
		printf("Test Fail: Expect child_2: %d, got %d\n",
			c_2, ret_pid);
		retval = PTS_FAIL;
		sem_post(sem);
		sem_post(sem);
		while ((wait(NULL) > 0));
		goto clean_up;
	}

clean_up:
	sem_close(sem);
	sem_close(sem_1);
	sem_unlink(semname);
	sem_unlink(semname_1);
	return retval;
}