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
* with this program; if not, write the Free Software Foundation, Inc., 59
* Temple Place - Suite 330, Boston MA 02111-1307, USA.
*
* History:
* Created by: Cyril Lacabanne (Cyril.Lacabanne@bull.net)
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <tirpc/netconfig.h>
#include <tirpc/rpc/rpc.h>
#include <tirpc/rpc/types.h>
#include <tirpc/rpc/xdr.h>
#include <tirpc/rpc/svc.h>
#include <errno.h>

//Standard define
#define CALCTHREADPROC	1000
#define VERSNUM 1

static int *thread_array_result;
int run_mode;
int progNum;
char *hostname;
char *nettype;

struct RES
{
	double locRes;
	double svcRes;
};

struct RES *resTbl;

struct datas
{
	double a;
	double b;
	double c;
};

bool_t xdr_datas(XDR *pt_xdr, struct datas* pt)
{
	return(xdr_double(pt_xdr, &(pt->a)) &&
		   xdr_double(pt_xdr, &(pt->b)) &&
		   xdr_double(pt_xdr, &(pt->c)));
}

double getRand()
{
	return (drand48() * 1000);
}

void *my_thread_process (void * arg)
{
	int i;
	struct datas vars;
	static double result = 0;

	if (run_mode == 1)
	{
		fprintf(stderr, "Thread %d\n", atoi(arg));
	}

	vars.a = getRand();
	vars.b = getRand();
	vars.c = getRand();

	resTbl[atoi(arg)].locRes = vars.a + (vars.b * vars.c);

	rpc_call(hostname, progNum, VERSNUM, CALCTHREADPROC,
             (xdrproc_t)xdr_datas, (char *)&vars, // xdr_in
             (xdrproc_t)xdr_double, (char *)&resTbl[atoi(arg)].svcRes, // xdr_out
             nettype);

	thread_array_result[atoi(arg)] = (resTbl[atoi(arg)].svcRes == resTbl[atoi(arg)].locRes) ? 0 : 1;

	if (run_mode == 1)
	{
		fprintf(stderr, "Thread #%d calc : %lf, received : %lf\n",
		        atoi(arg), resTbl[atoi(arg)].locRes,
		        resTbl[atoi(arg)].svcRes);
	}

    pthread_exit (0);
}

int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//					   argc[2] : Server Program Number
	//					   argc[3] : Number of threads
	//					   other arguments depend on test case

	//run_mode can switch into stand alone program or program launch by shell script
	//1 : stand alone, debug mode, more screen information
	//0 : launch by shell script as test case, only one printf -> result status
	run_mode = 0;
	int test_status = 0; //Default test result set to FAILED
	int threadNb = atoi(argc[3]);
	int i;
	pthread_t *pThreadArray;
    void *ret;

    hostname = argc[1];
    nettype = "VISIBLE";

    resTbl = (struct RES *)malloc(threadNb * sizeof(struct RES));

	progNum = atoi(argc[2]);

	if (run_mode == 1)
	{
		printf("Server #%d\n", progNum);
		printf("Thread to create %d\n", threadNb);
	}

	//Initialization : create threads results array, init elements to 0
	//Each thread will put function result (pas/fail) into array
	thread_array_result = (int *)malloc(threadNb * sizeof(int));
	memset(&thread_array_result[0], 0, threadNb * sizeof(int));

	//Create all threads
	//Run all threads
	pThreadArray = (pthread_t *)malloc(threadNb * sizeof(pthread_t));
	for (i = 0; i < threadNb; i++)
	{
		if (run_mode == 1)
			fprintf (stderr, "Try to create thread %d\n", i);
		if (pthread_create (&pThreadArray[i], NULL, my_thread_process, i) < 0)
	    {
	        fprintf (stderr, "pthread_create error for thread 1\n");
	        exit (1);
	    }
	}

	//Clean threads
	for (i = 0; i < threadNb; i++)
	{
		(void)pthread_join (pThreadArray[i], &ret);
	}

	//Check if all threads results are ok
	test_status = 0;
	for (i = 0; i < threadNb; i++)
	{
		if (thread_array_result[i] != 0)
		{
			test_status = 1;
			break;
		}
	}

	if (run_mode == 1)
	{
		for (i = 0; i < threadNb; i++)
		{
			fprintf(stderr, "Result[%d]=%d\n", i, thread_array_result[i]);
		}
	}

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	return test_status;
}
