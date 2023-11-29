// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2011  Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2012-2022
 * Copyright (c) 2023 Marius Kittler <mkittler@suse.de>
 */

/*\
 * [Description]
 *
 * In the user.* namespace, only regular files and directories can
 * have extended attributes. Otherwise getxattr(2) will return -1
 * and set errno to ENODATA.
 *
 * There are 4 test cases:
 *
 * - Get attribute from a FIFO, setxattr(2) should return -1 and
 *    set errno to ENODATA
 * - Get attribute from a char special file, setxattr(2) should
 *    return -1 and set errno to ENODATA
 * - Get attribute from a block special file, setxattr(2) should
 *    return -1 and set errno to ENODATA
 * - Get attribute from a UNIX domain socket, setxattr(2) should
 *    return -1 and set errno to ENODATA
 */

#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/xattr.h>
#include <stdio.h>
#include <stdlib.h>

#include "tst_res_flags.h"
#include "tst_test.h"
#include "tst_test_macros.h"

#define MNTPOINT "mntpoint"
#define FNAME MNTPOINT"/getxattr02"
#define XATTR_TEST_KEY "user.testkey"

#define FIFO "getxattr02fifo"
#define CHR  "getxattr02chr"
#define BLK  "getxattr02blk"
#define SOCK "getxattr02sock"

static struct test_case {
	const char *desc;
	char *fname;
	int mode;
} tcases[] = {
	{
	 .desc = "get attr from fifo",
	 .fname = FNAME FIFO,
	 .mode = S_IFIFO,
	},
	{
	 .desc = "get attr from char special",
	 .fname = FNAME CHR,
	 .mode = S_IFCHR,
	},
	{
	 .desc = "get attr from block special",
	 .fname = FNAME BLK,
	 .mode = S_IFBLK,
	},
	{
	 .desc = "get attr from UNIX domain socket",
	 .fname = FNAME SOCK,
	 .mode = S_IFSOCK,
	},
};

static void run(unsigned int i)
{
	char buf[BUFSIZ];
	struct test_case *tc = &tcases[i];
	dev_t dev = tc->mode == S_IFCHR ? makedev(1, 3) : 0u;

	if (mknod(tc->fname, tc->mode | 0777, dev) < 0)
		tst_brk(TBROK | TERRNO, "create %s (mode %i) failed (%s)",
				tc->fname, tc->mode, tc->desc);

	TEST(getxattr(tc->fname, XATTR_TEST_KEY, buf, BUFSIZ));
	if (TST_RET == -1 && TST_ERR == ENODATA)
		tst_res(TPASS | TTERRNO, "%s: expected return value",
				tc->desc);
	else
		tst_res(TFAIL | TTERRNO,
				"%s: unexpected return value - expected errno %d - got",
				tc->desc, ENODATA);

	unlink(tc->fname);
}

static void setup(void)
{
	/* assert xattr support in the current filesystem */
	SAFE_TOUCH(FNAME, 0644, NULL);
	TEST(setxattr(FNAME, "user.test", "test", 4, XATTR_CREATE));
	if (TST_ERR == ENOTSUP)
		tst_brk(TCONF,
			"No xattr support in fs or mount without user_xattr option");
	else if (TST_RET != 0)
		tst_brk(TBROK | TTERRNO, "setxattr failed");
}

static struct tst_test test = {
	.all_filesystems = 1,
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.skip_filesystems = (const char *const []) {
			"ramfs",
			"nfs",
			NULL
	},
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases)
};
