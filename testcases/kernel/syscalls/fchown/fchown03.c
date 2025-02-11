// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2001-2021
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * Verify that, fchown(2) succeeds to change the group of a file specified
 * by path when called by non-root user with the following constraints:
 *
 * - euid of the process is equal to the owner of the file
 * - the intended gid is either egid, or one of the supplementary gids
 *   of the process.
 *
 * Also verify that fchown() clears the setuid/setgid bits set on the file.
 */

#include <grp.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "tst_test.h"
#include "compat_tst_16.h"

#define FILE_MODE	(S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define NEW_PERMS	(S_IFREG|S_IRWXU|S_IRWXG|S_ISUID|S_ISGID)
#define FILENAME	"fchown03_testfile"

static int fd;
static struct passwd *ltpuser;

static void check_owner(struct stat *s, uid_t exp_uid, gid_t exp_gid)
{
	if (s->st_uid != exp_uid || s->st_gid != exp_gid)
		tst_res(TFAIL, "%s: wrong owner set to (uid=%d, gid=%d), expected (uid=%d, gid=%d)",
			FILENAME, s->st_uid, s->st_gid, exp_uid, exp_gid);
}

static void check_mode(struct stat *s, mode_t exp_mode)
{
	if (s->st_mode != exp_mode)
		tst_res(TFAIL, "%s: wrong mode permissions %#o, expected %#o",
			FILENAME, s->st_mode, exp_mode);
}

static void run(void)
{
	uid_t uid;
	gid_t gid;
	struct stat stat_buf;

	SAFE_SETEUID(0);
	SAFE_FCHOWN(fd, -1, 0);
	SAFE_FCHMOD(fd, NEW_PERMS);
	SAFE_SETEUID(ltpuser->pw_uid);

	UID16_CHECK((uid = geteuid()), "fchown");
	GID16_CHECK((gid = getegid()), "fchown");

	SAFE_STAT(FILENAME, &stat_buf);
	check_owner(&stat_buf, uid, 0);
	check_mode(&stat_buf, NEW_PERMS);

	TST_EXP_PASS(FCHOWN(fd, -1, gid), "fchown(fd, %d, %d)", -1, gid);
	SAFE_STAT(FILENAME, &stat_buf);
	check_owner(&stat_buf, uid, gid);
	check_mode(&stat_buf, NEW_PERMS & ~(S_ISUID | S_ISGID));
}

static void setup(void)
{
	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEGID(ltpuser->pw_gid);
	SAFE_SETEUID(ltpuser->pw_uid);

	fd = SAFE_OPEN(FILENAME, O_RDWR | O_CREAT, FILE_MODE);
}

static void cleanup(void)
{
	SAFE_SETEGID(0);
	SAFE_SETEUID(0);

	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
};
