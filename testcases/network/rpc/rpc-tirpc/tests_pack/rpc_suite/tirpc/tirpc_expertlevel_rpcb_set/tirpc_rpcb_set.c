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
#include <netinet/in.h>
#include <errno.h>
#include "rpc.h"

#define PROCNUM 1
#define VERSNUM 1

int main(int argn, char *argc[])
{
	/*
	 * argc[1] - HostName or Host IP
	 * argc[2] - Server Program Number
	 */

	int test_status = 1;
	int progNum = atoi(argc[2]);
	SVCXPRT *transp = NULL;
	struct netconfig *nconf = NULL;

	nconf = getnetconfigent("udp");
	if (nconf == NULL) {
		printf("err nconf\n");
		exit(1);
	}

	transp = svc_tli_create(RPC_ANYFD, nconf, NULL, 0, 0);
	if (transp == NULL) {
		printf("svc_tli_create() failed\n");
		exit(1);
	}

	test_status = !rpcb_set(progNum, VERSNUM, nconf, &(transp->xp_ltaddr));

	if (!rpcb_unset(progNum, VERSNUM, nconf)) {
		printf("rpcb_unset() failed\n");
		exit(1);
	}

	printf("%d\n", test_status);

	return test_status;
}
