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

#include <stdlib.h>
#include <stdio.h>
#include <rpc/rpc.h>
#include <errno.h>

//Standard define
#define PROCNUM 1
#define VERSNUM 667

int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//                                         argc[2] : Server Program Number
	//                                         other arguments depend on test case

	//run_mode can switch into stand alone program or program launch by shell script
	//1 : stand alone, debug mode, more screen information
	//0 : launch by shell script as test case, only one printf -> result status
	int run_mode = 0;
	int test_status = 1;	//Default test result set to FAILED
	int progNum = atoi(argc[2]);
	CLIENT *client = NULL;
	enum clnt_stat cs;
	int var_snd = 0;
	struct timeval tv;
	char nettype[8] = "udp";

	//Initialization
	if (run_mode) {
		printf("Before creation\n");
	}

	tv.tv_sec = 1;
	tv.tv_usec = 0;

	client = clnt_create(argc[1], progNum, VERSNUM, nettype);	/* Call wrong version RP */

	if (client == NULL) {
		clnt_pcreateerror("err");
		printf("%d\n", rpc_createerr.cf_stat);

		return 1;
	}

	cs = clnt_call(client, PROCNUM,
		       (xdrproc_t) xdr_int, (char *)&var_snd,
		       (xdrproc_t) xdr_int, (char *)&var_snd, tv);

	test_status = (cs == RPC_PROGVERSMISMATCH) ? 0 : 1;

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	clnt_destroy(client);

	return test_status;
}
