// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Cyril Hrubis <chrubis@suse.cz>
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

static void print_help(char *name)
{
	fprintf(stderr, "Usage: %s\n", name);
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		print_help(argv[0]);
		return 1;
	}

	tst_reinit();

	printf("%u\n", tst_remaining_runtime());

	return 0;
}
