
/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * This is a test about producer and consumer. Producer sends data
 * to a buffer. Consumer keeps reading data from the buffer.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "posixtest.h"

#define BUF_SIZE	5
#define Max_Num		10

typedef struct {
	int buffer[BUF_SIZE];
	sem_t occupied;
	sem_t empty;
	sem_t lock;
} buf_t;

int in, out;

int *producer(buf_t * buf)
{
	int data;
	int i;

	for (i = 0; i < Max_Num; i++) {
		if (-1 == sem_wait(&buf->occupied)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_wait(&buf->lock)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		data = 100 * i;
		buf->buffer[in] = data;
		printf("producer has added %d to the buffer[%d] \n", data, in);
		in = (in + 1) % BUF_SIZE;
		if (-1 == sem_post(&buf->lock)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_post(&buf->empty)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
	}
	pthread_exit(NULL);
}

int *consumer(buf_t * buf)
{
	int data;
	int i;

	for (i = 0; i < Max_Num; i++) {
		if (-1 == sem_wait(&buf->empty)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_wait(&buf->lock)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		data = buf->buffer[out];
		printf("consumer has taken %d from buffer[%d] \n", data, out);
		out = (out + 1) % BUF_SIZE;
		if (-1 == sem_post(&buf->lock)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_post(&buf->occupied)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
	}
	pthread_exit(0);
}

int main(void)
{
	int shared = 1;
	int occupied_value = BUF_SIZE;
	int empty_value = 0;
	int lock_value = 1;
	buf_t buf;
	pthread_t con, pro;

#ifndef  _POSIX_SEMAPHORES
	printf("_POSIX_SEMAPHORES is not defined \n");
	return PTS_UNRESOLVED;
#endif
	if (-1 == sem_init(&buf.occupied, shared, occupied_value)) {
		perror("sem_init didn't return success \n");
		return PTS_UNRESOLVED;
	}
	if (-1 == sem_init(&buf.empty, shared, empty_value)) {
		perror("sem_init didn't return success \n");
		return PTS_UNRESOLVED;
	}
	if (-1 == sem_init(&buf.lock, shared, lock_value)) {
		perror("sem_init didn't return success \n");
		return PTS_UNRESOLVED;
	}
	in = out = 0;

	pthread_create(&con, NULL, (void *)consumer, &buf);
	pthread_create(&pro, NULL, (void *)producer, &buf);
	pthread_join(con, NULL);
	pthread_join(pro, NULL);
	sem_destroy(&buf.occupied);
	sem_destroy(&buf.empty);
	return PTS_PASS;
}
