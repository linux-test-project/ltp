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
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <selinux/selinux.h>
#include <selinux/context.h>
#include <string.h>

int main(int argc, char **argv)
{
	int pid, rc, status;
	security_context_t context_s;
	context_t context;
	char *child_argv[3];

	if (argc != 3) {
		fprintf(stderr, "usage:  %s newdomain program\n", argv[0]);
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

	if (context_type_set(context, argv[1])) {
		fprintf(stderr, "%s:  unable to set new type\n", argv[0]);
		exit(-1);
	}

	freecon(context_s);
	context_s = context_str(context);
	if (!context_s) {
		fprintf(stderr, "%s:  unable to obtain new context string\n", argv[0]);
		exit(-1);
	}

	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(-1);
	} else if (pid == 0) {
		signal(SIGTRAP, SIG_IGN);
		rc =  ptrace(PTRACE_TRACEME, 0, 0, 0);
		if (rc < 0) {
			perror("ptrace: PTRACE_TRACEME");
			exit(-1);
		}
		child_argv[0] = strdup(argv[2]);
		child_argv[1] = strdup(context_s);
		child_argv[2] = 0;
		execv(child_argv[0], child_argv);
		perror(child_argv[0]);
		exit(1);
	}

repeat:
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
		rc = getpidcon(pid, &context_s);
		if (rc < 0) {
			perror("getpidcon");
			exit(-1);
		}
		fprintf(stderr, "Child has context %s\n", context_s);
		fprintf(stderr, "..Resuming the child.\n");
		rc = ptrace(PTRACE_CONT,pid,0,0);
		if (rc < 0){
			perror("ptrace: PTRACE_CONT");
			exit(-1);
		}
		goto repeat;
	}

	if (WIFSIGNALED(status)) {
		fprintf(stderr, "Child terminated by signal %d.\n", WTERMSIG(status));
		fprintf(stderr, "..This is consistent with a ptrace permission denial - check the audit message.\n");
		exit(1);
	}

	fprintf(stderr, "Unexpected exit status 0x%x\n", status);
	exit(-1);
}

