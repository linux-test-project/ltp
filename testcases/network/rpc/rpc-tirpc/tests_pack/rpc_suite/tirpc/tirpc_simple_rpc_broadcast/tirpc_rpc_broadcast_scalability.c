/*
* Copyright (c) Bull S.A.  2007 All Rights Reserved.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Further, this software is distributed without any warranty that it is
* free of the rightful claim of any third person regarding infringement
* or the like.  Any license provided herein, whether implied or
* otherwise, applies only to this software file.  Patent licenses, if
* any, provided herein do not apply to combinations of this program with
* other software, or any other product whatsoever.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
* History:
* Created by: Cyril Lacabanne (Cyril.Lacabanne@bull.net)
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include "rpc.h"

//Standard define
#define PROCNUM 1
#define VERSNUM 1

static double *thread_time_result;
static int maxThd = 1;
int run_mode;
int progNum;
int callNb;
char *nettype = "visible";
char *hostname;

int eachresult(char *out, struct sockaddr_in *addr)
{
	//Nothing to do for that test
	return 1;
}

void *my_thread_process(void *arg)
{
	int i;
	int sndVar = 0;
	int recVar;

	if (run_mode == 1) {
		fprintf(stderr, "Thread %d\n", atoi(arg));
	}

	for (i = 0; i < callNb; i++) {
		rpc_broadcast(progNum, VERSNUM, PROCNUM,
			      (xdrproc_t) xdr_int, (char *)&sndVar,
			      (xdrproc_t) xdr_int, (char *)&recVar,
			      (resultproc_t) eachresult, nettype);
	}

	pthread_exit(0);
}

int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//                                         argc[2] : Server Program Number
	//                                         argc[3] : Maximal number of threads
	//                                         argc[4] : Number of calls per thread
	//                                         other arguments depend on test case

	//run_mode can switch into stand alone program or program launch by shell script
	//1 : stand alone, debug mode, more screen information
	//0 : launch by shell script as test case, only one printf -> result status
	run_mode = 0;
	int test_status = 1;	//Default test result set to FAILED
	int i;
	long j;
	int threadNb = atoi((char *)argc[3]);
	int curThd = 1;

	//Thread declarations
	pthread_t *pThreadArray;
	void *ret = NULL;
	pthread_attr_t thread_attr;
	int ssz = 0;

	//Time measurement declarations
	struct timeval tv1, tv2;
	struct timezone tz;
	long long diff;
	double rslt;

	//Program initialization
	progNum = atoi((char *)argc[2]);
	callNb = atoi((char *)argc[4]);
	hostname = (char *)argc[1];

	//Initialization
	maxThd = maxThd << (threadNb - 1);	//Set the maximum threads number

	pthread_attr_init(&thread_attr);

	if (run_mode == 1) {
		pthread_attr_getstacksize(&thread_attr, (size_t *) & ssz);	//For debug purpose, get default thread stack size
		fprintf(stderr, "Server #%d\n", progNum);
		fprintf(stderr, "Calls per thread : %d\n", callNb);
		fprintf(stderr, "Instances : %d\n", threadNb);
		fprintf(stderr, "Max threads to create : %d\n", maxThd);
		fprintf(stderr, "Standard thread stack size in bytes %d\n",
			ssz);
	}

	pthread_attr_setstacksize(&thread_attr, 40000);	//Set thread stack size to 40 KB

	//Init results table
	thread_time_result = malloc((threadNb) * sizeof(double));
	memset(&thread_time_result[0], (double)0, (threadNb) * sizeof(double));

	//Create all threads
	//Run all threads
	pThreadArray = malloc(maxThd * sizeof(pthread_t));

	for (i = 0; i < threadNb; i++) {
		if (run_mode)
			fprintf(stderr, "Threads for pass %d : %d\n", i,
				curThd);

		gettimeofday(&tv1, &tz);

		for (j = 0; j < curThd; j++) {
			//Create thread using defined parameters (stack size = 40 KB)
			if (pthread_create
			    (&pThreadArray[j], &thread_attr, my_thread_process,
			     (void *)j) != 0) {
				fprintf(stderr,
					"pthread_create error for thread %ld\n",
					j);
				printf("1\n");
				exit(1);
			}
		}

		//Clean threads
		for (j = 0; j < curThd; j++) {
			if ((pthread_t *) pThreadArray[j] != NULL) {
				(void)pthread_join(pThreadArray[j], &ret);
			} else {
				fprintf(stderr, "pThread Join Err : %ld\n", j);
			}
		}

		gettimeofday(&tv2, &tz);

		//Calculate and store delay to table results
		diff =
		    (tv2.tv_sec - tv1.tv_sec) * 1000000L + (tv2.tv_usec -
							    tv1.tv_usec);
		rslt = (double)diff / 1000;
		thread_time_result[i] = rslt;

		curThd = curThd * 2;
	}

	//Check if all threads results are ok
	test_status = 0;
	for (i = 0; i < threadNb; i++) {
		if (thread_time_result[i] == 0) {
			test_status = 1;
			break;
		}
	}

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	//Print scalability results
	curThd = 1;
	for (i = 0; i < threadNb; i++) {
		printf("%d %lf\n", curThd, thread_time_result[i]);
		curThd = curThd * 2;
	}

	return test_status;
}
