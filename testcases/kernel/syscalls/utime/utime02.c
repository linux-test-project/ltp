// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *		07/2001 ported by John George
 *   Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that the system call utime() successfully changes the last
 * access and modification times of a file to the current time,
 * under the following constraints:
 * - The times argument is NULL.
 * - The user ID of the process is not "root".
 * - The file is owned by the user ID of the process.
 */

#include <utime.h>
#include <pwd.h>

#include "tst_test.h"
#include "tst_clocks.h"

#define MNT_POINT	"mntpoint"
#define TEMP_FILE	MNT_POINT"/tmp_file"
#define FILE_MODE	0444

#define TEST_USERNAME "nobody"


static void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM(TEST_USERNAME);

	SAFE_TOUCH(TEMP_FILE, FILE_MODE, NULL);
	SAFE_CHOWN(TEMP_FILE, pw->pw_uid, pw->pw_gid);

	tst_res(TINFO, "Switching effective user ID to user: %s", pw->pw_name);

	SAFE_SETEUID(pw->pw_uid);
}

static void run(void)
{
	struct utimbuf utbuf;
	struct stat stat_buf;
	time_t pre_time, post_time;

	utbuf.modtime = tst_get_fs_timestamp() - 5;
	utbuf.actime = utbuf.modtime + 1;
	TST_EXP_PASS_SILENT(utime(TEMP_FILE, &utbuf));
	SAFE_STAT(TEMP_FILE, &stat_buf);

	TST_EXP_EQ_LI(stat_buf.st_atime, utbuf.actime);
	TST_EXP_EQ_LI(stat_buf.st_mtime, utbuf.modtime);

	pre_time = tst_get_fs_timestamp();
	TST_EXP_PASS(utime(TEMP_FILE, NULL), "utime(%s, NULL)", TEMP_FILE);
	if (!TST_PASS)
		return;
	post_time = tst_get_fs_timestamp();
	SAFE_STAT(TEMP_FILE, &stat_buf);

	if (stat_buf.st_mtime < pre_time || stat_buf.st_mtime > post_time)
		tst_res(TFAIL, "utime() did not set expected mtime, "
				"pre_time: %ld, post_time: %ld, st_mtime: %ld",
				pre_time, post_time, stat_buf.st_mtime);

	if (stat_buf.st_atime < pre_time || stat_buf.st_atime > post_time)
		tst_res(TFAIL, "utime() did not set expected atime, "
				"pre_time: %ld, post_time: %ld, st_atime: %ld",
				pre_time, post_time, stat_buf.st_atime);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.mntpoint = MNT_POINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const[]) {
		"vfat",
		"exfat",
		NULL
	}
};
