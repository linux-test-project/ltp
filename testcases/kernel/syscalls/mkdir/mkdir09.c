// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) Linux Test Project, 2022
 */

/*\
 * [Description]
 *
 * Create multiple processes which create subdirectories in the
 * same directory multiple times within test time.
 */

#include <stdio.h>
#include <sys/param.h>
#include "tst_test.h"
#include "tst_safe_pthread.h"

#define MNTPOINT	"mntpoint"
#define MODE_RWX	07770
#define DIR_NAME	MNTPOINT "/X.%d"
#define DIR_NAME_GROUP	MNTPOINT "/X.%d.%d"
#define NCHILD		3

static int child_groups = 2;
static int test_time = 1;
static int nfiles = 10;
static volatile int done;

/*
 * Routine which attempts to create directories in the test
 * directory that already exist.
 */
static void test1(int child_num)
{
	int j;
	char tmpdir[MAXPATHLEN];

	while (!done) {
		for (j = 0; j < nfiles; j += NCHILD) {
			sprintf(tmpdir, DIR_NAME, j);
			TST_EXP_FAIL_SILENT(mkdir(tmpdir, MODE_RWX), EEXIST);
			if (!TST_PASS)
				break;
		}
	}
	tst_res(TPASS, "[%d] create dirs that already exist", child_num);
}

/*
 * Child routine which attempts to remove directories from the
 * test directory which do not exist.
 */
static void test2(int child_num)
{
	int j;
	char tmpdir[MAXPATHLEN];

	while (!done) {
		for (j = 1; j < nfiles; j += NCHILD) {
			sprintf(tmpdir, DIR_NAME, j);
			TST_EXP_FAIL_SILENT(rmdir(tmpdir), ENOENT);
			if (!TST_PASS)
				break;
		}
	}
	tst_res(TPASS, "[%d] remove dirs that do not exist", child_num);
}

/*
 * Child routine which creates and deletes directories in the
 * test directory.
 */
static void test3(int child_num)
{
	int j;
	char tmpdir[MAXPATHLEN];

	while (!done) {
		for (j = 2; j < nfiles; j += NCHILD) {
			sprintf(tmpdir, DIR_NAME_GROUP, j, child_num / NCHILD);
			TST_EXP_PASS_SILENT(mkdir(tmpdir, MODE_RWX));
			if (!TST_PASS)
				break;
		}
		for (j = 2; j < nfiles; j += NCHILD) {
			sprintf(tmpdir, DIR_NAME_GROUP, j, child_num / NCHILD);
			TST_EXP_PASS_SILENT(rmdir(tmpdir));
			if (!TST_PASS)
				break;
		}
	}
	tst_res(TPASS, "[%d] create/remove dirs", child_num);
}

static void *child_thread_func(void *arg)
{
	void (*tests[NCHILD])(int) = { test1, test2, test3 };
	int child_num = (long)arg;

	tests[child_num % NCHILD](child_num);

	/* if any thread failed, make other finish as well */
	done = 1;

	return NULL;
}

static void verify_mkdir(void)
{
	pthread_t child_thread[NCHILD * child_groups];
	long i;

	done = 0;
	for (i = 0; i < child_groups * NCHILD; i++) {
		SAFE_PTHREAD_CREATE(&child_thread[i], NULL,
			child_thread_func, (void *)i);
	}

	sleep(test_time);
	done = 1;

	for (i = 0; i < child_groups * NCHILD; i++)
		SAFE_PTHREAD_JOIN(child_thread[i], NULL);
}

static void setup(void)
{
	int j;
	char tmpdir[MAXPATHLEN];

	for (j = 0; j < nfiles; j += NCHILD) {
		sprintf(tmpdir, DIR_NAME, j);
		SAFE_MKDIR(tmpdir, MODE_RWX);
	}
}

static struct tst_test test = {
	.test_all = verify_mkdir,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.setup = setup,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
};
