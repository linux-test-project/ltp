/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * This test use semaphore to protect critical section between several
 * processes.
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>

#include "posixtest.h"

#define SEM_NAME       "/tmp/semaphore"
#define BUF_SIZE	200
#define DEFAULT_THREADS 5

int main(int argc, char *argv[])
{
	sem_t *sem_lock;
	int shared = 1;
	int value = 1;
	pid_t pid = 0;
	int i, num = 0;
	char buf[BUF_SIZE];
	char *c;

#ifndef	_POSIX_SEMAPHORES
	printf("_POSIX_SEMAPHORES is not defined \n");
	return PTS_UNRESOLVED;
#endif
	if ((2 != argc) || ((num = atoi(argv[1])) <= 0)) {
		printf("Setting num_of_processes to default value: %d\n",
		       DEFAULT_THREADS);
		num = DEFAULT_THREADS;
	}
	if ((sem_lock = malloc(sizeof(sem_t))) == NULL) {
		perror("malloc");
		return PTS_UNRESOLVED;
	}
	if (sem_init(sem_lock, shared, value) == -1) {
		perror("sem_init didn't return success\n");
		return PTS_UNRESOLVED;
	}
	for (i = 1; i < num; i++)
		switch ((pid = fork())) {
		case -1:
			perror("fork");
			return PTS_UNRESOLVED;
		case 0:
			break;
		default:
			sleep(2);
			break;
		}

	sprintf(buf, "%d process_ID: %ld parent_process_ID: %ld "
		"child_process_ID: %ld\n", i, (long)getpid(),
		(long)getppid(), (long)pid);

	if (sem_wait(sem_lock) == -1) {
		perror("sem_wait didn't return success\n");
		return PTS_UNRESOLVED;
	}
	for (i = 1; i <= 10; i++) {
		c = buf;
		while (*c != '\n') {
			fputc(*c, stdout);
			c++;
		}
		fputc('\n', stdout);
	}

	if (sem_post(sem_lock) == -1) {
		perror("sem_wait didn't return success\n");
		return PTS_UNRESOLVED;
	}

	return PTS_PASS;
}
