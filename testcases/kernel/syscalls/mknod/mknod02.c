// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *     07/2001 Ported by Wayne Boyer
 *   Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that if mknod(2) creates a filesystem node in a directory which
 * does not have the set-group-ID bit set, new node will not inherit the
 * group ownership from its parent directory and its group ID will be the
 * effective group ID of the process.
 */

#include <pwd.h>
#include "tst_test.h"

#define MODE_DIR 0777
#define MODE1	0010777
#define MODE_SGID	02000

#define TEMP_DIR "testdir"
#define TEMP_NODE TEMP_DIR "/testnode"

static struct stat buf;
static struct passwd *user_nobody;
static gid_t gid_nobody;

static void setup(void)
{
	user_nobody = SAFE_GETPWNAM("nobody");
	gid_nobody = user_nobody->pw_gid;

	SAFE_MKDIR(TEMP_DIR, MODE_DIR);
	SAFE_CHOWN(TEMP_DIR, -1, gid_nobody);
}

static void run(void)
{
	TST_EXP_PASS(mknod(TEMP_NODE, MODE1, 0), "mknod(%s, %o, 0)", TEMP_NODE, MODE1);

	SAFE_STAT(TEMP_NODE, &buf);
	TST_EXP_EQ_LI(buf.st_gid, 0);

	SAFE_UNLINK(TEMP_NODE);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.needs_tmpdir = 1
};
