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
#include <sys/socket.h>
#include <errno.h>
#include "rpc.h"

//Standard define
#define VERSNUM 1
#define PROCSIMPLEPING	1

char *simplePing_proc(char *i_var)
{
	static int result = 0;
	result = *i_var;
	return (char *)&result;
}

int main(int argn, char *argc[])
{
	//Server parameter is : argc[1] : Server Program Number
	//                                          others arguments depend on server program
	int run_mode = 0;
	int progNum = atoi(argc[1]);
	char *simplePing_proc();
	bool_t rslt;
	char nettype[16] = "visible";

	if (run_mode) {
		printf("Prog Num : %d\n", progNum);
	}

	svc_unreg(progNum, VERSNUM);

	rslt =
	    rpc_reg(progNum, VERSNUM, PROCSIMPLEPING, (void *)simplePing_proc,
		    (xdrproc_t) xdr_int, (xdrproc_t) xdr_int, nettype);

	svc_run();

	fprintf(stderr, "svc_run() returned.  ERROR has occurred.\n");
	svc_unreg(progNum, VERSNUM);

	return 1;
}
