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
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include "rpc.h"

//Standard define
#define VERSNUM 1
#define PINGPROC	1
#define PROGSYSERROR	10
#define PROGAUTHERROR	100

static void exm_proc();
int progNum;
int run_mode;

void *server_thread_process(void *arg)
{
	//Server process in a thread
	int err = 0;
	int i = (long)arg;

	if (run_mode == 1) {
		printf("Server #%d launched\n", i);
		printf("Server Nb : %d\n", progNum + i);
	}

	svc_unreg(progNum + atoi(arg), VERSNUM);

	err = svc_create(exm_proc, progNum + atoi(arg), VERSNUM, "VISIBLE");

	if (err == 0) {
		fprintf(stderr, "Cannot create service.\n");
		exit(1);
	}

	svc_run();

	fprintf(stderr, "svc_run() returned.  ERROR has occurred.\n");
	svc_unreg(progNum, VERSNUM);

	pthread_exit(0);
}

//****************************************//
//***           Main Function          ***//
//****************************************//
int main(int argn, char *argc[])
{
	//Server parameter is : argc[1] : Server Program Number
	//                                          argc[2] : Number of threads
	//                                          others arguments depend on server program
	run_mode = 0;
	int threadNb = atoi(argc[2]);
	long i;
	//Thread declaration
	pthread_t *pThreadArray;
	void *ret;

	progNum = atoi(argc[1]);

	pThreadArray = malloc(threadNb * sizeof(pthread_t));
	for (i = 0; i < threadNb; i++) {
		if (run_mode == 1)
			fprintf(stderr, "Try to create Thread Server %ld\n", i);
		if (pthread_create
		    (&pThreadArray[i], NULL, server_thread_process, (void*)i) < 0) {
			fprintf(stderr, "pthread_create error for thread 1\n");
			exit(1);
		}
	}

	//Clean threads
	for (i = 0; i < threadNb; i++) {
		(void)pthread_join(pThreadArray[i], &ret);
	}

	return 1;
}

//****************************************//
//***        Remotes Procedures        ***//
//****************************************//
char *pingProc(int *dt)
{
	//Makes a + b * c from structure dt and returns double
	//printf("*** In calcProc ***\n");
	static int result = 0;
	result = *dt;
	//printf("Received : %lf, %lf, %lf\n", dt->a, dt->b, dt->c);
	return (char *)&result;
}

//****************************************//
//***       Dispatch Function          ***//
//****************************************//
static void exm_proc(struct svc_req *rqstp, SVCXPRT * transp)
{
	//printf("* in Dispatch Func.\n");

	char *result;
	xdrproc_t xdr_argument;
	xdrproc_t xdr_result;
	int *(*proc) (int *);

	union {
		int varIn;
	} argument;

	switch (rqstp->rq_proc) {
	case PINGPROC:
		{
			//printf("** in PROCPONG dispatch Func.\n");
			xdr_argument = (xdrproc_t) xdr_int;
			xdr_result = (xdrproc_t) xdr_int;
			proc = (int *(*)(int *))pingProc;
			break;
		}
	case PROGSYSERROR:
		{
			//Simulate an error
			svcerr_systemerr(transp);
			return;
		}
	case PROGAUTHERROR:
		{
			//Simulate an authentification error
			svcerr_weakauth(transp);
			return;
		}
	default:
		{
			//Proc is unavaible
			svcerr_noproc(transp);
			return;
		}
	}
	memset((int *)&argument, (int)0, sizeof(argument));
	if (svc_getargs(transp, xdr_argument, (char *)&argument) == FALSE) {
		svcerr_decode(transp);
		return;
	}

	result = (char *)(*proc) ((int *)&argument);

	if ((result != NULL)
	    && (svc_sendreply(transp, xdr_result, result) == FALSE)) {
		svcerr_systemerr(transp);
	}
	if (svc_freeargs(transp, xdr_argument, (char *)&argument) == FALSE) {
		(void)fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}
