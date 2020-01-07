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
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include "rpc.h"

//Standard define
#define PROCNUM 1
#define VERSNUM 1

static void exm_proc();
static int *thread_array_result;
int run_mode;
int progNum;
int callNb;

void *my_thread_process(void *arg)
{
	SVCXPRT *transp = NULL;
	struct netconfig *nconf = NULL;
	int i;

	if (run_mode == 1) {
		fprintf(stderr, "Thread %ld\n", (long)arg);
	}

	nconf = getnetconfigent("udp");
	if (nconf == NULL) {
		//syslog(LOG_ERR, "getnetconfigent for udp failed");
		printf("err nconf\n");
		pthread_exit((void*)1l);
	}

	transp = svc_tli_create(RPC_ANYFD, nconf, NULL, 0, 0);

	for (i = 0; i < callNb; i++) {
		svc_unreg(progNum + atoi(arg), VERSNUM);
		thread_array_result[atoi(arg)] +=
		    svc_reg(transp, progNum + atoi(arg), VERSNUM, exm_proc,
			    nconf);
	}
	pthread_exit(0);
}

int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//                                         argc[2] : Server Program Number
	//                                         argc[3] : Number of threads
	//                                         argc[4] : Number of calls per thread
	//                                         other arguments depend on test case

	//run_mode can switch into stand alone program or program launch by shell script
	//1 : stand alone, debug mode, more screen information
	//0 : launch by shell script as test case, only one printf -> result status
	run_mode = 0;
	int test_status = 1;	//Default test result set to FAILED
	int threadNb = atoi(argc[3]);
	long i;
	pthread_t *pThreadArray;
	void *ret;

	progNum = atoi(argc[2]);
	callNb = atoi(argc[4]);

	if (run_mode == 1) {
		printf("Server #%d\n", progNum);
		printf("Thread to create %d\n", threadNb);
	}
	//Initialization : create threads results array, init elements to 0
	//Each thread will put function result (pas/fail) into array
	thread_array_result = malloc(threadNb * sizeof(int));
	memset(&thread_array_result[0], 0, threadNb * sizeof(int));

	//Create all threads
	//Run all threads
	pThreadArray = malloc(threadNb * sizeof(pthread_t));
	for (i = 0; i < threadNb; i++) {
		if (run_mode == 1)
			fprintf(stderr, "Try to create thread %ld\n", i);
		if (pthread_create(&pThreadArray[i], NULL, my_thread_process, (void*)i)
		    < 0) {
			fprintf(stderr, "pthread_create error for thread 1\n");
			exit(1);
		}
	}

	//Clean threads
	for (i = 0; i < threadNb; i++) {
		(void)pthread_join(pThreadArray[i], &ret);
	}

	//Check if all threads results are ok
	test_status = 0;
	for (i = 0; i < threadNb; i++) {
		if (thread_array_result[i] != callNb) {
			test_status = 1;
			break;
		}
	}

	if (run_mode == 1) {
		for (i = 0; i < threadNb; i++) {
			fprintf(stderr, "Result[%ld]=%d\n", i,
				thread_array_result[i]);
		}
	}
	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	return test_status;
}

static void exm_proc(struct svc_req *rqstp, SVCXPRT * transp)
{
	//Nothing to do here in that test case
}
