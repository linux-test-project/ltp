// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *    07/2001 ported by John George
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * [Description]
 *
 * Verify that the system call utime() successfully sets the modification
 * and access times of a file to the current time, under the following
 * constraints:
 *
 * - The times argument is NULL.
 * - The user ID of the process is not "root".
 * - The file is not owned by the user ID of the process.
 * - The user ID of the process has write access to the file.
 */

#include <sys/types.h>
#include <pwd.h>
#include <utime.h>
#include <sys/stat.h>
#include <time.h>

#include "tst_test.h"
#include "tst_uid.h"

#define MNTPOINT	"mntpoint"
#define TEMP_FILE	MNTPOINT"/tmp_file"
#define FILE_MODE	0766

static uid_t root_uid, user_uid;

static void setup(void)
{
	struct passwd *pw;
	uid_t test_users[2];
	int fd;

	root_uid = getuid();
	pw = SAFE_GETPWNAM("nobody");
	test_users[0] = pw->pw_uid;
	tst_get_uids(test_users, 1, 2);
	user_uid = test_users[1];

	fd = SAFE_CREAT(TEMP_FILE, FILE_MODE);
	SAFE_CLOSE(fd);

	/* Override umask */
	SAFE_CHMOD(TEMP_FILE, FILE_MODE);
	SAFE_CHOWN(TEMP_FILE, pw->pw_uid, pw->pw_gid);
}

static void run(void)
{
	struct utimbuf utbuf;
	struct stat statbuf;
	time_t mintime, maxtime;

	utbuf.modtime = time(0) - 5;
	utbuf.actime = utbuf.modtime + 1;
	TST_EXP_PASS_SILENT(utime(TEMP_FILE, &utbuf));
	SAFE_STAT(TEMP_FILE, &statbuf);

	if (statbuf.st_atime != utbuf.actime ||
		statbuf.st_mtime != utbuf.modtime) {
		tst_res(TFAIL, "Could not set initial file times");
		return;
	}

	SAFE_SETEUID(user_uid);
	mintime = time(0);
	TST_EXP_PASS(utime(TEMP_FILE, NULL));
	maxtime = time(0);
	SAFE_SETEUID(root_uid);
	SAFE_STAT(TEMP_FILE, &statbuf);

	if (statbuf.st_atime < mintime || statbuf.st_atime > maxtime)
		tst_res(TFAIL, "utime() did not set expected atime");

	if (statbuf.st_mtime < mintime || statbuf.st_mtime > maxtime)
		tst_res(TFAIL, "utime() did not set expected mtime");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const[]) {
		"v9",
		"vfat",
		"exfat",
		NULL
	}
};
