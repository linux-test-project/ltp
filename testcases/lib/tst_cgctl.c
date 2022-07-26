// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 Canonical Ltd.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "tst_cgroup.h"

static void cgctl_usage(void)
{
	fprintf(stderr, "Usage: tst_cgctl require [controller] [test_pid]\n\tcleanup [config (output of tst_cg_print_config)]\n\tprint\n\thelp\n");
}

static int cgctl_require(const char *ctrl, int test_pid)
{
	struct tst_cg_opts opts;

	memset(&opts, 0, sizeof(opts));
	opts.test_pid = test_pid;

	tst_cg_require(ctrl, &opts);
	tst_cg_print_config();

	return 0;
}

static int cgctl_cleanup(const char *const config)
{
	tst_cg_scan();
	tst_cg_load_config(config);
	tst_cg_cleanup();

	return 0;
}

static int cgctl_print(void)
{
	tst_cg_scan();
	tst_cg_print_config();

	return 0;
}

int main(int argc, char *argv[])
{
	int test_pid;
	const char *cmd_name = argv[1];

	if (argc < 2)
		goto error;

	if (!strcmp(cmd_name, "require")) {
		if (argc != 4)
			goto arg_num_error;
		test_pid = atoi(argv[3]);
		if (!test_pid) {
			fprintf(stderr, "tst_cgctl: Invalid test_pid '%s' given\n",
				argv[3]);
			goto error;
		}
		return cgctl_require(argv[2], test_pid);
	} else if (!strcmp(cmd_name, "cleanup")) {
		if (argc != 3)
			goto arg_num_error;
		return cgctl_cleanup(argv[2]);
	} else if (!strcmp(cmd_name, "print")) {
		return cgctl_print();
	} else if (!strcmp(cmd_name, "help")) {
		cgctl_usage();
		return 0;
	}

	fprintf(stderr, "tst_cgctl: Unknown command '%s' given\n", cmd_name);
	goto error;

arg_num_error:
	fprintf(stderr,
		"tst_cgctl: Invalid number of arguments given for command '%s'\n",
		cmd_name);
error:
	cgctl_usage();
	return 1;
}
