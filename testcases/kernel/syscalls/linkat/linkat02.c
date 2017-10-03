/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.
 */
/*
 * Test that linkat() fails and sets the proper errno values.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <sys/mount.h>

#include "test.h"
#include "lapi/syscalls.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"

#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)
#define TEST_FILE	"testfile"
#define TEST_EXIST	"testexist"
#define TEST_ELOOP	"testeloop"
#define TEST_EACCES	"./tmp/testeeacces"
#define TEST_EACCES2	"./tmp/testeeacces2"
#define TEST_EROFS	"mntpoint"
#define TEST_EROFS2	"mntpoint/testerofs2"
#define TEST_EMLINK	"emlink_dir/testfile0"
#define TEST_EMLINK2	"emlink_dir/testfile"
#define BASENAME	"mntpoint/basename"

static char nametoolong[PATH_MAX+2];
static const char *device;
static int mount_flag;
static int max_hardlinks;
static const char *fs_type;

static void setup(void);
static void cleanup(void);
static void setup_eacces(void);
static void cleanup_eacces(void);
static void setup_erofs(void);

static struct test_struct {
	const char *oldfname;
	const char *newfname;
	int flags;
	int expected_errno;
	void (*setupfunc) (void);
	void (*cleanfunc) (void);
} test_cases[] = {
	{TEST_FILE, nametoolong, 0, ENAMETOOLONG, NULL, NULL},
	{nametoolong, TEST_FILE, 0, ENAMETOOLONG, NULL, NULL},
	{TEST_EXIST, TEST_EXIST, 0, EEXIST, NULL, NULL},
	{TEST_ELOOP, TEST_FILE, AT_SYMLINK_FOLLOW, ELOOP, NULL, NULL},
	{TEST_EACCES, TEST_EACCES2, 0, EACCES, setup_eacces, cleanup_eacces},
	{TEST_EROFS, TEST_EROFS2, 0, EROFS, setup_erofs, NULL},
	{TEST_EMLINK, TEST_EMLINK2, 0, EMLINK, NULL, NULL},
};

char *TCID = "linkat02";
int TST_TOTAL = ARRAY_SIZE(test_cases);

static struct passwd *ltpuser;
static void linkat_verify(const struct test_struct *);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			linkat_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void linkat_verify(const struct test_struct *desc)
{
	if (desc->expected_errno == EMLINK && max_hardlinks == 0) {
		tst_resm(TCONF, "EMLINK test is not appropriate");
		return;
	}

	if (desc->setupfunc != NULL) {
		desc->setupfunc();
	}

	TEST(ltp_syscall(__NR_linkat, AT_FDCWD, desc->oldfname,
			 AT_FDCWD, desc->newfname, desc->flags));

	if (desc->cleanfunc != NULL)
		desc->cleanfunc();

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL,
			 "linkat(""AT_FDCWD"", %s, ""AT_FDCWD"", %s, %d)"
			 "succeeded unexpectedly", desc->oldfname,
			 desc->newfname, desc->flags);
		return;
	}

	if (TEST_ERRNO == desc->expected_errno) {
		tst_resm(TPASS | TTERRNO, "linkat failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "linkat failed unexpectedly; expected: "
			 "%d - %s", desc->expected_errno,
			 strerror(desc->expected_errno));
	}
}

static void setup(void)
{
	if ((tst_kvercmp(2, 6, 16)) < 0)
		tst_brkm(TCONF, NULL, "This test needs kernel 2.6.16 or newer");

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to acquire device");

	TEST_PAUSE;

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");

	SAFE_TOUCH(cleanup, TEST_FILE, 0644, NULL);

	memset(nametoolong, 'a', PATH_MAX+1);

	SAFE_TOUCH(cleanup, TEST_EXIST, 0644, NULL);

	SAFE_SYMLINK(cleanup, TEST_ELOOP, "test_file_eloop2");
	SAFE_SYMLINK(cleanup, "test_file_eloop2", TEST_ELOOP);

	SAFE_MKDIR(cleanup, "./tmp", DIR_MODE);
	SAFE_TOUCH(cleanup, TEST_EACCES, 0666, NULL);

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);
	SAFE_MKDIR(cleanup, "mntpoint", DIR_MODE);

	SAFE_MOUNT(cleanup, device, "mntpoint", fs_type, 0, NULL);
	mount_flag = 1;

	max_hardlinks = tst_fs_fill_hardlinks(cleanup, "emlink_dir");
}

static void setup_eacces(void)
{
	SAFE_SETEUID(cleanup, ltpuser->pw_uid);
}

static void cleanup_eacces(void)
{
	SAFE_SETEUID(cleanup, 0);
}

static void setup_erofs(void)
{
	SAFE_MOUNT(cleanup, device, "mntpoint", fs_type,
		   MS_REMOUNT | MS_RDONLY, NULL);
	mount_flag = 1;
}

static void cleanup(void)
{
	if (mount_flag && tst_umount("mntpoint") < 0)
		tst_resm(TWARN | TERRNO, "umount device:%s failed", device);

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
