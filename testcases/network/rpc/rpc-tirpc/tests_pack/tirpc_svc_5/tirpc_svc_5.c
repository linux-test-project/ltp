/*
* Copyright (c) Bull S.A.  2007 All Rights Reserved.
* Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
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
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include "librpc-tirpc.h"
#include "rpc.h"

#define VERSNUM 1
#define PROCSIMPLEPING	1

struct server_def_t {
	SVCXPRT *transp;
	struct netconfig *nconf;
	char netid[4];
	int domain;
	int type;
	int sock;
	SVCXPRT *(*svc_func)(const int, const u_int, const u_int);
};

static struct server_def_t server_defs[] = {
	{ NULL, NULL, "tcp", AF_INET, SOCK_STREAM, -1, svc_vc_create },
	{ NULL, NULL, "udp", AF_INET, SOCK_DGRAM, -1, svc_dg_create },
};

static int server_instances = sizeof(server_defs) / sizeof(*server_defs);

static void exm_proc(struct svc_req *rqstp, SVCXPRT *transp);

int main(int argn, char *argc[])
{
	int progNum = atoi(argc[1]);
	int i;
	struct server_def_t *this;

	svc_unreg(progNum, VERSNUM);

	for (i = 0; i < server_instances; i++) {
		this = &(server_defs[i]);

		this->nconf = getnetconfigent(this->netid);
		if (this->nconf == NULL) {
			fprintf(stderr, "Cannot get a netconfig entry for %s",
				this->netid);
			goto cleanup;
		}

		this->sock = bound_socket(this->domain, this->type);
		if (this->sock < 0) {
			perror("bound_socket() failed");
			goto cleanup;
		}

		if (this->type == SOCK_STREAM) {
			if (listen(this->sock, 10) < 0) {
				perror("listen() failed");
				goto cleanup;
			}
		}

		this->transp = this->svc_func(this->sock, 1024, 1024);
		if (this->transp == NULL) {
			fprintf(stderr, "Cannot create service.\n");
			goto cleanup;
		}

		if (!svc_reg(this->transp, progNum, VERSNUM,
				exm_proc, this->nconf)) {
			fprintf(stderr, "svc_reg failed!!\n");
			goto cleanup;
		}
	}


	svc_run();

	fprintf(stderr, "svc_run() returned.  ERROR has occurred.\n");

cleanup:
	svc_unreg(progNum, VERSNUM);

	for (i = 0; i < server_instances; i++) {
		this = &(server_defs[i]);

		if (this->transp != NULL)
			svc_destroy(this->transp);

		if (this->sock >= 0)
			close(this->sock);
	}

	return 1;
}

/* Remote Procedures */
char *simplePing(char *in)
{
	static int result = 0;
	result = *in;
	return (char *)&result;
}

/* Dispatch Function */
static void exm_proc(struct svc_req *rqstp, SVCXPRT *transp)
{
	union {
		int varIn;
	} argument;

	char *result;
	xdrproc_t xdr_argument = NULL;
	xdrproc_t xdr_result = NULL;
	char *(*proc) (char *);

	switch (rqstp->rq_proc) {
	case PROCSIMPLEPING:
		{
			xdr_argument = (xdrproc_t) xdr_int;
			xdr_result = (xdrproc_t) xdr_int;
			proc = (char *(*)(char *))simplePing;
			break;
		}
	}
	memset((char *)&argument, (int)0, sizeof(argument));
	if (svc_getargs(transp, xdr_argument, (char *)&argument) == FALSE) {
		svcerr_decode(transp);
		return;
	}

	result = (char *)(*proc) ((char *)&argument);

	if ((result != NULL)
	    && (svc_sendreply(transp, xdr_result, result) == FALSE)) {
		svcerr_systemerr(transp);
	}
	if (svc_freeargs(transp, xdr_argument, (char *)&argument) == FALSE) {
		(void)fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}
