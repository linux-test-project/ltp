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
//Complex procs
#define CALCPROC   10000

void rcp_service(register struct svc_req *rqstp, register SVCXPRT * transp);

struct datas {
	double a;
	double b;
	double c;
} argument;

//XDR Struct function
bool_t xdr_datas(XDR * pt_xdr, struct datas *pt)
{
	return (xdr_double(pt_xdr, &(pt->a)) &&
		xdr_double(pt_xdr, &(pt->b)) && xdr_double(pt_xdr, &(pt->c)));
}

//****************************************//
//***           Main Function          ***//
//****************************************//
int main(int argn, char *argc[])
{
	//Server parameter is : argc[1] : Server Program Number
	//                                          others arguments depend on server program
	int run_mode = 1;
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

	if (run_mode) {
		printf("SVC TCP : %p\n", transpTCP);
		printf("SVC UDP : %p\n", transpUDP);
	}

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

char *calcProc(struct datas *dt, SVCXPRT * svc)
{
	//Makes a + b * c from structure dt and returns double
	//printf("*** In calcProc ***\n");
	static double result = 0;
	result = dt->a + (dt->b * dt->c);
	//printf("Received : %lf, %lf, %lf\n", dt->a, dt->b, dt->c);
	return (char *)&result;
}

//****************************************//
//***       Dispatch Function          ***//
//****************************************//
void rcp_service(register struct svc_req *rqstp, register SVCXPRT * transp)
{
	//printf("* in Dispatch Func.\n");

	char *result;
	xdrproc_t xdr_argument;
	xdrproc_t xdr_result;
	char *(*proc) (struct datas *, SVCXPRT *);

	switch (rqstp->rq_proc) {
	case CALCPROC:
		{
			//printf("** in CALCPROC dispatch Func.\n");
			xdr_argument = (xdrproc_t) xdr_datas;
			xdr_result = (xdrproc_t) xdr_double;
			proc = (char *(*)(struct datas *, SVCXPRT *))calcProc;
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

	result = (char *)(*proc) ((struct datas *)&argument, transp);

	if ((result != NULL)
	    && (svc_sendreply(transp, xdr_result, result) == FALSE)) {
		svcerr_systemerr(transp);
	}
	if (svc_freeargs(transp, xdr_argument, (char *)&argument) == FALSE) {
		(void)fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}
