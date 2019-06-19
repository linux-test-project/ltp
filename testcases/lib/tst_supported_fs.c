// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#include <stdio.h>
#include <string.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_fs.h"

static void usage(void)
{
	fprintf(stderr, "Usage: tst_supported_fs [fs_type]\n");
	fprintf(stderr, "   If fs_type is supported, return 0\n");
	fprintf(stderr, "   If fs_type isn't supported, return 1\n");
	fprintf(stderr, "   If fs_type isn't specified, print the list of supported filesystems\n");
	fprintf(stderr, "   fs_type - a specified filesystem type\n");
}

int main(int argc, char *argv[])
{
	const char *const *filesystems;
	int i;

	if (argc > 2) {
		fprintf(stderr, "Can't specify multiple fs_type\n");
		usage();
		return 2;
	}

	if (argv[1] && !strcmp(argv[1], "-h")) {
		usage();
		return 0;
	}

	if (argv[1])
		return !tst_fs_is_supported(argv[1], 0);

	filesystems = tst_get_supported_fs_types(0);
	for (i = 0; filesystems[i]; i++)
		printf("%s\n", filesystems[i]);

	return 0;
}
