/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test the well-known sleeping barber problem.
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
#include <time.h>

#include "posixtest.h"
#define CHAIR_NUM	5
#define CUS_NUM		10
#define LOOP_NUM	30

sem_t customer;
sem_t barber;
sem_t lock;
sem_t print;

int waiting = 0;

#ifdef __GNUC__
#define my_printf(x...) do { \
	sem_wait(&print); \
	printf(x); \
	sem_post(&print); \
} while (0)
#else
#define my_printf printf
#endif

void mdelay(unsigned msecs)
{
	struct timespec req;
	req.tv_sec = msecs / 1000;
	req.tv_nsec = (msecs % 1000) * 1000000;
	nanosleep(&req, NULL);
}

void *barbers(void *unused LTP_ATTRIBUTE_UNUSED)
{
	int i;
	for (i = 0; i < LOOP_NUM; i++) {
		if (-1 == sem_wait(&lock)) {
			perror("sem_wait(&lock) didn't return success");
			pthread_exit((void *)1);
		}
		if (waiting == 0) {
			my_printf
			    ("There are no more customers waiting, barber will sleep.\n");
		}
		if (-1 == sem_post(&lock)) {
			perror("sem_post(&lock) didn't return success");
			pthread_exit((void *)1);
		}
		if (-1 == sem_wait(&customer)) {
			perror("sem_wait(&customer) didn't return success");
			pthread_exit((void *)1);
		}
		if (-1 == sem_wait(&lock)) {
			perror("sem_wait(&lock) didn't return success");
			pthread_exit((void *)1);
		}
		if (waiting >= 1)
			waiting--;
		my_printf
		    ("A customer sits in the barber's chair and get a hair cut.  %d customers left waiting.\n",
		     waiting);
		if (-1 == sem_post(&lock)) {
			perror("sem_post(&lock) didn't return success");
			pthread_exit((void *)1);
		}
		if (-1 == sem_post(&barber)) {
			perror("sem_post(&barber) didn't return success");
			pthread_exit((void *)1);
		}

	}
	return NULL;
}

void *customers(void *ID)
{
	int CusID;
	CusID = *(int *)ID;

	if (CusID == 8)
		mdelay(10);

	my_printf("customer %d enters the room.\n", CusID);
	if (-1 == sem_wait(&lock)) {
		perror("sem_wait(&lock) didn't return success");
		pthread_exit((void *)1);
	}
	if (waiting < CHAIR_NUM) {
		waiting = waiting + 1;
		if (-1 == sem_post(&customer)) {
			perror("sem_post(&customer) didn't return success");
			pthread_exit((void *)1);
		}
		my_printf
		    ("Customer %d sits down, now %d customers are waiting.\n",
		     CusID, waiting);
		if (-1 == sem_post(&lock)) {
			perror("sem_post(&lock) didn't return success");
			pthread_exit((void *)1);
		}
		if (-1 == sem_wait(&barber)) {
			perror("sem_wait(&barber) didn't return success");
			pthread_exit((void *)1);
		}
		my_printf("Customer %d leaves with nice hair.\n", CusID);
	} else {
		my_printf
		    ("No chairs available, customer %d leaves without a haircut.\n",
		     CusID);
		if (-1 == sem_post(&lock)) {
			perror("sem_post(&lock) didn't return success");
			pthread_exit((void *)1);
		}
	}
	return NULL;
}

int main(void)
{
	pthread_t bar, cus[CUS_NUM];
	int shared = 0;
	int barber_value = 0;
	int customer_value = 0;
	int lock_value = 1;
	int i, ID[CUS_NUM];

	if (-1 == sem_init(&print, shared, 1)) {
		perror("sem_init(&print) didn't return success");
		return PTS_UNRESOLVED;
	}
#ifndef  _POSIX_SEMAPHORES
	my_printf("_POSIX_SEMAPHORES is not defined\n");
	return PTS_UNRESOLVED;
#endif
	if (-1 == sem_init(&customer, shared, customer_value)) {
		perror("sem_init(&customer) didn't return success");
		return PTS_UNRESOLVED;
	}
	if (-1 == sem_init(&barber, shared, barber_value)) {
		perror("sem_init(&barber) didn't return success");
		return PTS_UNRESOLVED;
	}
	if (-1 == sem_init(&lock, shared, lock_value)) {
		perror("sem_init(&lock) didn't return success");
		return PTS_UNRESOLVED;
	}
	for (i = 0; i < CUS_NUM; i++) {
		ID[i] = i;
		pthread_create(&cus[i], NULL, customers, (void *)&ID[i]);
	}
	pthread_create(&bar, NULL, barbers, NULL);
	for (i = 0; i < CUS_NUM; i++)
		pthread_join(cus[i], NULL);

	return PTS_PASS;
}
