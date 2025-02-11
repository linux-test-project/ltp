// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Check that renaming/moving a file from directory where the current user does
 * not have write permissions fails with EACCES.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "tst_test.h"
#include "tst_safe_file_ops.h"
#include "tst_uid.h"

#define SRCDIR   "srcdir"
#define DESTDIR  "destdir"
#define SRCFILE  SRCDIR "/file"
#define DESTFILE DESTDIR "/file"
#define PERMS    0700

static uid_t orig_uid, test_users[2];

static void setup(void)
{
	umask(0);
	orig_uid = getuid();
	tst_get_uids(test_users, 0, 2);
}

static void run(void)
{
	gid_t curgid = getgid();

	SAFE_MKDIR(SRCDIR, PERMS);
	SAFE_TOUCH(SRCFILE, PERMS, NULL);
	SAFE_CHOWN(SRCDIR, test_users[0], curgid);
	SAFE_CHOWN(SRCFILE, test_users[0], curgid);

	SAFE_SETEUID(test_users[1]);
	SAFE_MKDIR(DESTDIR, PERMS);
	SAFE_TOUCH(DESTFILE, PERMS, NULL);

	TST_EXP_FAIL(rename(SRCFILE, DESTFILE), EACCES, "rename()");

	/* Cleanup between loops */
	SAFE_SETEUID(orig_uid);
	tst_purge_dir(tst_tmpdir_path());
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.needs_tmpdir = 1,
};
