// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that statmount() raises EPERM when mount point is not accessible.
 */

#define _GNU_SOURCE

#include <pwd.h>
#include "statmount.h"
#include "lapi/stat.h"

static struct statmount *st_mount;
static uint64_t root_id;
static gid_t nobody_gid;
static uid_t nobody_uid;

static void run(void)
{
	if (SAFE_FORK())
		return;

	SAFE_SETEGID(nobody_gid);
	SAFE_SETEUID(nobody_uid);

	memset(st_mount, 0, sizeof(struct statmount));

	TST_EXP_FAIL(statmount(root_id,	STATMOUNT_SB_BASIC, st_mount,
		sizeof(struct statmount), 0), EPERM);

	exit(0);
}

static void setup(void)
{
	struct ltp_statx sx;
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");
	nobody_gid = pw->pw_gid;
	nobody_uid = pw->pw_uid;

	SAFE_STATX(AT_FDCWD, "/", 0, STATX_MNT_ID_UNIQUE, &sx);
	root_id = sx.data.stx_mnt_id;

	SAFE_CHROOT(tst_tmpdir_path());
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.min_kver = "6.8",
	.bufs = (struct tst_buffers []) {
		{&st_mount, .size = sizeof(struct statmount)},
		{}
	}
};
