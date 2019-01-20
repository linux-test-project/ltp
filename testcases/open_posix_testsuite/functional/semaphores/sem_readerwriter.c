/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * This test use semaphore to implement reader and writer problem. Some readers
 * and some writers read/write on one blackboard. Only one writer allow to
 * write on the board at the same time. Reader and Writer can't use the board
 * the same time. Reader has higher priority than writer, which means only when
 * no reader reads the board, the writer can write the board.
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
#include <pthread.h>

#include "posixtest.h"

#define SEM_NAME       "/tmp/semaphore"
#define READ_NUM	10
#define WRITE_NUM	15

sem_t r_lock, w_lock;
int reader_count = 0;
int data = 0;

int read_fun(int ID LTP_ATTRIBUTE_UNUSED)
{
	printf("read the board, data=%d \n", data);
	return 0;
}

int write_fun(int ID)
{
	data = 100 * ID + ID;
	printf("write the board, data=%d \n", data);
	return 0;
}

int *reader(void *ID)
{
	int ThID = *(int *)ID;
	if (-1 == sem_wait(&r_lock)) {
		perror("sem_wait didn't return success\n");
		pthread_exit((void *)1);
	}
	reader_count++;
	printf("Enter into Reader thread, reader_count=%d \n", reader_count);
	if (reader_count == 1) {
		if (-1 == sem_wait(&w_lock)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
	}
	if (-1 == sem_post(&r_lock)) {
		perror("sem_post didn't return success \n");
		pthread_exit((void *)1);
	}
	sleep(1);
	read_fun(ThID);
	if (-1 == sem_wait(&r_lock)) {
		perror("sem_wait didn't return success \n");
		pthread_exit((void *)1);
	}
	reader_count--;
	if (reader_count == 0) {
		if (-1 == sem_post(&w_lock)) {
			perror("sem_post didn't return success \n");
			pthread_exit((void *)1);
		}
	}
	if (-1 == sem_post(&r_lock)) {
		perror("sem_post didn't return success \n");
		pthread_exit((void *)1);
	}
	printf("Reader Thread [%d] exit...reader_count=%d \n", ThID,
	       reader_count);
	pthread_exit(NULL);
}

int *writer(void *ID)
{
	int ThID = *(int *)ID;
/* When ThID is equal to WRITE_NUM/2, sleep 2 second and let reader read the data */
	if (ThID >= WRITE_NUM / 2)
		sleep(2);
	if (-1 == sem_wait(&w_lock)) {
		perror("sem_wait didn't return success \n");
		pthread_exit((void *)1);
	}
	write_fun(ThID);
	if (-1 == sem_post(&w_lock)) {
		perror("sem_post didn't return success \n");
		pthread_exit((void *)1);
	}
	printf("Writer Thread [%d] exit...\n", ThID);
	pthread_exit(NULL);
}

int main(void)
{
	pthread_t rea[READ_NUM], wri[WRITE_NUM];
	int ReadID[READ_NUM], WriteID[WRITE_NUM];
	int shared = 1;
	int r_value = 1;
	int w_value = 1;
	int i;

#ifndef  _POSIX_SEMAPHORES
	printf("_POSIX_SEMAPHORES is not defined \n");
	return PTS_UNRESOLVED;
#endif
	if (-1 == sem_init(&r_lock, shared, r_value)) {
		perror("sem_init didn't return success \n");
		return PTS_UNRESOLVED;
	}
	if (-1 == sem_init(&w_lock, shared, w_value)) {
		perror("sem_init didn't return success \n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < WRITE_NUM; i++) {
		WriteID[i] = i;
		pthread_create(&wri[i], NULL, (void *)writer, &WriteID[i]);
	}
	for (i = 0; i < READ_NUM; i++) {
		ReadID[i] = i;
		pthread_create(&rea[i], NULL, (void *)reader, &ReadID[i]);
	}

	for (i = 0; i < READ_NUM; i++)
		pthread_join(rea[i], NULL);
	for (i = 0; i < WRITE_NUM; i++)
		pthread_join(wri[i], NULL);

	if (-1 == sem_destroy(&r_lock)) {
		perror("sem_destroy didn't return success \n");
		return PTS_UNRESOLVED;
	}
	if (-1 == sem_destroy(&w_lock)) {
		perror("sem_destroy didn't return success \n");
		return PTS_UNRESOLVED;
	}
	return PTS_PASS;
}
