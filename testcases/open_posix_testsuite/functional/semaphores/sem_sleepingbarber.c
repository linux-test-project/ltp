
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
#include <getopt.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "posixtest.h"
#define CHAIR_NUM	5
#define CUS_NUM		10
#define LOOP_NUM	30

sem_t customer;
sem_t barber;
sem_t lock;

int waiting = 0;

int get_haircut(int ID)
{
	printf("Customer [%d] gets the haircut\n", ID);
	return 0;
}
int barbers()
{
	int i;
	for (i = 0; i < LOOP_NUM; i++) {
		if (-1 == sem_wait(&lock)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		if (waiting == 0) {
			printf("There is no customer waiting now, barber will sleep...\n");
		 	if (-1 == sem_post(&lock)) {
				perror("sem_post didn't return success \n");
				pthread_exit((void *)1);
			}
			break;
		}
		if (-1 == sem_post(&lock)) {
			perror("sem_post didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_wait(&customer)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_wait(&lock)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		if (waiting >= 1)
			waiting--;
		printf("reduce one customers, totoally %d customers are waiting . \n", waiting);
		if (-1 == sem_post(&lock)) {
			perror("sem_post didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_post(&barber)) {
			perror("sem_post didn't return success \n");
			pthread_exit((void *)1);
		}
			
	}	
	pthread_exit((void *) 0);
}
int customers(void* ID)
{
	int CusID;
	CusID = *(int *)ID;

	printf("customer %d enter into the barber room...\n", CusID);
	if (-1 == sem_wait(&lock)) {
		perror("sem_wait didn't return success \n");
		pthread_exit((void *)1);
	}
	if (waiting < CHAIR_NUM) {
		waiting = waiting + 1;
		if (-1 == sem_post(&customer)) {
			perror("sem_post didn't return success \n");
			pthread_exit((void *)1);
		}
		printf("one customer added, totally %d customers are waiting...\n", waiting);
		if (-1 == sem_post(&lock)) {	
			perror("sem_post didn't return success \n");
			pthread_exit((void *)1);
		}
		if (-1 == sem_wait(&barber)) {
			perror("sem_wait didn't return success \n");
			pthread_exit((void *)1);
		}
		get_haircut(CusID);
	}
	else
	{
		printf("Chair is full, customer %d quit.. \n", CusID);
		if (-1 == sem_post(&lock)) {	
			perror("sem_post didn't return success \n");
			pthread_exit((void *)1);
		}
	}
	pthread_exit((void *) 0);
}
int main(int argc, char *argv[])
{
	pthread_t bar, cus[CUS_NUM];
	int shared = 1;
	int barber_value = 0;
	int customer_value = 0;
	int lock_value=1;
	int i, ID[CUS_NUM];

#ifndef  _POSIX_SEMAPHORES
	printf("_POSIX_SEMAPHORES is not defined \n");
	return PTS_UNRESOLVED;
#endif 
	if (-1 == sem_init(&customer, shared, customer_value)) {
		perror("sem_init didn't return success \n"); 
		return PTS_UNRESOLVED;
	}
	if (-1 == sem_init(&barber, shared, barber_value)) {
		perror("sem_init didn't return success \n"); 
		return PTS_UNRESOLVED;
	}
	if (-1 == sem_init(&lock, shared, lock_value)) {
		perror("sem_init didn't return success \n"); 
		return PTS_UNRESOLVED;
	}
	for (i = 0; i < CUS_NUM; i++) {
		ID[i] = i;
		pthread_create(&cus[i], NULL, (void *)customers, &ID[i]);	
	}
	pthread_create(&bar, NULL, (void *)barbers, NULL);	
	pthread_join(bar, NULL);
	for (i = 0; i< CUS_NUM; i++) 
		pthread_join(cus[i], NULL);

	sem_destroy(&barber);
	sem_destroy(&customer);
	return PTS_PASS;
}
