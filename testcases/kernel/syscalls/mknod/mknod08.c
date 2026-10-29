// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Verify that mknod(2) succeeds when used to create a filesystem node on a
 * directory without set group-ID bit set. The node created should not have
 * set group-ID bit set and its gid should be equal to that of its parent
 * directory.
 */

#include <pwd.h>
#include "tst_uid.h"
#include "tst_test.h"

#define MODE_RWX 0777
#define MODE_FIFO_RWX (S_IFIFO | 0777)

#define TEMP_DIR "tempdir"
#define TEMP_NODE TEMP_DIR "/testnode"

static uid_t nobody_uid;
static gid_t nobody_gid, free_gid;

static void run(void)
{
	struct stat buf;

	SAFE_MKNOD(TEMP_NODE, MODE_FIFO_RWX, 0);

	SAFE_STAT(TEMP_NODE, &buf);
	TST_EXP_EQ_LI(buf.st_mode & S_ISGID, 0);
	TST_EXP_EQ_LI(buf.st_gid, nobody_gid);

	SAFE_UNLINK(TEMP_NODE);
}

static void setup(void)
{
	struct passwd *ltpuser = SAFE_GETPWNAM("nobody");

	nobody_uid = ltpuser->pw_uid;
	nobody_gid = ltpuser->pw_gid;
	free_gid = tst_get_free_gid(nobody_gid);

	SAFE_MKDIR(TEMP_DIR, MODE_RWX);
	SAFE_CHOWN(TEMP_DIR, nobody_uid, free_gid);
	SAFE_SETGID(nobody_gid);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.needs_tmpdir = 1,
};
