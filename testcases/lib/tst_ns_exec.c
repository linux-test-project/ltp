// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Red Hat, Inc.
 *               Matus Marhefka <mmarhefk@redhat.com>
 * Copyright (c) Linux Test Project, 2015-2023
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Enters the namespace(s) of a process specified by a PID and then executes
 * the indicated program inside that namespace(s).
 */

#define TST_NO_DEFAULT_MAIN

#include <stdio.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "tst_ns_common.h"

extern struct tst_test *tst_test;

static struct tst_test test = {
	.forks_child = 1, /* Needed by SAFE_CLONE */
};

static int ns_fd[NS_TOTAL];
static int ns_fds;

static void print_help(void)
{
	int i;

	printf("usage: tst_ns_exec <NS_PID> <%s", params[0].name);

	for (i = 1; params[i].name; i++)
		printf("|,%s", params[i].name);

	printf("> <PROGRAM> [ARGS]\nSecond argument indicates the types"
		" of a namespaces maintained by NS_PID\nand is specified"
		" as a comma separated list.\n"
		"Example: tst_ns_exec 1234 net,ipc ip a\n");
}

static void open_ns_fd(const char *pid, const char *ns)
{
	int fd;
	char file_buf[64];

	sprintf(file_buf, "%s/%s/ns/%s", PROC_PATH, pid, ns);

	fd = SAFE_OPEN(file_buf, O_RDONLY);
	ns_fd[ns_fds] = fd;

	++ns_fds;
}

static void close_ns_fd(void)
{
	int i;

	for (i = 0; i < ns_fds; i++)
		SAFE_CLOSE(ns_fd[i]);
}

int main(int argc, char *argv[])
{
	struct tst_clone_args args = { 0, SIGCHLD };
	int i, status, pid;
	char *token;

	tst_test = &test;

	if (argc < 4) {
		print_help();
		return 1;
	}

	memset(ns_fd, 0, sizeof(ns_fd));

	while ((token = strsep(&argv[2], ","))) {
		struct param *p = get_param(token);

		if (!p) {
			printf("Unknown namespace: %s\n", token);
			print_help();
			return 1;
		}

		open_ns_fd(argv[1], token);
	}

	if (!ns_fds) {
		printf("no namespace entries in /proc/%s/ns/\n", argv[1]);
		return 1;
	}

	for (i = 0; i < ns_fds; i++)
		SAFE_SETNS(ns_fd[i], 0);

	pid = SAFE_CLONE(&args);
	if (!pid)
		SAFE_EXECVP(argv[3], argv+3);

	SAFE_WAITPID(pid, &status, 0);

	close_ns_fd();

	if (WIFEXITED(status))
		return WEXITSTATUS(status);

	return 0;
}
