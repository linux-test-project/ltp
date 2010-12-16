/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <selinux/selinux.h>
#include <selinux/context.h>
#include <sched.h>
#include "test.h"

int clone_fn(void *in)
{
	char **argv = (char **) in;
	execv(argv[3], argv+3);
	perror(argv[3]);
	return -1;
}

int main(int argc, char **argv)
{
	int pid, rc, status, cloneflags;
	security_context_t context_s;
	context_t context;

	if (argc != 4) {
		fprintf(stderr, "usage:  %s cloneflags newdomain program\n", argv[0]);
		exit(-1);
	}

	cloneflags = strtol(argv[1], NULL, 0);
	if (!cloneflags) {
		fprintf(stderr, "invalid clone flags %s\n", argv[1]);
		exit(-1);
	}

	rc = getcon(&context_s);
	if (rc < 0) {
		fprintf(stderr, "%s:  unable to get my context\n", argv[0]);
		exit(-1);

	}

	context = context_new(context_s);
	if (!context) {
		fprintf(stderr, "%s:  unable to create context structure\n", argv[0]);
		exit(-1);
	}

	if (context_type_set(context, argv[2])) {
		fprintf(stderr, "%s:  unable to set new type\n", argv[0]);
		exit(-1);
	}

	freecon(context_s);
	context_s = context_str(context);
	if (!context_s) {
		fprintf(stderr, "%s:  unable to obtain new context string\n", argv[0]);
		exit(-1);
	}

	rc = setexeccon(context_s);
	if (rc < 0) {
		fprintf(stderr, "%s:  unable to set exec context to %s\n", argv[0], context_s);
		exit(-1);
	}
	pid = ltp_clone_quick(cloneflags | SIGCHLD, clone_fn, argv);
	if (pid < 0) {
		perror("clone");
		exit(-1);
	}

	pid = wait(&status);
	if (pid < 0) {
		perror("wait");
		exit(-1);
	}

	if (WIFEXITED(status)) {
		fprintf(stderr, "Child exited with status %d.\n", WEXITSTATUS(status));
		exit(WEXITSTATUS(status));
	}

	if (WIFSTOPPED(status)) {
		fprintf(stderr, "Child stopped by signal %d.\n", WSTOPSIG(status));
		fprintf(stderr, "..This shouldn't happen.\n");
		fprintf(stderr, "..Killing the child.\n");
		rc = kill(pid,SIGKILL);
		if (rc < 0) {
			perror("kill");
			exit(-1);
		}
		exit(-1);
	}

	if (WIFSIGNALED(status)) {
		fprintf(stderr, "Child terminated by signal %d.\n", WTERMSIG(status));
		fprintf(stderr, "..This is consistent with a share permission denial, check the audit message.\n");
		exit(1);
	}

	fprintf(stderr, "Unexpected exit status 0x%x\n", status);
	exit(-1);
}