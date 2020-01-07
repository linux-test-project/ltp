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

int main(void)
{
	int test_status = 1;
	SVCXPRT *transp = NULL;
	int sock;

	sock = bound_socket(AF_INET, SOCK_STREAM);
	if (sock < 0) {
		perror("bound_socket() failed");
		return 1;
	}

	if (listen(sock, 10) < 0) {
		perror("listen() failed");
		return 1;
	}

	transp = svc_vc_create(sock, 0, 0);
	test_status = ((SVCXPRT *) transp != NULL) ? 0 : 1;

	if (transp != NULL)
		svc_destroy(transp);

	close(sock);

	printf("%d\n", test_status);

	return test_status;
}
