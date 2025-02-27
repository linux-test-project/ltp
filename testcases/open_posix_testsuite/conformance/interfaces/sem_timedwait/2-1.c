/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/* sem_timedwait will return successfully when sem_post
 * will unlock the semaphore from another process.
 */


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include "posixtest.h"
#include "proc.h"

#define TEST "2-1"
#define SHM_NAME "/posixtest_2-1"
#define FUNCTION "sem_timedwait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t *mysemp;
	struct timespec ts;
	int fd, pid;

	fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror(ERROR_PREFIX "shm_open()");
		return PTS_UNRESOLVED;
	}

	if (ftruncate(fd, sizeof(*mysemp)) == -1) {
		perror(ERROR_PREFIX "ftruncate()");
		return PTS_UNRESOLVED;
	}

	if (shm_unlink(SHM_NAME) != 0) {
		perror(ERROR_PREFIX "shm_unlink()");
		return PTS_UNRESOLVED;
	}

	mysemp = mmap(NULL, sizeof(*mysemp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (mysemp == MAP_FAILED) {
		perror(ERROR_PREFIX "mmap");
		return PTS_UNRESOLVED;
	}

	/* Semaphore started out locked */
	if (sem_init(mysemp, 1, 0) == -1) {
		perror(ERROR_PREFIX "sem_init");
		return PTS_UNRESOLVED;
	}

	pid = fork();
	if (pid == 0)		// child to lock semaphore
	{
		ts.tv_sec = time(NULL) + 2;
		ts.tv_nsec = 0;

		if (sem_timedwait(mysemp, &ts) == -1) {
			puts("TEST FAILED");
			return PTS_FAIL;
		} else {
			puts("TEST PASSED");
			sem_destroy(mysemp);
			return PTS_PASS;
		}
	} else if (pid > 0)	// parent to unlock semaphore
	{
		int i;

		tst_process_state_wait3(pid, 'S', 1);
		if (sem_post(mysemp) == -1) {
			perror(ERROR_PREFIX "sem_post");
			return PTS_FAIL;
		}
		if (wait(&i) == -1) {
			perror("Error waiting for child to exit");
			return PTS_UNRESOLVED;
		}

		if (!WIFEXITED(i) || WEXITSTATUS(i)) {
			return PTS_FAIL;
		}
		puts("TEST PASSED");
		sem_destroy(mysemp);
		if (munmap(mysemp, sizeof(*mysemp)) == -1) {
			perror(ERROR_PREFIX "munmap");
			return PTS_UNRESOLVED;
		}
		return PTS_PASS;
	}

	close(fd);
	return PTS_UNRESOLVED;
}
