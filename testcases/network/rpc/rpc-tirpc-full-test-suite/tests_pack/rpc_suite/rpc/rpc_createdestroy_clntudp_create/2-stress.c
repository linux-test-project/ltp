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
#include <rpcsvc/rusers.h>
#include <utmp.h>
#include <sys/time.h>
#include <netdb.h>

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
	CLIENT *clnt = NULL;
	struct sockaddr_in server_addr;
	struct hostent *hp = NULL;
	struct timeval pertry_timeout;
	int sock = RPC_ANYSOCK;
    int nbCall = atoi(argc[3]);
	int nbOk = 0;
	int i;

	//Test initialization
	if ((hp = gethostbyname(argc[1])) == NULL) {
        fprintf(stderr, "can't get addr for %s\n",argc[1]);
        exit(-1);
    }

    pertry_timeout.tv_sec = 1;
    pertry_timeout.tv_usec = 0;

    bcopy(hp->h_addr, (caddr_t)&server_addr.sin_addr, hp->h_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 0;

	//First of all, create a client
	for (i = 0; i < nbCall; i++)
	{
		clnt = clntudp_create(&server_addr, progNum, VERSNUM, pertry_timeout, &sock);
		if ((CLIENT *)clnt != NULL)
			nbOk++;
	}

	//If we are here, macro call was successful
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
