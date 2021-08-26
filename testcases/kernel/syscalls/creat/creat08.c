// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *     Ported from SPIE by Airong Zhang <zhanga@us.ibm.com>
 * Copyright (c) 2021 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * [Description]
 *
 * Verify that the group ID and setgid bit are set correctly when a new file
 * is created.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_uid.h"

#define MODE_RWX        0777
#define MODE_SGID       (S_ISGID|0777)

#define DIR_A		"dir_a"
#define DIR_B		"dir_b"
#define SETGID_A	DIR_A "/setgid"
#define NOSETGID_A	DIR_A "/nosetgid"
#define SETGID_B	DIR_B "/setgid"
#define NOSETGID_B	DIR_B "/nosetgid"
#define ROOT_SETGID	DIR_B "/root_setgid"

static char *tmpdir;
static uid_t orig_uid, nobody_uid;
static gid_t nobody_gid, free_gid;
static int fd = -1;

static void setup(void)
{
	struct passwd *ltpuser = SAFE_GETPWNAM("nobody");

	orig_uid = getuid();
	nobody_uid = ltpuser->pw_uid;
	nobody_gid = ltpuser->pw_gid;
	free_gid = tst_get_free_gid(nobody_gid);
	tmpdir = tst_get_tmpdir();
}

static void file_test(const char *name, mode_t mode, int sgid, gid_t gid)
{
	struct stat buf;

	fd = SAFE_CREAT(name, mode);
	SAFE_STAT(name, &buf);
	SAFE_CLOSE(fd);

	if (buf.st_gid != gid) {
		tst_res(TFAIL, "%s: Incorrect group, %u != %u", name,
			buf.st_gid, gid);
	} else {
		tst_res(TPASS, "%s: Owned by correct group", name);
	}

	if (sgid < 0) {
		tst_res(TINFO, "%s: Skipping setgid bit check", name);
		return;
	}

	if (buf.st_mode & S_ISGID)
		tst_res(sgid ? TPASS : TFAIL, "%s: Setgid bit is set", name);
	else
		tst_res(sgid ? TFAIL : TPASS, "%s: Setgid bit not set", name);
}

static void run(void)
{
	struct stat buf;

	/* Create directories and set permissions */
	SAFE_MKDIR(DIR_A, MODE_RWX);
	SAFE_CHOWN(DIR_A, nobody_uid, free_gid);
	SAFE_STAT(DIR_A, &buf);

	if (buf.st_mode & S_ISGID)
		tst_brk(TBROK, "%s: Setgid bit is set", DIR_A);

	if (buf.st_gid != free_gid) {
		tst_brk(TBROK, "%s: Incorrect group, %u != %u", DIR_A,
			buf.st_gid, free_gid);
	}

	SAFE_MKDIR(DIR_B, MODE_RWX);
	SAFE_CHOWN(DIR_B, nobody_uid, free_gid);
	SAFE_CHMOD(DIR_B, MODE_SGID);
	SAFE_STAT(DIR_B, &buf);

	if (!(buf.st_mode & S_ISGID))
		tst_brk(TBROK, "%s: Setgid bit not set", DIR_B);

	if (buf.st_gid != free_gid) {
		tst_brk(TBROK, "%s: Incorrect group, %u != %u", DIR_B,
			buf.st_gid, free_gid);
	}

	/* Switch to user nobody and create two files in DIR_A */
	/* Both files should inherit GID from the process */
	SAFE_SETGID(nobody_gid);
	SAFE_SETREUID(-1, nobody_uid);
	file_test(NOSETGID_A, MODE_RWX, 0, nobody_gid);
	file_test(SETGID_A, MODE_SGID, 1, nobody_gid);

	/* Create two files in DIR_B and validate owner and permissions */
	/* Both files should inherit GID from the parent directory */
	file_test(NOSETGID_B, MODE_RWX, 0, free_gid);
	/*
	 * CVE 2018-13405 (privilege escalation using setgid bit) has its
	 * own test, skip setgid check here
	 */
	file_test(SETGID_B, MODE_SGID, -1, free_gid);

	/* Switch back to root UID and create a file in DIR_B */
	/* The file should inherid GID from parent directory */
	SAFE_SETREUID(-1, orig_uid);
	file_test(ROOT_SETGID, MODE_SGID, 1, free_gid);

	/* Cleanup between loops */
	tst_purge_dir(tmpdir);
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);

	free(tmpdir);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_tmpdir = 1,
};
