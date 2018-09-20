// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved. */

#include <stdio.h>
#include "tst_kernel.h"

int main(int argc, const char *argv[])
{
	const char *name;
	int i;

	if (argc < 2) {
		fprintf(stderr, "Please provide kernel driver list\n");
		return 1;
	}

	for (i = 1; (name = argv[i]); ++i) {
		if (tst_check_driver(name)) {
			fprintf(stderr, "%s", name);
			return 1;
		}
	}

	return 0;
}
