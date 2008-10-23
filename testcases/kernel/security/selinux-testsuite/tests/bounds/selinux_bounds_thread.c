/*
 * Copyright (c) 2008 NEC Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <selinux/selinux.h>
#include <selinux/context.h>

static int thread_status = 0;

static void *worker(void *datap)
{
	security_context_t security_context = datap;
	int rc;

	rc = setcon(security_context);
	if (rc < 0)
		thread_status = errno;

	return NULL;
}

int main(int argc, char *argv[])
{
	security_context_t security_context;
	context_t context;
	pthread_t thread;
	int rc;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <new domain>\n", argv[0]);
		return 1;
	}

	rc = getcon(&security_context);
	if (rc < 0) {
		fprintf(stderr, "%s: unable to get my context\n", argv[0]);
		return 1;
	}

	context = context_new(security_context);
	if (!context) {
		fprintf(stderr, "%s: unable to create context structure\n", argv[0]);
		return 1;
	}

	if (context_type_set(context, argv[1])) {
		fprintf(stderr, "%s: unable to set new type\n", argv[0]);
		return 1;
        }

	freecon(security_context);
	security_context = context_str(context);
	if (!security_context) {
		fprintf(stderr, "%s: unable to obtain new context string\n", argv[0]);
		return 1;
	}

	rc = pthread_create(&thread, NULL, worker, security_context);
	if (rc) {
		fprintf(stderr, "%s: unable to kick a new thread\n", argv[0]);
		return 1;
	}

	rc = pthread_join(thread, NULL);
	if (rc) {
		fprintf(stderr, "%s: unable to join its thread\n", argv[0]);
		return 1;
	}

	fprintf(stderr, "%s: setcon('%s') : %s\n",
		argv[0], argv[1], strerror(thread_status));

	return thread_status;
}
