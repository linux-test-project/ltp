/*
 * Copyright (c) 2013-2015 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author:
 * Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 * Test checks following preconditions:
 * since Linux kernel 3.7 it is possible to set extended attributes
 * to cgroup files.
 */

#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "cgroup_xattr";

static const char subdir_name[]	= "test";

#define MAX_SUBSYS		16
#define MAX_OPTIONS_LEN		256
#define MAX_DIR_NAME		64

/* struct to store available mount options */
struct cgrp_option {
	char str[MAX_OPTIONS_LEN];
	char dir[MAX_DIR_NAME];
	int hier;
	int mounted;
	int subdir;
};
static struct cgrp_option cgrp_opt[MAX_SUBSYS];
static int cgrp_opt_num;

struct tst_key {
	const char *name;
	int good;
};

/* only security.* & trusted.* are valid key names */
static struct tst_key tkeys[] = {
	{ .name = "security.",		.good = 0,	},  /* see setup() */
	{ .name = "trusted.test",	.good = 1,	},
	{ .name = "trusted.",		.good = 0,	},  /* see setup() */
	{ .name = "user.",		.good = 0,	},
	{ .name = "system.",		.good = 0,	},
};

#define DEFAULT_VALUE_SIZE	8

/* struct to store key's value */
struct tst_val {
	char *buf;
	size_t size;
};
static struct tst_val val;

/* it fills value's buffer */
static char id;

/*
 * When test breaks, all open dirs should be closed
 * otherwise umount won't succeed
 */
#define MAX_OPEN_DIR		32
static DIR *odir[MAX_OPEN_DIR];
static int odir_num;

/* test options */
static char *narg;
static int nflag;
static int skip_cleanup;
static int verbose;
static const option_t options[] = {
	{"n:", &nflag, &narg},
	{"s", &skip_cleanup, NULL},
	{"v", &verbose, NULL},
	{NULL, NULL, NULL}
};

static void help(void);
static void setup(int argc, char *argv[]);
static void test_run(void);
static void cleanup(void);

static int mount_cgroup(void);
static int set_xattrs(const char *file);
static int get_xattrs(const char *file);
/*
 * set or get xattr recursively
 *
 * @path: start directory
 * @xattr_operation: can be set_xattrs() or get_xattrs()
 */
static int cgrp_files_walking(const char *path,
	int (*xattr_operation)(const char *));

int main(int argc, char *argv[])
{
	setup(argc, argv);

	test_run();

	cleanup();

	tst_exit();
}

static void help(void)
{
	printf("  -n x    Write x bytes to xattr value, default is %d\n",
		DEFAULT_VALUE_SIZE);
	printf("  -s      Skip cleanup\n");
	printf("  -v      Verbose\n");
}

void setup(int argc, char *argv[])
{
	unsigned int i;

	tst_parse_opts(argc, argv, options, help);

	tst_require_root();

	if (access("/proc/cgroups", F_OK) == -1)
		tst_brkm(TCONF, NULL, "Kernel doesn't support cgroups");

	for (i = 0; i < ARRAY_SIZE(tkeys); ++i) {
		if (!strcmp(tkeys[i].name, "security.")) {
			tkeys[i].good = tst_kvercmp(3, 15, 0) < 0;
		} else if (!strcmp(tkeys[i].name, "trusted.")) {
			tkeys[i].good = tst_kvercmp(4, 5, 0) < 0;
		}
	}

	int value_size = DEFAULT_VALUE_SIZE;
	if (nflag) {
		if (sscanf(narg, "%i", &value_size) != 1)
			tst_brkm(TBROK, NULL, "-n option arg is not a number");
		if (value_size <= 0)
			tst_brkm(TBROK, NULL, "-n option arg is less than 1");
	}

	/* initialize test value */
	val.size = value_size;
	val.buf = SAFE_MALLOC(NULL, value_size);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	if (!mount_cgroup())
		tst_brkm(TCONF, cleanup, "Nothing  mounted");
}

static void test_run(void)
{
	int i, set_res = 0, get_res = 0;

	for (i = 0; i < cgrp_opt_num; ++i) {
		if (!cgrp_opt[i].mounted)
			continue;

		SAFE_CHDIR(cleanup, cgrp_opt[i].dir);
		/* reset value */
		id = 0;
		/* set xattr to each file in cgroup fs */
		set_res |= cgrp_files_walking(".", set_xattrs);

		id = 0;
		/* get & check xattr */
		get_res |= cgrp_files_walking(".", get_xattrs);
		SAFE_CHDIR(cleanup, "..");
	}

	/* final results */
	tst_resm(TINFO, "All test-cases have been completed, summary:");
	tst_resm(TINFO, "Set tests result: %s", (set_res) ? "FAIL" : "PASS");
	tst_resm(TINFO, "Get tests result: %s", (get_res) ? "FAIL" : "PASS");
}

static void cleanup(void)
{
	if (val.buf != NULL)
		free(val.buf);

	if (skip_cleanup)
		return;

	/*
	 * Kernels 3.7 can crash while unmounting cgroups with xattr,
	 * call tst_old_flush() to make sure all buffered data written
	 * before it happens
	 */
	tst_old_flush();

	int i;
	for (i = 0; i < odir_num; ++i) {
		SAFE_CLOSEDIR(NULL, odir[i]);
	}

	char *cwd = tst_get_tmpdir();
	SAFE_CHDIR(NULL, cwd);
	free(cwd);

	for (i = 0; i < cgrp_opt_num; ++i) {
		if (cgrp_opt[i].subdir) {
			SAFE_CHDIR(NULL, cgrp_opt[i].dir);
			SAFE_RMDIR(NULL, subdir_name);
			SAFE_CHDIR(NULL, "..");
		}
		if (cgrp_opt[i].mounted) {
			SAFE_UMOUNT(NULL, cgrp_opt[i].dir);
		}
	}

	tst_rmdir();
}

int mount_cgroup(void)
{
	FILE *fd = fopen("/proc/cgroups", "r");
	if (fd == NULL)
		tst_brkm(TBROK, cleanup, "Failed to read /proc/cgroups");
	char str[MAX_DIR_NAME], name[MAX_DIR_NAME];
	int hier = 0, num = 0, enabled = 0, first = 1;
	/* make mount options */
	while ((fgets(str, MAX_DIR_NAME, fd)) != NULL) {
		/* skip first line */
		if (first) {
			first = 0;
			continue;
		}
		if (sscanf(str, "%s\t%d\t%d\t%d",
			name, &hier, &num, &enabled) != 4)
			tst_brkm(TBROK, cleanup, "Can't parse /proc/cgroups");
		if (!enabled)
			continue;

		/* BUG WORKAROUND
		 * Only mount those subsystems, which are not mounted yet.
		 * It's a workaround to a bug when mount doesn't return any err
		 * code while mounting already mounted subsystems, but with
		 * additional "xattr" option. In that case, mount will succeed,
		 * but xattr won't be supported in the new mount anyway.
		 * Should be removed as soon as a fix committed to upstream.
		 *
		 * But not applicable for kernels >= 3.15 where xattr supported
		 * natively.
		 */
		if (hier != 0 && tst_kvercmp(3, 15, 0) < 0)
			continue;

		int i, found = 0;
		for (i = 0; i < cgrp_opt_num; ++i) {
			if (cgrp_opt[i].hier == hier) {
				found = 1;
				break;
			}
		}
		if (!found) {
			i = cgrp_opt_num++;
			cgrp_opt[i].hier = hier;
		}
		char *str = cgrp_opt[i].str;
		if (str[0] == '\0')
			strcpy(str, "xattr");
		snprintf(str + strlen(str), MAX_OPTIONS_LEN - strlen(str),
			",%s", name);
	}
	fclose(fd);

	int i, any_mounted = 0;
	for (i = 0; i < cgrp_opt_num; ++i) {
		char dir[MAX_DIR_NAME];
		struct cgrp_option *opt = &cgrp_opt[i];
		tst_resm(TINFO, "mount options %d: %s (hier = %d)",
			i, opt->str, opt->hier);
		snprintf(opt->dir, MAX_DIR_NAME, "cgx_%d", opt->hier);
		SAFE_MKDIR(cleanup, opt->dir, 0755);

		if (mount(opt->dir, opt->dir, "cgroup", 0, opt->str) == -1) {
			tst_resm(TINFO, "Can't mount: %s", dir);
			continue;
		}

		any_mounted = 1;
		opt->mounted = 1;

		/* create new hierarchy */
		SAFE_CHDIR(cleanup, opt->dir);
		SAFE_MKDIR(cleanup, subdir_name, 0755);
		opt->subdir = 1;
		SAFE_CHDIR(cleanup, "..");
	}
	return any_mounted;
}

static int set_xattrs(const char *file)
{
	unsigned int i, err, fail, res = 0;

	for (i = 0; i < ARRAY_SIZE(tkeys); ++i) {
		err = setxattr(file, tkeys[i].name,
			(const void *)val.buf, val.size, 0) == -1;

		fail = err && tkeys[i].good;
		res |= fail;

		tst_resm((fail) ? TFAIL : TPASS,
			"Expect: %s set xattr key '%s' to file '%s'",
			(tkeys[i].good) ? "can" : "can't",
			tkeys[i].name, file);

		if (verbose && tkeys[i].good)
			tst_resm_hexd(TINFO, val.buf, val.size, "value:");
	}
	return res;
}

static int get_xattrs(const char *file)
{
	unsigned int i, fail, res = 0;

	for (i = 0; i < ARRAY_SIZE(tkeys); ++i) {
		/* get value size */
		ssize_t size = getxattr(file, tkeys[i].name, NULL, 0);
		fail = (size == -1 && tkeys[i].good);
		res |= fail;

		tst_resm((fail) ? TFAIL : TPASS,
			"Expect: %s read xattr %s of file '%s'",
			(tkeys[i].good) ? "can" : "can't",
			tkeys[i].name, file);

		if (fail || size == -1)
			continue;

		/* get xattr value */
		char xval[size];
		if (getxattr(file, tkeys[i].name, xval, size) == -1) {
			tst_brkm(TBROK, cleanup,
				"Can't get buffer of key %s",
				tkeys[i].name);
		}
		fail = val.size != (size_t)size ||
			strncmp(val.buf, xval, val.size) != 0;
		res |= fail;

		tst_resm((fail) ? TFAIL : TPASS, "Expect: values equal");

		if (verbose && fail) {
			tst_resm_hexd(TINFO, xval, size,
				"Read  xattr  value:");
			tst_resm_hexd(TINFO, val.buf, val.size,
				"Expect xattr value:");
		}
	}
	return res;
}

static int cgrp_files_walking(const char *path,
	int (*xattr_operation)(const char *))
{
	int res = 0;
	struct dirent *entry;
	DIR *dir = opendir(path);

	odir[odir_num] = dir;
	if (++odir_num >= MAX_OPEN_DIR) {
		tst_brkm(TBROK, cleanup,
			"Unexpected num of open dirs, max: %d", MAX_OPEN_DIR);
	}

	SAFE_CHDIR(cleanup, path);

	tst_resm(TINFO, "In dir %s", path);

	errno = 0;
	while ((entry = readdir(dir)) != NULL) {
		const char *file = entry->d_name;
		/* skip current and up directories */
		if (!strcmp(file, "..") || !strcmp(file, "."))
			continue;
		struct stat stat_buf;
		TEST(lstat(file, &stat_buf));
		if (TEST_RETURN != -1 && S_ISDIR(stat_buf.st_mode)) {
			/* proceed to subdir */
			res |= cgrp_files_walking(file, xattr_operation);
			tst_resm(TINFO, "In dir %s", path);
		}
		memset(val.buf, id++, val.size);
		res |= xattr_operation(file);
		errno = 0;
	}
	if (errno && !entry) {
		tst_brkm(TWARN | TERRNO, cleanup,
			"Error while reading dir '%s'", path);
	}
	if (closedir(dir) == -1)
		tst_brkm(TWARN, cleanup, "Failed to close dir '%s'", path);
	else
		odir[--odir_num] = NULL;

	if (strcmp(path, "."))
		SAFE_CHDIR(cleanup, "..");
	return res;
}
