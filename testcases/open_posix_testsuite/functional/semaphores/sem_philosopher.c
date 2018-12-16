/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test the well-known philosophy problem.
 *
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

#define	PH_NUM		5
#define LOOP_NUM	20
#define thinking	0
#define hungry		1
#define eating		2

sem_t ph[PH_NUM];
sem_t lock;

int state[PH_NUM];

int think(int ID)
{
	printf("Philosoper [%d] is thinking... \n", ID);
	return 0;
}

int eat(int ID)
{
	printf("Philosoper [%d] is eating... \n", ID);
	return 0;
}

int test(int ID)
{
	int preID = 0, postID = 0;
	if ((ID - 1) < 0)
		preID = PH_NUM + (ID - 1);
	else
		preID = (ID - 1) % PH_NUM;

	if ((ID + 1) >= PH_NUM)
		postID = ID + 1 - PH_NUM;
	else
		postID = (ID + 1) % PH_NUM;

	if ((state[ID] == hungry) && (state[preID] != eating)
	    && (state[postID] != eating)) {
		state[ID] = eating;
		sem_post(&ph[ID]);
	}
	return 0;

}

int philosopher(void *ID)
{
	int PhID = *(int *)ID;
	int prePH, postPH;
	int i;

	for (i = 0; i < LOOP_NUM; i++) {
		think(PhID);
		sleep(1);
		if (-1 == sem_wait(&lock)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		state[PhID] = hungry;
		test(PhID);
		if (-1 == sem_post(&lock)) {
			perror("sem_post didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_wait(&ph[PhID])) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		eat(PhID);
		sleep(1);
		if (-1 == sem_wait(&lock)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		state[PhID] = thinking;
		if ((PhID - 1) < 0)
			prePH = PH_NUM + (PhID - 1);
		else
			prePH = (PhID - 1) % PH_NUM;
		if ((PhID + 1) >= PH_NUM)
			postPH = PhID + 1 - PH_NUM;
		else
			postPH = (PhID + 1) % PH_NUM;
		test(prePH);
		test(postPH);
		if (-1 == sem_post(&lock)) {
			perror("sem_post didn't return success \n");
			pthread_exit((void *)1);
		}
	}
	pthread_exit(NULL);
}

int main(void)
{
	pthread_t phi[PH_NUM];
	int PhID[PH_NUM];
	int shared = 1;
	int ph_value = 0;
	int lock_value = 1;
	int i;

#ifndef  _POSIX_SEMAPHORES
	printf("_POSIX_SEMAPHORES is not defined \n");
	return PTS_UNRESOLVED;
#endif
	for (i = 0; i < PH_NUM; i++) {
		if (-1 == sem_init(&ph[i], shared, ph_value)) {
			perror("sem_init didn't return success \n");
			return PTS_UNRESOLVED;
		}
		state[i] = 0;
	}
	if (-1 == sem_init(&lock, shared, lock_value)) {
		perror("sem_init didn't return success \n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < PH_NUM; i++) {
		PhID[i] = i;
		pthread_create(&phi[i], NULL, (void *)philosopher, &PhID[i]);
	}

	for (i = 0; i < PH_NUM; i++) {
		pthread_join(phi[i], NULL);
	}

	for (i = 0; i < PH_NUM; i++) {
		if (-1 == sem_destroy(&ph[i])) {
			perror("sem_destroy didn't return success \n");
			return PTS_UNRESOLVED;
		}
	}
	if (-1 == sem_destroy(&lock)) {
		perror("sem_destroy didn't return success \n");
		return PTS_UNRESOLVED;
	}
	return PTS_PASS;
}
