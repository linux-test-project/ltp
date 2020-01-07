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
#define PROCNUM 1
#define VERSNUM 1

//Other define
#define NBCASE 3

typedef struct {
	int init_tout;
	int next_tout;
} params;

int eachresult(char *out, struct sockaddr_in *addr)
{
	//Nothing to do for that test
	return 1;
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
	int test_status = 0;	//Default test result set to PASS
	enum clnt_stat rslt;
	char nettype[16] = "visible";
	int sndVar = 0;
	int recVar = -1;
	int i;
	int progNum = atoi(argc[2]);
	params paramList[NBCASE];

	//Test initialization
	paramList[0].init_tout = 1;
	paramList[0].next_tout = 1;
	paramList[1].init_tout = 10;
	paramList[1].next_tout = 10;
	paramList[2].init_tout = 1000;
	paramList[2].next_tout = 1000;

	//Call tested function using all tests cases
	for (i = 0; i < NBCASE; i++) {
		//Debug mode prints
		if (run_mode == 1) {
			printf("Test using values : %d ",
			       paramList[i].init_tout);
			printf("%d", paramList[i].next_tout);
			printf("\n");
		}
		//Call function
		rslt = rpc_broadcast_exp(progNum, VERSNUM, PROCNUM,
					 (xdrproc_t) xdr_int, (char *)&sndVar,
					 (xdrproc_t) xdr_int, (char *)&recVar,
					 (resultproc_t) eachresult,
					 paramList[i].init_tout,
					 paramList[i].next_tout, nettype);

		//Check result
		if (rslt != RPC_TIMEDOUT) {
			//test has failed
			test_status = 1;
			break;
		}
	/**/}

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	return test_status;
}
