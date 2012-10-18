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
#include <sys/time.h>
#include <tirpc/rpc/rpc.h>

//Standard define
#define PROCNUM 1
#define VERSNUM 1

//Set number of test call
int maxIter;

double average(double *tbl)
{
	//Return average of values in tbl
	int i;
	double rslt = 0;

	for (i = 0; i < maxIter; i++)
	{
		rslt += tbl[i];
	}
	rslt = rslt / maxIter;
	return rslt;
}

double mini(double *tbl)
{
	//Return minimal of values in tbl
	int i;
	double rslt = tbl[0];

	for (i = 0; i < maxIter; i++)
	{
		if (rslt > tbl[i])
			rslt = tbl[i];
	}
	return rslt;
}

double maxi(double *tbl)
{
	//Return maximal of values in tbl
	int i;
	double rslt = tbl[0];

	for (i = 0; i < maxIter; i++)
	{
		if (rslt < tbl[i])
			rslt = tbl[i];
	}
	return rslt;
}

int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//					   argc[2] : Server Program Number
	//					   argc[3] : Number of test call
	//					   other arguments depend on test case

	//run_mode can switch into stand alone program or program launch by shell script
	//1 : stand alone, debug mode, more screen information
	//0 : launch by shell script as test case, only one printf -> result status
	int run_mode = 0;
	int test_status = 0; //Default test result set to FAILED
	int i;
	double *resultTbl;
	struct timeval tv1,tv2;
    struct timezone tz;
    long long diff;
    double rslt;
	int progNum = atoi(argc[2]);
	char nettype[16] = "visible";
	CLIENT *clnt = NULL;
	enum clnt_stat cs;
	int sndVar = 0;
    int recVar = -1;
    struct timeval total_timeout;

	//Test initialisation
    maxIter = atoi(argc[3]);
    resultTbl = (double *)malloc(maxIter * sizeof(double));

    total_timeout.tv_sec = 1;
	total_timeout.tv_usec = 1;

	clnt = clnt_create(argc[1], progNum, VERSNUM, nettype);

	if (clnt == NULL)
	{
		printf("5\n");
		return 5;
	}

	//Call tested function several times
	for (i = 0; i < maxIter; i++)
	{
		//Tic
		gettimeofday(&tv1, &tz);

		//Call function
		cs = clnt_call((CLIENT *)clnt, PROCNUM,
						(xdrproc_t)xdr_int, (char *)&sndVar, // xdr_in
                   		(xdrproc_t)xdr_int, (char *)&recVar, // xdr_out
						total_timeout);

		//Toc
		gettimeofday(&tv2, &tz);

		//Add function execution time (toc-tic)
		diff = (tv2.tv_sec-tv1.tv_sec) * 1000000L + (tv2.tv_usec-tv1.tv_usec);
		rslt = (double)diff / 1000;

    	if (cs == RPC_SUCCESS)
    	{
    		resultTbl[i] = rslt;
    	}
    	else
    	{
    		test_status = 1;
    		clnt_perrno(cs);
    		break;
    	}

    	if (run_mode)
    	{
    		fprintf(stderr, "lf time  = %lf usecn\n", resultTbl[i]);
    	}
	}

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);
	printf("%lf %d\n", average(resultTbl), maxIter);
	printf("%lf\n", mini(resultTbl));
	printf("%lf\n", maxi(resultTbl));

	return test_status;
}
