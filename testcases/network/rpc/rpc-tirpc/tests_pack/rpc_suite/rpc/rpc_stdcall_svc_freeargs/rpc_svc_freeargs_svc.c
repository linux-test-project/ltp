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
#include "rpc.h"

//Standard define
#define VERSNUM 1
#define PROCSIMPLEPING	1
#define SVCGETCALLTEST	2

void rcp_service(register struct svc_req *rqstp, register SVCXPRT * transp);
char *simplePing(int inVar, SVCXPRT * transp);

static int argument;

//****************************************//
//***           Main Function          ***//
//****************************************//
int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//                                         argc[2] : Server Program Number
	//                                         other arguments depend on test case

	//run_mode can switch into stand alone program or program launch by shell script
	//1 : stand alone, debug mode, more screen information
	//0 : launch by shell script as test case, only one printf -> result status
	int run_mode = 0;
	int progNum = atoi(argc[1]);
	SVCXPRT *transpUDP = NULL;

	//Initialization
	pmap_unset(progNum, VERSNUM);

	//registerrpc(progNum, VERSNUM, PROCSIMPLEPING,
	//                  simplePing, xdr_int, xdr_int);
	transpUDP = svcudp_create(RPC_ANYSOCK);

	if (run_mode) {
		printf("SVC TCP : %p\n", transpUDP);
	}

	if (!svc_register
	    (transpUDP, progNum, VERSNUM, (void *)rcp_service, IPPROTO_UDP)) {
		fprintf(stderr, "svc_register: error (TCP)\n");
	}

	svc_run();
	fprintf(stderr, "Error: svc_run returned!\n");
	//Test has failed if we are here
	printf("1\n");

	return 1;
}

//****************************************//
//***        Remotes Procedures        ***//
//****************************************//
char *simplePing(int inVar, SVCXPRT * transp)
{
	static int result;
	result = inVar;
	return (char *)&result;
}

//****************************************//
//***       Dispatch Function          ***//
//****************************************//
void rcp_service(register struct svc_req *rqstp, register SVCXPRT * transp)
{
	char *result;
	xdrproc_t xdr_argument;
	xdrproc_t xdr_result;
	char *(*proc) (int, SVCXPRT *);
	int test_status = 1;

	switch (rqstp->rq_proc) {
	case PROCSIMPLEPING:
		{
			//printf("** in PROCPONG dispatch Func.\n");
			xdr_argument = (xdrproc_t) xdr_int;
			xdr_result = (xdrproc_t) xdr_int;
			proc = (char *(*)(int, SVCXPRT *))simplePing;
			break;
		}
	}

	memset((char *)&argument, (int)0, sizeof(argument));
	if (svc_getargs(transp, xdr_argument, (char *)&argument) == FALSE) {
		svcerr_decode(transp);
		return;
	}

	result = (char *)(*proc) (argument, transp);

	if ((result != NULL)
	    && (svc_sendreply(transp, xdr_result, result) == FALSE)) {
		svcerr_systemerr(transp);
	}
	if (svc_freeargs(transp, xdr_argument, (char *)&argument) == FALSE) {
		//Test has failed
		test_status = 1;
	} else {
		//Test succeeds
		test_status = 0;
	}

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	exit(test_status);
}
