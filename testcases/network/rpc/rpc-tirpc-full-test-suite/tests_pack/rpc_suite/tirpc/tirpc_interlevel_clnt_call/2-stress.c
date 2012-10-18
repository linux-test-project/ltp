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
#include <tirpc/netconfig.h>
#include <tirpc/rpc/rpc.h>
#include <tirpc/rpc/types.h>
#include <tirpc/rpc/xdr.h>
#include <tirpc/rpc/svc.h>
#include <errno.h>

//Standard define
#define PROCNUM 1
#define VERSNUM 1

int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//					   argc[2] : Server Program Number
	//					   argc[3] : Number of testes function calls
	//					   other arguments depend on test case

	//run_mode can switch into stand alone program or program launch by shell script
	//1 : stand alone, debug mode, more screen information
	//0 : launch by shell script as test case, only one printf -> result status
	int run_mode = 0;
	int test_status = 1; //Default test result set to FAILED
	int progNum = atoi(argc[2]);
    int sndVar = 0;
    int recVar = -1;
    CLIENT *client = NULL;
	struct netconfig *nconf = NULL;
	struct timeval tv;
	enum clnt_stat rslt;
	int nbCall = atoi(argc[3]);
	int nbOk = 0;
	int i;

    //First, test initialization : create client using intermediate level API
	nconf = getnetconfigent("udp");

    if ((struct netconfig *)nconf == NULL)
    {
    	//Test failed
    	printf("5\n");
    	return 5;
    }

    tv.tv_sec = 1;
	tv.tv_usec = 1;

    client = clnt_tp_create_timed(argc[1], progNum,
                                  VERSNUM, (struct netconfig *)nconf, &tv);

    if (client == NULL)
    {
    	printf("5\n");
    	return 5;
    }

	//Call routine
	for (i = 0; i < nbCall; i++)
	{
		rslt = clnt_call(client, PROCNUM,
					 (xdrproc_t)xdr_int, (char *)&sndVar, // xdr_in
                     (xdrproc_t)xdr_int, (char *)&recVar, // xdr_out
                     tv);
    	if (rslt == RPC_SUCCESS)
			nbOk++;
	}

	if (run_mode == 1)
	{
		printf("Aimed : %d\n", nbCall);
		printf("Got : %d\n", nbOk);
	}

	test_status = (nbOk == nbCall) ? 0 : 1;

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	return test_status;
}
