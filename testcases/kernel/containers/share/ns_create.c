// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Red Hat, Inc.
 *               Matus Marhefka <mmarhefk@redhat.com>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Creates a child process in the new specified namespace(s), child is then
 * daemonized and is running in the background. PID of the daemonized child
 * process is printed on the stdout. As the new namespace(s) is(are) maintained
 * by the daemonized child process it(they) can be removed by killing this
 * process.
 */

#define TST_NO_DEFAULT_MAIN

#include <stdio.h>
#include <string.h>
#include "tst_test.h"
#include "ns_common.h"

extern struct tst_test *tst_test;

static struct tst_test test = {
	.forks_child = 1, /* Needed by SAFE_CLONE */
};

static void print_help(void)
{
	int i;

	printf("usage: ns_create <%s", params[0].name);

	for (i = 1; params[i].name; i++)
		printf("|,%s", params[i].name);

	printf(">\n");
}

static void child_fn(void)
{
	int i;

	SAFE_SETSID();
	SAFE_CHDIR("/");

	for (i = 0; i < SAFE_SYSCONF(_SC_OPEN_MAX); i++)
		close(i);

	printf("pausing child\n");
	pause();
}

int main(int argc, char *argv[])
{
	struct tst_clone_args args = { 0, SIGCHLD };
	char *token;
	int pid;

	if (argc < 2) {
		print_help();
		return 1;
	}

	tst_test = &test;

	while ((token = strsep(&argv[1], ","))) {
		struct param *p = get_param(token);

		if (!p) {
			printf("Unknown namespace: %s\n", token);
			print_help();
			return 1;
		}

		args.flags |= p->flag;
	}

	pid = SAFE_CLONE(&args);
	if (!pid) {
		child_fn();
		return 0;
	}

	printf("%d", pid);

	return 0;
}
