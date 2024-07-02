// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 FUJITSU LIMITED. All Rights Reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that unlink(2) fails with EROFS when target file is on a read-only
 * filesystem.
 */

#include <sys/ioctl.h>
#include "tst_test.h"
#include "lapi/fs.h"

#define MNTPOINT "erofs"
#define FILENAME MNTPOINT"/file"

static void run(void)
{
	TST_EXP_FAIL(unlink(FILENAME), EROFS,
		"%s", "target file in read-only filesystem");
}

static struct tst_test test = {
	.test_all = run,
	.needs_rofs = 1,
	.needs_root = 1,
	.mntpoint = MNTPOINT,
};
