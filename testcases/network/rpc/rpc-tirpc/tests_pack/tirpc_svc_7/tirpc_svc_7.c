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
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include "rpc.h"

//Standard define
#define VERSNUM 1
//Define limits test proc
#define PROCSIMPLEPING	1
//Define DataInt procs
#define INTPROCNUM 10
#define DBLPROCNUM 20
#define LNGPROCNUM 30
#define STRPROCNUM 40

static void exm_proc();

union u_argument {
	int varInt;
	double dbl;
	long lng;
	char *str;
} argument;

//****************************************//
//***           Main Function          ***//
//****************************************//
int main(int argn, char *argc[])
{
	int progNum = atoi(argc[1]);
	SVCXPRT *transp = NULL;
	struct netconfig *nconf;

	//Initialization
	svc_unreg(progNum, VERSNUM);

	if ((nconf = getnetconfigent("udp")) == NULL) {
		fprintf(stderr, "Cannot get netconfig entry for UDP\n");
		exit(1);
	}

	transp = svc_tp_create(exm_proc, progNum, VERSNUM, nconf);

	if (transp == NULL) {
		fprintf(stderr, "Cannot create service.\n");
		exit(1);
	}

	if (!svc_reg(transp, progNum, VERSNUM, exm_proc, nconf)) {
		fprintf(stderr, "svc_reg failed!!\n");
		exit(1);
	}

	svc_run();

	fprintf(stderr, "svc_run() returned.  ERROR has occurred.\n");
	svc_unreg(progNum, VERSNUM);

	return 1;
}

//****************************************//
//***        Remotes Procedures        ***//
//****************************************//
char *simplePing(union u_argument *in)
{
	//printf("*** in Ping Func.\n");
	//Simple function, returns what received
	static int result;
	result = in->varInt;
	return (char *)&result;
}

char *intTestProc(union u_argument *in)
{
	//printf("*** in intTestProc.\n");
	//returns what received
	static int result;
	result = in->varInt;
	//printf("%d\n", result);
	return (char *)&result;
}

char *lngTestProc(union u_argument *in)
{
	//printf("*** in lngTestProc.\n");
	//returns what received
	static long result;
	result = in->lng;
	//printf("%ld\n", result);
	return (char *)&result;
}

char *dblTestProc(union u_argument *in)
{
	//printf("*** in dblTestProc.\n");
	//returns what received
	static double result;
	result = in->dbl;
	//printf("%lf\n", result);
	return (char *)&result;
}

char *strTestProc(union u_argument *in)
{
	//printf("*** in strTestProc.\n");
	//returns what received
	static char *result;
	result = in->str;
	//printf("%s\n", result);
	return (char *)&result;
}

//****************************************//
//***       Dispatch Function          ***//
//****************************************//
static void exm_proc(struct svc_req *rqstp, SVCXPRT * transp)
{
	char *result;
	xdrproc_t xdr_argument;
	xdrproc_t xdr_result;
	char *(*proc) (union u_argument *);

	switch (rqstp->rq_proc) {
	case PROCSIMPLEPING:
		{
			xdr_argument = (xdrproc_t) xdr_int;
			xdr_result = (xdrproc_t) xdr_int;
			proc = (char *(*)(union u_argument *))simplePing;
			break;
		}
	case INTPROCNUM:
		{
			xdr_argument = (xdrproc_t) xdr_int;
			xdr_result = (xdrproc_t) xdr_int;
			proc = (char *(*)(union u_argument *))intTestProc;
			break;
		}
	case DBLPROCNUM:
		{
			xdr_argument = (xdrproc_t) xdr_double;
			xdr_result = (xdrproc_t) xdr_double;
			proc = (char *(*)(union u_argument *))dblTestProc;
			break;
		}
	case LNGPROCNUM:
		{
			xdr_argument = (xdrproc_t) xdr_long;
			xdr_result = (xdrproc_t) xdr_long;
			proc = (char *(*)(union u_argument *))lngTestProc;
			break;
		}
	case STRPROCNUM:
		{
			xdr_argument = (xdrproc_t) xdr_wrapstring;
			xdr_result = (xdrproc_t) xdr_wrapstring;
			proc = (char *(*)(union u_argument *))strTestProc;
			break;
		}
	default:
		{
			//Proc is unavaible
			svcerr_noproc(transp);
			return;
		}
	}
	memset((char *)&argument, (int)0, sizeof(argument));
	if (svc_getargs(transp, xdr_argument, (char *)&argument) == FALSE) {
		svcerr_decode(transp);
		return;
	}

	result = (char *)(*proc) ((union u_argument *)&argument);

	if ((result != NULL)
	    && (svc_sendreply(transp, xdr_result, (char *)result) == FALSE)) {
		svcerr_systemerr(transp);
	}
	if (svc_freeargs(transp, xdr_argument, (char *)&argument) == FALSE) {
		(void)fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}
