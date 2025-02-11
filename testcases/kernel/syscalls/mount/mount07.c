// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

/*\
 * It is a basic test for MS_NOSYMFOLLOW mount option and is copied
 * from kernel selftests nosymfollow-test.c.
 *
 * It tests to make sure that symlink traversal fails with ELOOP when
 * 'nosymfollow' is set, but symbolic links can still be created, and
 * readlink(2) and realpath(3) still work properly. It also verifies
 * that statfs(2) correctly returns ST_NOSYMFOLLOW.
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mount.h>
#include <stdbool.h>
#include "tst_test.h"
#include "lapi/mount.h"

#ifndef ST_NOSYMFOLLOW
# define ST_NOSYMFOLLOW 0x2000
#endif

#define MNTPOINT "mntpoint"

static char *test_file;
static char *link_file;
static char *temp_link_file;
static int flag;

static void setup_symlink(void)
{
	int fd;

	fd = SAFE_CREAT(test_file, O_RDWR);
	SAFE_SYMLINK(test_file, link_file);
	SAFE_CLOSE(fd);
	flag = 1;
}

static void test_link_traversal(bool nosymfollow)
{
	if (nosymfollow) {
		TST_EXP_FAIL2(open(link_file, 0, O_RDWR), ELOOP,
				"open(%s, 0, O_RDWR)", link_file);
	} else {
		TST_EXP_FD(open(link_file, 0, O_RDWR));
	}

	if (TST_RET > 0)
		SAFE_CLOSE(TST_RET);
}

static void test_readlink(void)
{
	char buf[4096];

	memset(buf, 0, 4096);
	TST_EXP_POSITIVE(readlink(link_file, buf, sizeof(buf)),
			"readlink(%s, buf, %ld)", link_file, sizeof(buf));
	if (strcmp(buf, test_file) != 0) {
		tst_res(TFAIL, "readlink strcmp failed, %s, %s",
				buf, test_file);
	} else {
		tst_res(TPASS, "readlink strcmp succeeded");
	}
}

static void test_realpath(void)
{
	TESTPTR(realpath(link_file, NULL));

	if (!TST_RET_PTR) {
		tst_res(TFAIL | TERRNO, "realpath failed");
		return;
	}

	if (strcmp(TST_RET_PTR, test_file) != 0) {
		tst_res(TFAIL, "realpath strcmp failed, %s, %s",
				(char *)TST_RET_PTR, test_file);
	} else {
		tst_res(TPASS, "realpath strcmp succeeded");
	}
}

static void test_cycle_link(void)
{
	TST_EXP_PASS(symlink(test_file, temp_link_file), "symlink(%s, %s)",
			test_file, temp_link_file);
	TST_EXP_PASS(unlink(temp_link_file));
}

static void test_statfs(bool nosymfollow)
{
	struct statfs buf;

	SAFE_STATFS(MNTPOINT, &buf);
	if (buf.f_flags & ST_NOSYMFOLLOW) {
		tst_res(nosymfollow ? TPASS : TFAIL, "ST_NOSYMFOLLOW set on %s",
				MNTPOINT);
	} else {
		tst_res(nosymfollow ? TFAIL : TPASS, "ST_NOSYMFOLLOW not set on %s",
				MNTPOINT);
	}
}

static void setup(void)
{
	test_file = tst_tmpdir_genpath("%s/test_file", MNTPOINT);
	link_file = tst_tmpdir_genpath("%s/link_file", MNTPOINT);
	temp_link_file = tst_tmpdir_genpath("%s/temp_link_file", MNTPOINT);
}

static void cleanup(void)
{
	if (tst_is_mounted(MNTPOINT))
		SAFE_UMOUNT(MNTPOINT);
}

static void run_tests(bool nosymfollow)
{
	test_link_traversal(nosymfollow);
	test_readlink();
	test_realpath();
	test_cycle_link();
	test_statfs(nosymfollow);
}

static void run(void)
{
	tst_res(TINFO, "Testing behaviour when not setting MS_NOSYMFOLLOW");

	TST_EXP_PASS_SILENT(mount(tst_device->dev, MNTPOINT, tst_device->fs_type,
		0, NULL));
	if (!flag || !strcmp(tst_device->fs_type, "tmpfs"))
		setup_symlink();
	run_tests(false);

	tst_res(TINFO, "Testing behaviour when setting MS_NOSYMFOLLOW");
	TST_EXP_PASS_SILENT(mount(tst_device->dev, MNTPOINT, tst_device->fs_type,
		MS_REMOUNT | MS_NOSYMFOLLOW, NULL));
	run_tests(true);

	SAFE_UMOUNT(MNTPOINT);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_root = 1,
	.min_kver = "5.10",
	.format_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const []){
		"exfat",
		"vfat",
		"ntfs",
		NULL
	},
};
