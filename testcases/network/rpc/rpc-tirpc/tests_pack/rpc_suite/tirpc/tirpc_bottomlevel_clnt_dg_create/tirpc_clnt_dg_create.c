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
#include <unistd.h>
#include "librpc-tirpc.h"
#include "rpc.h"

#define PROCNUM 1
#define VERSNUM 1
#define ADDRBUFSIZE 100

int main(int argn, char *argc[])
{
	/*
	 * argc[1] - HostName or Host IP
	 * argc[2] - Server Program Number
	 */

	int test_status = 1;
	int progNum = atoi(argc[2]);
	CLIENT *client = NULL;
	struct netconfig *nconf = NULL;
	struct netbuf svcaddr;
	char addrbuf[ADDRBUFSIZE];
	int sock;

	nconf = getnetconfigent("udp");
	if (nconf == NULL) {
		fprintf(stderr, "err nconf\n");
		printf("5\n");
		exit(1);
	}

	svcaddr.len = 0;
	svcaddr.maxlen = ADDRBUFSIZE;
	svcaddr.buf = addrbuf;

	if (!rpcb_getaddr(progNum, VERSNUM, nconf, &svcaddr, argc[1])) {
		fprintf(stderr, "rpcb_getaddr failed!!\n");
		printf("5\n");
		exit(1);
	}

	sock = bound_socket(AF_INET, SOCK_DGRAM);
	if (sock < 0) {
		perror("bound_socket() failed");
		exit(1);
	}

	client = clnt_dg_create(sock, &svcaddr,
				progNum, VERSNUM, 1024, 1024);
	test_status = ((CLIENT *) client != NULL) ? 0 : 1;

	if (client != NULL)
		clnt_destroy(client);

	close(sock);

	printf("%d\n", test_status);

	return test_status;
}
