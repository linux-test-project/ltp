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
#include <utmp.h>
#include "rpc.h"

//Standard define
#define VERSNUM 1
//Simple Ping proc
#define PROCSIMPLEPING	1
//RPC Error proc
#define SVCGETCALLTEST	2
#define PROGSYSERROR	10
#define PROGAUTHERROR	100
#define PROGWKAUTHERROR	101
//DataInt procs
#define INTPROCNUM 1000
#define DBLPROCNUM 2000
#define LNGPROCNUM 3000
#define STRPROCNUM 4000
#define SVCGETARGSPROC 5000

void rcp_service(register struct svc_req *rqstp, register SVCXPRT * transp);

//static int argument;
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
	SVCXPRT *transpTCP = NULL;
	SVCXPRT *transpUDP = NULL;
	//char *simplePing();

	//Initialization
	pmap_unset(progNum, VERSNUM);
	svc_unregister(progNum, VERSNUM);

	//registerrpc(progNum, VERSNUM, PROCSIMPLEPING,
	//                  simplePing, xdr_int, xdr_int);
	transpTCP = svctcp_create(RPC_ANYSOCK, 1000, 1000);
	transpUDP = svcudp_create(RPC_ANYSOCK);

	if (!svc_register
	    (transpTCP, progNum, VERSNUM, (void *)rcp_service, IPPROTO_TCP)) {
		fprintf(stderr, "svc_register: error (TCP)\n");
	}

	if (!svc_register
	    (transpUDP, progNum, VERSNUM, (void *)rcp_service, IPPROTO_UDP)) {
		fprintf(stderr, "svc_register: error (UDP)\n");
	}

	svc_run();
	fprintf(stderr, "Error: svc_run returned!\n");
	exit(1);
}

//****************************************//
//***        Remotes Procedures        ***//
//****************************************//
char *simplePing(union u_argument *inVar, SVCXPRT * transp)
{
	static int result;
	result = inVar->varInt;
	return (char *)&result;
}

char *svc_getcaller_test(union u_argument *inVar, SVCXPRT * transp)
{
	//In this function we test svc_getcaller function basically (simple call)
	struct sockaddr_in *sa = NULL;
	static int result;

	sa = svc_getcaller(transp);
	//If the result is not NULL we consider that function call succeeds
	//so returns 0 (PASS)
	result = (sa != NULL) ? 0 : 1;
	return (char *)&result;
}

char *intTestProc(union u_argument *in, SVCXPRT * transp)
{
	//printf("*** in intTestProc.\n");
	//returns what received
	static int result;
	result = in->varInt;
	//printf("%d\n", result);
	return (char *)&result;
}

char *lngTestProc(union u_argument *in, SVCXPRT * transp)
{
	//printf("*** in lngTestProc.\n");
	//returns what received
	static long result;
	result = in->lng;
	//printf("%ld\n", result);
	return (char *)&result;
}

char *dblTestProc(union u_argument *in, SVCXPRT * transp)
{
	//printf("*** in dblTestProc.\n");
	//returns what received
	static double result;
	result = in->dbl;
	//printf("%lf\n", result);
	return (char *)&result;
}

char *strTestProc(union u_argument *in, SVCXPRT * transp)
{
	//printf("*** in strTestProc.\n");
	//returns what received
	static char *result;
	result = in->str;
	//printf("%s\n", result);
	return (char *)&result;
}

char *svcGetargsProc(union u_argument *in, SVCXPRT * transp)
{
	//printf("*** in svcGetargsProc.\n");
	//returns what received inside this procedure : test svc_getargs function
	union u_argument args;

	static char *result;
	result = in->str;

	if ((svc_getargs(transp, (xdrproc_t) xdr_int, (char *)&args)) == FALSE) {
		svcerr_decode(transp);
		return NULL;
	}
	//printf("%s\n", result);
	return (char *)&result;
}

//****************************************//
//***       Dispatch Function          ***//
//****************************************//
void rcp_service(register struct svc_req *rqstp, register SVCXPRT * transp)
{
	//printf("* in Dispatch Func.\n");
	/*union {
	   int varIn;
	   } argument; */

	char *result;
	xdrproc_t xdr_argument;
	xdrproc_t xdr_result;
	char *(*proc) (union u_argument *, SVCXPRT *);
	enum auth_stat why;

	switch (rqstp->rq_proc) {
	case PROCSIMPLEPING:
		{
			//printf("** in PROCSIMPLEPING dispatch Func.\n");
			xdr_argument = (xdrproc_t) xdr_int;
			xdr_result = (xdrproc_t) xdr_int;
			proc =
			    (char *(*)(union u_argument *, SVCXPRT *))
			    simplePing;
			break;
		}
	case SVCGETCALLTEST:
		{
			//printf("** in SVCGETCALLTEST dispatch Func.\n");
			xdr_argument = (xdrproc_t) xdr_int;
			xdr_result = (xdrproc_t) xdr_int;
			proc =
			    (char *(*)(union u_argument *, SVCXPRT *))
			    svc_getcaller_test;
			break;
		}
	case PROGSYSERROR:
		{
			//printf("** in PROGSYSERROR dispatch Func.\n");
			//Simulate an error
			svcerr_systemerr(transp);
			return;
		}
	case PROGAUTHERROR:
		{
			//printf("** in PROGAUTHERROR dispatch Func.\n");
			//Simulate an authentification error
			svcerr_auth(transp, why);
			return;
		}
	case PROGWKAUTHERROR:
		{
			//printf("** in PROGWKAUTHERROR dispatch Func.\n");
			//Simulate an authentification error
			svcerr_weakauth(transp);
			return;
		}
	case INTPROCNUM:
		{
			//printf("** in INTPROCNUM dispatch Func.\n");
			xdr_argument = (xdrproc_t) xdr_int;
			xdr_result = (xdrproc_t) xdr_int;
			proc =
			    (char *(*)(union u_argument *, SVCXPRT *))
			    intTestProc;
			//(char *(*)(union u_argument *))
			break;
		}
	case DBLPROCNUM:
		{
			//printf("** in DBLPROCNUM dispatch Func.\n");
			xdr_argument = (xdrproc_t) xdr_double;
			xdr_result = (xdrproc_t) xdr_double;
			proc =
			    (char *(*)(union u_argument *, SVCXPRT *))
			    dblTestProc;
			break;
		}
	case LNGPROCNUM:
		{
			//printf("** in LNGPROCNUM dispatch Func.\n");
			xdr_argument = (xdrproc_t) xdr_long;
			xdr_result = (xdrproc_t) xdr_long;
			proc =
			    (char *(*)(union u_argument *, SVCXPRT *))
			    lngTestProc;
			break;
		}
	case STRPROCNUM:
		{
			//printf("** in STRPROCNUM dispatch Func.\n");
			xdr_argument = (xdrproc_t) xdr_wrapstring;
			xdr_result = (xdrproc_t) xdr_wrapstring;
			proc =
			    (char *(*)(union u_argument *, SVCXPRT *))
			    strTestProc;
			break;
		}
	case SVCGETARGSPROC:
		{
			//printf("** in SVCGETARGSPROC dispatch Func.\n");
			xdr_argument = (xdrproc_t) xdr_int;
			xdr_result = (xdrproc_t) xdr_int;
			proc =
			    (char *(*)(union u_argument *, SVCXPRT *))
			    svcGetargsProc;
			break;
		}
	default:
		{
			//printf("** in NOT DEFINED dispatch Func.\n");
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

	result = (char *)(*proc) ((union u_argument *)&argument, transp);

	if ((result != NULL)
	    && (svc_sendreply(transp, xdr_result, result) == FALSE)) {
		svcerr_systemerr(transp);
	}
	if (svc_freeargs(transp, xdr_argument, (char *)&argument) == FALSE) {
		(void)fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}
