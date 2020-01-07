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
#include "rpc.h"

//Standard define
#define VERSNUM 1
#define PROCSIMPLEPING	1

static void exm_proc();

//****************************************//
//***           Main Function          ***//
//****************************************//
int main(int argn, char *argc[])
{
	//Server parameter is : argc[1] : Server Program Number
	//                                          others arguments depend on server program
	int run_mode = 0;
	int progNum = atoi(argc[1]);
	bool_t rslt;

	SVCXPRT *transp = NULL;
	struct netconfig *nconf;
	struct netbuf svcaddr;

	//Initialization
	svc_unreg(progNum, VERSNUM);

	if ((nconf = getnetconfigent("udp")) == NULL) {
		fprintf(stderr, "Cannot get netconfig entry for UDP\n");
		exit(1);
	}

	transp = svc_tli_create(RPC_ANYFD, nconf, NULL, 0, 0);

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
char *simplePing(char *in)
{
	//printf("*** in Ping Func.\n");
	//Simple function, returns what received
	static int result = 0;
	result = *in;
	return (char *)&result;
}

//****************************************//
//***       Dispatch Function          ***//
//****************************************//
static void exm_proc(struct svc_req *rqstp, SVCXPRT * transp)
{
	//printf("* in Dispatch Func.\n");
	union {
		int varIn;
	} argument;

	char *result;
	xdrproc_t xdr_argument;
	xdrproc_t xdr_result;
	char *(*proc) (char *);

	switch (rqstp->rq_proc) {
	case PROCSIMPLEPING:
		{
			//printf("** in PROCPONG dispatch Func.\n");
			xdr_argument = (xdrproc_t) xdr_int;
			xdr_result = (xdrproc_t) xdr_int;
			proc = (char *(*)(char *))simplePing;
			break;
		}
	}
	memset((char *)&argument, (int)0, sizeof(argument));
	if (svc_getargs(transp, xdr_argument, (char *)&argument) == FALSE) {
		svcerr_decode(transp);
		return;
	}

	result = (char *)(*proc) ((char *)&argument);

	if ((result != NULL)
	    && (svc_sendreply(transp, xdr_result, result) == FALSE)) {
		svcerr_systemerr(transp);
	}
	if (svc_freeargs(transp, xdr_argument, (char *)&argument) == FALSE) {
		(void)fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}
