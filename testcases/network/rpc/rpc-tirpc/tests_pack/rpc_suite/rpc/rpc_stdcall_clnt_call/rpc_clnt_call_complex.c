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
#include "rpc.h"

//Standard define
#define CALCPROC 10000
#define VERSNUM  1

#define MAXCALC 100

struct RES {
	double locRes;
	double svcRes;
};

struct datas {
	double a;
	double b;
	double c;
};

bool_t xdr_datas(XDR * pt_xdr, struct datas *pt)
{
	return (xdr_double(pt_xdr, &(pt->a)) &&
		xdr_double(pt_xdr, &(pt->b)) && xdr_double(pt_xdr, &(pt->c)));
}

double getRand(void)
{
	return (drand48() * 1000);
}

int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//                                         argc[2] : Server Program Number
	//                                         other arguments depend on test case

	//run_mode can switch into stand alone program or program launch by shell script
	//1 : stand alone, debug mode, more screen information
	//0 : launch by shell script as test case, only one printf -> result status
	int run_mode = 0;
	int test_status = 0;	//Default test result set to FAILED
	int progNum = atoi(argc[2]);
	char proto[8] = "udp";
	CLIENT *clnt = NULL;
	enum clnt_stat cs;
	struct timeval to;
	int i;
	struct RES resTbl[MAXCALC];
	struct datas vars;

	//Initialization
	to.tv_sec = 1;
	to.tv_usec = 100;

	//First of all, create a client
	clnt = clnt_create(argc[1], progNum, VERSNUM, proto);

	if (run_mode == 1) {
		printf("CLIENT : %p\n", clnt);
		printf("progNum : %d\n", progNum);
		printf("Proto : %s\n", proto);
	}

	if ((CLIENT *) clnt == NULL) {
		clnt_pcreateerror("err");
		printf("1\n");
		return 1;
	}

	for (i = 0; i < MAXCALC; i++) {
		vars.a = getRand();
		vars.b = getRand();
		vars.c = getRand();

		resTbl[i].locRes = vars.a + (vars.b * vars.c);

		cs = clnt_call(clnt, CALCPROC,
			       (xdrproc_t) xdr_datas, (char *)&vars,
			       (xdrproc_t) xdr_double,
			       (char *)&resTbl[i].svcRes, to);

		if (resTbl[i].locRes != resTbl[i].svcRes) {
			test_status = 1;
			break;
		}

		if (run_mode == 1) {
			fprintf(stderr, "value sent : %lf, %lf, %lf\n", vars.a,
				vars.b, vars.c);
			fprintf(stderr, "value localy calculated : %lf\n",
				resTbl[i].locRes);
			fprintf(stderr, "value from server : %lf\n",
				resTbl[i].svcRes);
		}
	}

	if (cs != RPC_SUCCESS)
		clnt_perrno(cs);

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	return test_status;
}
