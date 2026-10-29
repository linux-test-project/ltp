// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that listmount() raises EPERM when mount point is not accessible.
 */

#define _GNU_SOURCE

#include <pwd.h>
#include "listmount.h"
#include "lapi/stat.h"

#define LISTSIZE 32

static uint64_t root_id;
static gid_t nobody_gid;
static uid_t nobody_uid;

static void run(void)
{
	if (SAFE_FORK())
		return;

	uint64_t list[LISTSIZE];

	SAFE_SETEGID(nobody_gid);
	SAFE_SETEUID(nobody_uid);

	TST_EXP_FAIL(listmount(root_id, 0, list, LISTSIZE, 0), EPERM);

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
};

