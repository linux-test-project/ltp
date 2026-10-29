// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2019-2022
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SKIP_DELIMITER ','

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_fs.h"
#include "tst_private.h"

#define err_exit(...) ({ \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
	usage(); \
	exit(2); \
})

#define fail_exit(...) ({ \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
	exit(1); \
})

#define info_exit(...) ({ \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
	exit(0); \
})

static void usage(void)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "* all filesystems\n");
	fprintf(stderr, "tst_supported_fs [-s skip_list]\n");
	fprintf(stderr, "   print the list of supported filesystems\n");
	fprintf(stderr, "   if fs_type is supported and not in skip_list (optional),\n"
			"   print list of supported filesystems and return 0\n");
	fprintf(stderr, "   if fs_type isn't supported or in skip_list, return 1\n\n");

	fprintf(stderr, "* single filesystem\n");
	fprintf(stderr, "tst_supported_fs fs_type\n");
	fprintf(stderr, "   if fs_type is supported, return 0 otherwise return 1\n\n");

	fprintf(stderr, "tst_supported_fs -s skip_list fs_type\n");
	fprintf(stderr, "   if fs_type is in skip_list, return 1 otherwise return 0\n\n");

	fprintf(stderr, "tst_supported_fs -s skip_list -d path\n");
	fprintf(stderr, "   if filesystem mounted on path is in skip_list, return 1 otherwise return 0\n\n");

	fprintf(stderr, "fs_type - a specified filesystem type\n");
	fprintf(stderr, "skip_list - filesystems to skip, delimiter: '%c'\n",
			SKIP_DELIMITER);
	fprintf(stderr, "path - any valid file or directory\n");
}

static char **parse_skiplist(char *fs)
{
	char **skiplist;
	int i, cnt = 1;

	for (i = 0; fs[i]; i++) {
		if (optarg[i] == SKIP_DELIMITER)
			cnt++;
	}

	skiplist = malloc(++cnt * sizeof(char *));
	if (!skiplist) {
		fprintf(stderr, "malloc() failed\n");
		return NULL;
	}

	for (i = 0; i < cnt; i++)
		skiplist[i] = strtok_r(fs, TST_TO_STR(SKIP_DELIMITER), &fs);

	return skiplist;
}

int main(int argc, char *argv[])
{
	const char *const *filesystems;
	const char *fsname = NULL;
	int i, ret;
	char **skiplist = NULL;

	while ((ret = getopt(argc, argv, "d:hs:"))) {
		if (ret < 0)
			break;

		switch (ret) {
		case '?':
			usage();
			return 2;

		case 'h':
			usage();
			return 0;

		case 's':
			skiplist = parse_skiplist(optarg);
			if (!skiplist)
				return 2;
			break;

		case 'd':
			if (fsname)
				err_exit("Can't specify multiple paths");

			fsname = tst_fs_type_name(tst_fs_type(optarg));
			break;
		}
	}

	if (fsname && !skiplist)
		err_exit("Parameter -d requires skiplist");

	if (argc - optind > 1)
		err_exit("Can't specify multiple fs_type");

	/* fs_type */
	if (optind < argc) {
		if (fsname)
			err_exit("Can't specify fs_type and -d together");

		fsname = argv[optind];
	}

	if (fsname) {
		if (fsname[0] == '\0')
			err_exit("fs_type is empty");

		if (skiplist) {
			if (tst_fs_in_skiplist(fsname, (const char * const*)skiplist))
				fail_exit("%s is skipped", fsname);

			info_exit("%s is not skipped", fsname);
		}

		if (tst_fs_is_supported(fsname) == TST_FS_UNSUPPORTED)
			fail_exit("%s is not supported", fsname);

		info_exit("%s is supported", fsname);
	}

	/* all filesystems */
	filesystems = tst_get_supported_fs_types((const char * const*)skiplist);

	if (!filesystems[0])
		fail_exit("There are no supported filesystems or all skipped");

	for (i = 0; filesystems[i]; i++)
		printf("%s\n", filesystems[i]);

	return 0;
}
