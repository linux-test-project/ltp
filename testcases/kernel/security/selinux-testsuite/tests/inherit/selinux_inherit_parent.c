/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <selinux/selinux.h>
#include <selinux/context.h>

int main(int argc, char **argv)
{
	char **childargv;
	security_context_t context_s;
	context_t context;
	int rc, fd;

	if (argc != 4) {
		fprintf(stderr, "usage:  %s childdomain testfile childprogram\n", argv[0]);
		exit(1);
	}

	rc = getcon(&context_s);
	if (rc < 0) {
		fprintf(stderr, "%s:  unable to get my context\n", argv[0]);
		exit(1);

	}

	context = context_new(context_s);
	if (!context) {
		fprintf(stderr, "%s:  unable to create context structure\n", argv[0]);
		exit(1);
	}

	if (context_type_set(context, argv[1])) {
		fprintf(stderr, "%s:  unable to set new type\n", argv[0]);
		exit(1);
	}

	freecon(context_s);
	context_s = context_str(context);
	if (!context_s) {
		fprintf(stderr, "%s:  unable to obtain new context string\n", argv[0]);
		exit(1);
	}

	rc = setexeccon(context_s);
	if (rc < 0) {
		fprintf(stderr, "%s:  unable to set exec context to %s\n", argv[0], context_s);
		exit(1);
	}

	fd = open(argv[2], O_RDWR);
	if (fd < 0) {
		perror(argv[2]);
		exit(1);
	}

	childargv = malloc(3*sizeof(char*));
	if (!childargv) {
		fprintf(stderr, "%s:  out of memory\n", argv[0]);
		exit(1);
	}
	childargv[0] = strdup(argv[3]);
	if (!childargv[0]) {
		fprintf(stderr, "%s:  out of memory\n", argv[0]);
		exit(1);
	}
	childargv[1] = malloc(6);
	if (!childargv[1]) {
		fprintf(stderr, "%s:  out of memory\n", argv[0]);
		exit(1);
	}
	sprintf(childargv[1], "%d", fd);
	childargv[2] = NULL;

	execv(childargv[0],childargv);
	perror(childargv[0]);
	exit(1);
}