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

int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//                                         argc[2] : Server Program Number
	//                                         other arguments depend on test case

	int test_status = 0;	//Default test result set to PASS
	int progNum = atoi(argc[2]);
	CLIENT *client = NULL;
	struct netconfig *nconf = NULL;
	struct timeval tv;
	bool_t rslt = 0;
	int fd = 0;
	struct netconfig *tst_nconf;
	unsigned long clver;

	//First, test initialization
	nconf = getnetconfigent("udp");

	if ((struct netconfig *)nconf == NULL) {
		//Test failed
		printf("1\n");
		return 1;
	}

	tv.tv_sec = 10;
	tv.tv_usec = 20;

	client = clnt_tp_create_timed(argc[1], progNum,
				      VERSNUM, (struct netconfig *)nconf, &tv);
	if (client == NULL) {
		//No client creation
		printf("1\n");
		return 1;
	}
	//Call tested function using all tests cases
	rslt =
	    clnt_control(client, CLGET_SVC_ADDR, (struct netbuf *)&tst_nconf);
	if (rslt == 0) {
		test_status = 1;
	}

	rslt = clnt_control(client, CLGET_TIMEOUT, (struct timeval *)&tv);
	if (rslt == 0) {
		test_status = 1;
	}

	rslt = clnt_control(client, CLGET_FD, (int *)&fd);
	if (rslt == 0) {
		test_status = 1;
	}

	rslt = clnt_control(client, CLGET_VERS, (unsigned long *)&clver);
	if (rslt == 0) {
		test_status = 1;
	}
	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	return test_status;
}
