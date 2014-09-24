/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * This is a test about multiple producers and consumers. Producers send data
 * to a buffer. Consumers keeps reading data from the buffer.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <pthread.h>

#include <semaphore.h>
#include "posixtest.h"

#define BUF_SIZE	5
#define Max_Num		5
#define Max_Threads	127

typedef struct {
	int buffer[BUF_SIZE];
	sem_t full;
	sem_t empty;
	sem_t lock;
} buf_t;

buf_t *buf;
int in, out;

int *producer(void *ID)
{
	int data;
	int i;
	int ThreadID = *(int *)ID;
	int full_value;

	printf("Enter into Producer Thread %d... \n", ThreadID);
	for (i = 0; i < Max_Num - 1; i++) {
		if (-1 == sem_wait(&buf->full)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_getvalue(&buf->full, &full_value)) {
			perror("sem_getvalue didn't return success \n");
			pthread_exit((void *)1);
		}
		printf("The value of the full semaphore is %d \n", full_value);
		if (-1 == sem_wait(&buf->lock)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		data = 100 * ThreadID + i;
		buf->buffer[in] = data;
		printf("[%d] producer has added %d to the buffer[%d] \n",
		       ThreadID, data, in);
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
	if (-1 == sem_wait(&buf->full)) {
		perror("sem_wait didn't return success \n");
		pthread_exit((void *)1);
	}
	if (-1 == sem_getvalue(&buf->full, &full_value)) {
		perror("sem_getvalue didn't return success \n");
		pthread_exit((void *)1);
	}
	printf("The value of the full is %d \n", full_value);
	if (-1 == sem_wait(&buf->lock)) {
		perror("sem_wait didn't return success \n");
		pthread_exit((void *)1);
	}
	data = -1;
	buf->buffer[in] = data;
	printf("[%d] producer has added %d to the buffer[%d] \n", ThreadID,
	       data, in);
	in = (in + 1) % BUF_SIZE;
	if (-1 == sem_post(&buf->lock)) {
		perror("sem_wait didn't return success \n");
		pthread_exit((void *)1);
	}
	if (-1 == sem_post(&buf->empty)) {
		perror("sem_wait didn't return success \n");
		pthread_exit((void *)1);
	}
	printf("Producer %d exit... \n", ThreadID);
	pthread_exit(NULL);
}

int *consumer(void *ID)
{
	int data;
	int ThreadID = *(int *)ID;
	int full_value;

	printf("Enter into Consumer Thread %d... \n", ThreadID);
	do {
		if (-1 == sem_wait(&buf->empty)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_wait(&buf->lock)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		data = buf->buffer[out];
		printf("[%d] consumer has taken %d from buffer[%d] \n",
		       ThreadID, data, out);
		out = (out + 1) % BUF_SIZE;
		if (-1 == sem_post(&buf->lock)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_post(&buf->full)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_getvalue(&buf->full, &full_value)) {
			perror("sem_getvalue didn't return success \n");
			pthread_exit((void *)1);
		}
		printf("The value of the full semaphore is %d \n", full_value);
	}
	while (data != -1);

	printf("Consumer %d exit... \n", ThreadID);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int shared = 1;
	int full_value = BUF_SIZE;
	int empty_value = 0;
	int lock_value = 1;
	int num;
	int i;
	pthread_t con[Max_Threads], pro[Max_Threads];
	int ThreadID[Max_Threads];

#ifndef	_POSIX_SEMAPHORES
	printf("_POSIX_SEMAPHORES is not defined \n");
	return PTS_UNRESOLVED;
#endif

	buf = malloc(sizeof(buf_t));

	if ((2 != argc) || ((num = atoi(argv[1])) <= 0)) {
		fprintf(stderr, "Usage: %s number_of_threads\n", argv[0]);
		return PTS_FAIL;
	}
	if (num > Max_Threads) {
		printf
		    ("The num of producers/consumers threads are too large.  Reset to %d\n",
		     Max_Threads);
		num = Max_Threads;
	}

	if (-1 == sem_init(&buf->full, shared, full_value)) {
		perror("sem_init didn't return success \n");
		return PTS_UNRESOLVED;
	}
	if (-1 == sem_getvalue(&buf->full, &full_value)) {
		perror("sem_getvalue didn't return success \n");
		return PTS_UNRESOLVED;
	}
	printf("The initial value of the full semaphore is %d \n", full_value);
	if (-1 == sem_init(&buf->empty, shared, empty_value)) {
		perror("sem_init didn't return success \n");
		return PTS_UNRESOLVED;
	}
	if (-1 == sem_init(&buf->lock, shared, lock_value)) {
		perror("sem_init didn't return success \n");
		return PTS_UNRESOLVED;
	}
	in = out = 0;

	for (i = 0; i < num; i++) {
		ThreadID[i] = i;
		pthread_create(&con[i], NULL, (void *)consumer,
			       (void *)&ThreadID[i]);
	}
	for (i = 0; i < num; i++) {
		ThreadID[i] = i;
		pthread_create(&pro[i], NULL, (void *)producer,
			       (void *)&ThreadID[i]);
	}
	for (i = 0; i < num; i++)
		pthread_join(con[i], NULL);
	for (i = 0; i < num; i++)
		pthread_join(pro[i], NULL);

	sem_destroy(&buf->full);
	sem_destroy(&buf->empty);
	sem_destroy(&buf->lock);
	return PTS_PASS;
}
