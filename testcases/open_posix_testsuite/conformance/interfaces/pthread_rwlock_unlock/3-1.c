/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  	If there are threads blocked on the lock when it becomes available,
 *	the scheduling policy shall determine which thread(s) shall acquire the lock.
 *	If the Thread Execution Scheduling option is supported, when threads executing
 *	with the scheduling policies SCHED_FIFO, SCHED_RR, or SCHED_SPORADIC are waiting
 *	on the lock, they shall acquire the lock in priority order when the lock
 *	becomes available. For equal priority threads, write locks shall take precedence
 *	over read locks.
 *
 Steps:
 * We have four threads, main(also a reader), writer1, reader, writer2
 * main has the highest priority, writer1 and reader has same priority, writer2 has lowest
 * priority.
 *
 * 1.  Main thread set its shcedule policy as "SCHED_FIFO", with highest priority
 *     the three: sched_get_priority_min()+3.
 * 2.  Main thread write lock 'rwlock'
 * 3.  Create a writer1 thread, with schedule policy as "SCHED_FIFO", and priority
 * 	using sched_get_priority_min()+2. The writer should block.
 * 4.  Create reader thread, with same priority as writer1. The reader should also block.
 * 5.  Create a writer2 thread, with priority sched_get_priority_min(). It should block
 *     on write lock too.
 * 7.  Main thread release the 'rwlock', make sure the threads got the lock in the order
 *     writer1, reader, writer2.
 *
 */

/* NOTE: The test result is UNSUPPORTED if Thread Execution Scheduling option
 * 	 is not supported.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include "posixtest.h"

#define TRD_POLICY SCHED_FIFO

static pthread_rwlock_t rwlock;
static int rd_thread_state;
static int wr_thread_state_1, wr_thread_state_2;

/* thread_state:
	1: not in child thread yet;
	2: just enter child thread ;
	3: after locking
	4: thread is exiting
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define PASSED_LOCK 3
#define EXITING_THREAD 4

static int set_priority(pthread_t pid, unsigned policy, unsigned prio)
{
	struct sched_param sched_param;
	memset(&sched_param, 0, sizeof(sched_param));
	sched_param.sched_priority = prio;
	if (pthread_setschedparam(pid, policy, &sched_param) == -1) {
		printf("Can't set policy to %d and prio to %d\n", policy, prio);
		exit(PTS_UNRESOLVED);
	}
	return 0;
}

static void *fn_rd(void *arg)
{
	int rc = 0;
	int priority;
	rd_thread_state = ENTERED_THREAD;

	priority = (long)arg;
	set_priority(pthread_self(), TRD_POLICY, priority);

	printf("reader: attempt read lock\n");
	rc = pthread_rwlock_rdlock(&rwlock);
	if (rc != 0) {
		printf
		    ("Error: rd_thread failed to get read lock, Error code:%d\n",
		     rc);
		exit(PTS_UNRESOLVED);
	}

	rd_thread_state = PASSED_LOCK;
	printf("reader: acquired read lock\n");

	/* Wait for main to wake us up */
	do {
		sleep(1);
	} while (rd_thread_state != EXITING_THREAD);

	printf("reader: unlock read lock\n");
	if (pthread_rwlock_unlock(&rwlock) != 0) {
		printf("rd_thread: Error at pthread_rwlock_unlock()\n");
		exit(PTS_UNRESOLVED);
	}
	pthread_exit(0);
	return NULL;
}

static void *fn_wr_1(void *arg)
{
	int rc = 0;
	int priority;
	wr_thread_state_1 = ENTERED_THREAD;

	priority = (int)(long)arg;
	set_priority(pthread_self(), TRD_POLICY, priority);

	printf("writer1: attempt write lock\n");
	rc = pthread_rwlock_wrlock(&rwlock);
	if (rc != 0) {
		printf
		    ("Error: wr_thread failed to get write lock, Error code:%d\n",
		     rc);
		exit(PTS_UNRESOLVED);
	}

	wr_thread_state_1 = PASSED_LOCK;
	printf("writer1: acquired write lock\n");

	/* Wait for main to wake us up */

	do {
		sleep(1);
	} while (wr_thread_state_1 != EXITING_THREAD);

	printf("writer1: unlock write lock\n");
	if (pthread_rwlock_unlock(&rwlock) != 0) {
		printf("wr_thread: Error at pthread_rwlock_unlock()\n");
		exit(PTS_UNRESOLVED);
	}

	pthread_exit(0);
	return NULL;
}

static void *fn_wr_2(void *arg)
{
	int rc = 0;
	int priority;
	wr_thread_state_2 = ENTERED_THREAD;

	priority = (long)arg;
	set_priority(pthread_self(), TRD_POLICY, priority);

	printf("writer2: attempt write lock\n");
	rc = pthread_rwlock_wrlock(&rwlock);
	if (rc != 0) {
		printf
		    ("Error: wr_thread failed to get write lock, Error code:%d\n",
		     rc);
		exit(PTS_UNRESOLVED);
	}

	wr_thread_state_2 = PASSED_LOCK;
	printf("writer2: acquired writer lock\n");

	/* Wait for main to wake us up */
	do {
		sleep(1);
	} while (wr_thread_state_2 != EXITING_THREAD);

	printf("writer2: unlock writer lock\n");
	if (pthread_rwlock_unlock(&rwlock) != 0) {
		printf("wr_thread: Error at pthread_rwlock_unlock()\n");
		exit(PTS_UNRESOLVED);
	}

	pthread_exit(0);
	return NULL;
}

int main(void)
{
#ifndef _POSIX_THREAD_PRIORITY_SCHEDULING
	printf("Posix Thread Execution Scheduling not supported\n");
	return PTS_UNSUPPORTED;
#endif

	int cnt = 0;
	pthread_t writer1, reader, writer2;
	int priority;

	/* main thread needs to have the highest priority */
	priority = sched_get_priority_min(TRD_POLICY) + 3;
	set_priority(pthread_self(), TRD_POLICY, priority);

	if (pthread_rwlock_init(&rwlock, NULL) != 0) {
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: write lock\n");
	/* We have no lock, this read lock should succeed */
	if (pthread_rwlock_wrlock(&rwlock) != 0) {
		printf
		    ("Error: main cannot get write lock when no one owns the lock\n");
		return PTS_UNRESOLVED;
	}

	/* Now create 3 threads that block on this rwlock */

	wr_thread_state_1 = NOT_CREATED_THREAD;
	priority = sched_get_priority_min(TRD_POLICY) + 2;
	printf("main: create writer1, with priority: %d\n", priority);
	if (pthread_create(&writer1, NULL, fn_wr_1, (void *)(long)priority) !=
	    0) {
		printf("main: Error creating writer1\n");
		return PTS_UNRESOLVED;
	}

	/* If the shared data is not altered by child after 2 seconds,
	   we regard it as blocked */

	/* We expect the writer1 to block */
	cnt = 0;
	do {
		sleep(1);
	} while (wr_thread_state_1 != 3 && cnt++ < 3);

	if (wr_thread_state_1 == 3) {
		printf
		    ("writer1 did not block on write lock, when main owns the lock\n");
		exit(PTS_UNRESOLVED);
	} else if (wr_thread_state_1 != 2) {
		printf("Unexpected writer1 state\n");
		exit(PTS_UNRESOLVED);
	}

	/* Reader thread same priority as Writer1 thread */

	rd_thread_state = 1;
	priority = sched_get_priority_min(TRD_POLICY) + 2;
	printf("main: create reader, with priority: %d\n", priority);
	if (pthread_create(&reader, NULL, fn_rd, (void *)(long)priority) != 0) {
		printf("main: failed at creating reader\n");
		return PTS_UNRESOLVED;
	}

	/* We expect the reader to block */
	cnt = 0;
	do {
		sleep(1);
	} while (rd_thread_state != 3 && cnt++ < 2);

	if (rd_thread_state == 3) {
		printf("Test Fail: reader did not block on read lock\n");
		exit(PTS_FAIL);
	} else if (rd_thread_state != 2) {
		printf("Unexpected reader state\n");
		exit(PTS_UNRESOLVED);
	}

	/* Writer2 is the lowest priority thread */

	wr_thread_state_2 = 1;
	priority = sched_get_priority_min(TRD_POLICY);
	printf("main: create writer2, with priority: %d\n", priority);
	if (pthread_create(&writer2, NULL, fn_wr_2, (void *)(long)priority) !=
	    0) {
		printf("main: Error creating writer2\n");
		return PTS_UNRESOLVED;
	}

	/* If the shared data is not altered by child after 3 seconds,
	   we regard it as blocked */

	/* We expect the writer2 to block */
	cnt = 0;
	do {
		sleep(1);
	} while (wr_thread_state_2 != 3 && cnt++ < 2);

	if (wr_thread_state_2 == 3) {
		printf
		    ("writer2 did not block on write lock, when main owns the lock\n");
		exit(PTS_UNRESOLVED);
	} else if (wr_thread_state_2 != 2) {
		printf("Unexpected writer1 state\n");
		exit(PTS_UNRESOLVED);
	}

	printf("main: release write lock\n");
	if (pthread_rwlock_unlock(&rwlock) != 0) {
		printf("main: failed to release write lock\n");
		exit(PTS_UNRESOLVED);
	}

	/* we expect writer1 get the lock */
	cnt = 0;
	do {
		sleep(1);
	} while (wr_thread_state_1 != 3 && cnt++ < 3);

	if (wr_thread_state_1 == 2) {
		printf
		    ("Test fail: writer did not get write lock, when main release the lock\n");
		exit(PTS_FAIL);
	} else if (wr_thread_state_1 != 3) {
		printf("Unexpected writer1 state\n");
		exit(PTS_UNRESOLVED);
	}

	/* Let writer1 release the lock */
	wr_thread_state_1 = 4;

	if (pthread_join(writer1, NULL) != 0) {
		printf("main: Error joining writer1\n");
		exit(PTS_UNRESOLVED);
	}

	/* we expect the reader get the lock when writer1 has release the lock */
	cnt = 0;
	do {
		sleep(1);
	} while (rd_thread_state != 3 && cnt++ < 3);

	if (rd_thread_state == 2) {
		printf
		    ("Test failed: reader did not get the lock when writer1 release the lock\n");
		exit(PTS_FAIL);
	} else if (rd_thread_state != 3) {
		printf("Unexpected reader state\n");
		exit(PTS_UNRESOLVED);
	}

	/* Inform reader release the lock */
	rd_thread_state = 4;

	if (pthread_join(reader, NULL) != 0) {
		printf("main: Error joining reader\n");
		exit(PTS_UNRESOLVED);
	}

	/* we expect writer2 get the lock */
	cnt = 0;
	do {
		sleep(1);
	} while (wr_thread_state_2 != 3 && cnt++ < 3);

	if (wr_thread_state_2 == 2) {
		printf
		    ("Test fail: writer2 still blocked on write lock, when main release the lock\n");
		exit(PTS_FAIL);
	} else if (wr_thread_state_2 != 3) {
		printf("Unexpected writer2 state\n");
		exit(PTS_UNRESOLVED);
	}

	wr_thread_state_2 = 4;

	if (pthread_join(writer2, NULL) != 0) {
		printf("main: Error joining writer1\n");
		exit(PTS_UNRESOLVED);
	}

	if (pthread_rwlock_destroy(&rwlock) != 0) {
		printf("Error at pthread_rwlockattr_destroy()");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
