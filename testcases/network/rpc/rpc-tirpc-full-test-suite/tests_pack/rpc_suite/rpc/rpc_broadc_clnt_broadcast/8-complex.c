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
#include <sys/socket.h>
#include <netdb.h>
#include <rpc/pmap_clnt.h>

//Standard define
#define PROCNUM 1000
#define VERSNUM 1

int currentAnswer;
int maxAnswer;

bool_t eachResult (char *out, struct sockaddr_in *addr)
{
	//printf("in each result\n");
	currentAnswer++;
	if (currentAnswer >= maxAnswer)
	{
		return (1);
	}
	return (0);
}

int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//					   argc[2] : Server Program Number
	//					   argc[3] : Number of host ready to answer to broadcast
	//					   other arguments depend on test case

	//run_mode can switch into stand alone program or program launch by shell script
	//1 : stand alone, debug mode, more screen information
	//0 : launch by shell script as test case, only one printf -> result status
	int run_mode = 0;
	int test_status = 1; //Default test result set to FAILED
	int progNum = atoi(argc[2]);
	enum clnt_stat cs;
	int varSnd = 0;
	int varRec;

	bool_t eachResult (char *out, struct sockaddr_in *addr);
	maxAnswer = atoi(argc[3]);
	currentAnswer = 0;

	//Show information in debug mode...
	if (run_mode == 1)
	{
		printf("progNum : %d\n", progNum);
		printf("Max SVC : %d\n", maxAnswer);
	}

	//Call broadcast routine
	cs = clnt_broadcast(progNum, VERSNUM, PROCNUM,
				   		(xdrproc_t)xdr_int, (char *)&varSnd,
				   		(xdrproc_t)xdr_int, (char *)&varRec,
				   		eachResult);

	if (currentAnswer == maxAnswer)
		test_status = 0;

	if (cs != RPC_SUCCESS)
		clnt_perrno(cs);

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	return test_status;
}
