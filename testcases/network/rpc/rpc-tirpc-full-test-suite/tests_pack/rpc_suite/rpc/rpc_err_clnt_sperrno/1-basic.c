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
* with this program; if not, write the Free Software Foundation, Inc., 59
* Temple Place - Suite 330, Boston MA 02111-1307, USA.
*
* History:
* Created by: Cyril Lacabanne (Cyril.Lacabanne@bull.net)
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <rpc/rpc.h>
#include <errno.h>
#include <time.h>

//Standard define
#define PROCNUM 1
#define VERSNUM 1

int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//					   argc[2] : Server Program Number
	//					   other arguments depend on test case

	//run_mode can switch into stand alone program or program launch by shell script
	//1 : stand alone, debug mode, more screen information
	//0 : launch by shell script as test case, only one printf -> result status
	int run_mode = 0;
	int test_status = 1; //Default test result set to FAILED
	int progNum = atoi(argc[2]);
	char nettype[16] = "udp";
	CLIENT *clnt = NULL;
	enum clnt_stat rslt;
    int recVar = -1;
    struct timeval total_timeout;
    char *chrRslt = NULL;

	if (run_mode == 1)
	{
		printf("Server : %s\n", argc[1]);
		printf("Server # %d\n", progNum);
		printf("Net : %s\n", nettype);
	}

	//Initialisation
	total_timeout.tv_sec = 1;
	total_timeout.tv_usec = 1;/**/

	//First of all, create client using top level API
	clnt = clnt_create(argc[1], progNum, VERSNUM, nettype);

	//Then call remote procedure
	rslt = clnt_call((CLIENT *)clnt, PROCNUM,
						    (xdrproc_t)xdr_int, (char *)&recVar, // xdr_in
                    		(xdrproc_t)xdr_int, (char *)&recVar, // xdr_out
						    total_timeout);	/**/

	chrRslt = clnt_sperrno(rslt);

	//If we are here, test has passed
	test_status = (chrRslt == NULL);

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	return test_status;
}
