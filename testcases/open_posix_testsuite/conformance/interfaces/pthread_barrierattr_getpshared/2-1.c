/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The idea of this case comes from GNU C library NPTL test tst-barrier2.c.
 *
 * The process-shared attribute is set to PTHREAD_PROCESS_SHARED to permit
 * a barrier to be operated upon by any thread that has access to the memory
 * where the barrier is allocated. If the process-shared attribute
 * is PTHREAD_PROCESS_PRIVATE, the barrier shall only be operated
 * upon by threads created within the same process as the thread
 * that initialized the barrier; if threads of different processes attempt
 * to operate on such a barrier, the behavior is undefined.
 * The default value of the attribute shall be PTHREAD_PROCESS_PRIVATE. Both constants
 * PTHREAD_PROCESS_SHARED and PTHREAD_PROCESS_PRIVATE are defined in <pthread.h>.
 *
 * steps:
 *	1. Create a piece of shared memory object, create pthread barrier object 'barrier'
 *	   and set the PTHREAD_PROCESS_SHARED attribute.
 *	2. Parent map the shared memory to its memory space, put 'barrier' into it;
 *	3. Parent fork to create child process;
 *	4. Child process map the 'barrier' to its memory space;
 *	5. Parent and Child execute same code: loop N times, calling pthread_barrier_wait()
 *	6. Parent and Child should not block on pthread_barrier_wait()
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include "posixtest.h"

#define LOOP_NUM 10

void sig_handler()
{
	printf("Interrupted by SIGALRM\n");
	printf("Test Fail: block on pthread_barrier_wait()\n");
	exit(PTS_FAIL);
}

int main(void)
{

	/* Make sure there is process-shared capability. */
#ifndef PTHREAD_PROCESS_SHARED
	fprintf(stderr,
		"process-shared attribute is not available for testing\n");
	return PTS_UNSUPPORTED;
#endif

	static pthread_barrier_t *barrier;
	pthread_barrierattr_t ba;
	int pshared = PTHREAD_PROCESS_SHARED;

	char shm_name[] = "tmp_pthread_barrierattr_getpshared";
	int shm_fd;
	int pid;
	int loop;
	int serial = 0;
	int rc;
	int status = 0;
	struct sigaction act;

	/* Set up parent to handle SIGALRM */
	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	sigfillset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);

	/* Initialize a barrier attributes object */
	if (pthread_barrierattr_init(&ba) != 0) {
		printf("Error at pthread_barrierattr_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Set the pshard value to private to shared */
	if (pthread_barrierattr_setpshared(&ba, pshared) != 0) {
		printf("Error at pthread_barrierattr_setpshared()\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_barrierattr_getpshared(&ba, &pshared) != 0) {
		printf
		    ("Test FAILED: Error at pthread_barrierattr_getpshared()\n");
		return PTS_FAIL;
	}

	if (pshared != PTHREAD_PROCESS_SHARED) {
		printf("Test FAILED: Incorrect pshared value %d\n", pshared);
		return PTS_FAIL;
	}

	/* Create shared object */
	shm_unlink(shm_name);
	shm_fd =
	    shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (shm_fd == -1) {
		perror("Error at shm_open()");
		return PTS_UNRESOLVED;
	}

	if (ftruncate(shm_fd, sizeof(pthread_barrier_t)) != 0) {
		perror("Error at ftruncate()");
		shm_unlink(shm_name);
		return PTS_UNRESOLVED;
	}

	/* Map the shared memory object to my memory */
	barrier = mmap(NULL, sizeof(pthread_barrier_t), PROT_READ | PROT_WRITE,
		       MAP_SHARED, shm_fd, 0);

	if (barrier == MAP_FAILED) {
		perror("Error at first mmap()");
		shm_unlink(shm_name);
		return PTS_UNRESOLVED;
	}

	/* Initialize a barrier */
	if ((pthread_barrier_init(barrier, &ba, 2)) != 0) {
		printf("Error at pthread_barrier_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Cleanup */
	if ((pthread_barrierattr_destroy(&ba)) != 0) {
		printf("Error at pthread_barrierattr_destroy()\n");
		return PTS_UNRESOLVED;
	}

	/* Fork a child process */
	pid = fork();
	if (pid == -1) {
		perror("Error at fork()");
		return PTS_UNRESOLVED;
	} else if (pid == 0) {
		/* Child */
		/* Map the shared object to child's memory */
		barrier =
		    mmap(NULL, sizeof(pthread_barrier_t),
			 PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

		if (barrier == MAP_FAILED) {
			perror("child: Error at first mmap()");
			return PTS_UNRESOLVED;
		}
	} else {
		printf("parent pid : %d, child pid : %d\n", getpid(), pid);
		printf
		    ("parent: send me SIGALRM 2 secs later in case I am blocked\n");
		alarm(2);
	}

	for (loop = 0; loop < LOOP_NUM; loop++) {
		rc = pthread_barrier_wait(barrier);
		if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
			printf
			    ("Test FAILED: %d: pthread_barrier_wait() got unexpected "
			     "return code : %d\n", getpid(), rc);
			exit(PTS_FAIL);
		} else if (rc == PTHREAD_BARRIER_SERIAL_THREAD) {
			serial++;
			printf
			    ("process %d: get PTHREAD_BARRIER_SERIAL_THREAD\n",
			     getpid());
		}

	}

	if (pid > 0) {
		/* parent */
		if (wait(&status) != pid) {
			printf("parent: error at waitpid()\n");
			return PTS_UNRESOLVED;
		}

		if (!WIFEXITED(status)) {
			printf("Child exited abnormally\n");
			return PTS_UNRESOLVED;
		}

		if ((WEXITSTATUS(status) + serial) != LOOP_NUM) {
			printf("status = %d\n", status);
			printf("serial = %d\n", serial);
			printf
			    ("Test FAILED: One of the two processes should get "
			     "PTHREAD_BARRIER_SERIAL_THREAD\n");
			return PTS_FAIL;
		}

		/* Cleanup */
		if (pthread_barrier_destroy(barrier) != 0) {
			printf("Error at pthread_barrier_destroy()");
			return PTS_UNRESOLVED;
		}

		if ((shm_unlink(shm_name)) != 0) {
			perror("Error at shm_unlink()");
			return PTS_UNRESOLVED;
		}

		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (pid == 0) {
		exit(serial);
	}

}
