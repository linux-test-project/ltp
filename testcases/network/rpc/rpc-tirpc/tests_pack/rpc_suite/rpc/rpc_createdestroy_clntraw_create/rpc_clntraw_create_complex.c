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
#include <time.h>
#include <rpc/rpc.h>

#define VERSNUM	1
#define PROCNUM 10000

#define MAXITER	10

static void serverDisp();

int main(int argc, char **argv)
{
	CLIENT *clnt = NULL;
	SVCXPRT *svc = NULL;
	int progNum = atoi(argv[2]);
	int test_status = 0;
	int run_mode = 0;
	struct timeval tv;
	long long resTbl[MAXITER];
	struct timeval tv1, tv2;
	struct timezone tz;
	long long diff;
	int intSnd = 0;
	int intRec;
	int i;
	int sum;

	//Initialization
	tv.tv_sec = 0;
	tv.tv_usec = 100;

	for (i = 0; i < MAXITER; i++)
		resTbl[i] = -1;

	//Create both client and server handle
	svc = svcraw_create();

	if (svc == NULL) {
		fprintf(stderr, "Could not create server handle\n");
		exit(5);
	}

	if (!svc_register(svc, progNum, VERSNUM, (void *)serverDisp, 0)) {
		fprintf(stderr, "Error svc_register\n");
		exit(5);
	}

	clnt = clntraw_create(progNum, VERSNUM);

	if (clnt == NULL) {
		clnt_pcreateerror("raw");
		exit(1);
	}

	if (run_mode == 1) {
		printf("CLNT %p\n", clnt);
		printf("SVC %p\n", svc);
	}
	//Call RPC using testing mode (raw)
	for (i = 0; i < MAXITER; i++) {
		intSnd = i;

		gettimeofday(&tv1, &tz);
		if (clnt_call
		    (clnt, PROCNUM, (xdrproc_t) xdr_int, (char *)&intSnd,
		     (xdrproc_t) xdr_int, (char *)&intRec, tv) != RPC_SUCCESS) {
			clnt_perror(clnt, "raw");
			exit(1);
		} else {
			gettimeofday(&tv2, &tz);
			diff =
			    (tv2.tv_sec - tv1.tv_sec) * 1000000L +
			    (tv2.tv_usec - tv1.tv_usec);
			resTbl[i] = diff;
		}
	}

	//Test all returned values, calc. average
	for (i = 0; i < MAXITER; i++) {
		if (resTbl[i] == -1) {
			test_status = 1;
			break;
		}
		sum += resTbl[i];
		if (run_mode == 1)
			fprintf(stderr, "%lld\n", resTbl[i]);
	}
	sum = (int)(sum / MAXITER);

	printf("%d\n", test_status);

	return test_status;
}

static void serverDisp(struct svc_req *rqstp, SVCXPRT * transp)
{
	int numRec;
	fprintf(stderr, "in server proc\n");

	switch (rqstp->rq_proc) {
	case 0:
		if (svc_sendreply(transp, (xdrproc_t) xdr_void, 0) == FALSE) {
			fprintf(stderr, "error in null proc\n");
			exit(1);
		}
		return;
	case PROCNUM:
		break;
	default:
		svcerr_noproc(transp);
		return;
	}

	if (!svc_getargs(transp, (xdrproc_t) xdr_int, (char *)&numRec)) {
		svcerr_decode(transp);
		return;
	}

	numRec++;
	if (svc_sendreply(transp, (xdrproc_t) xdr_int, (char *)&numRec) ==
	    FALSE) {
		fprintf(stderr, "error in sending answer\n");
		exit(1);
	}

	return;
}
