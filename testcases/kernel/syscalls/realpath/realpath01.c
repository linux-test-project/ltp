// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2018 Petr Vorel <pvorel@suse.cz>
 * Copyright (C) 2018 Michael Moese <mmoese@suse.de>
 *
 * Based on the reproducer posted upstream so other copyrights may apply.
 * Author: Dmitry V. Levin <ldv@altlinux.org>
 * LTP conversion from glibc source: Petr Vorel <pvorel@suse.cz>
 */

/*\
 * CVE-2018-1000001 realpath buffer underflow.
 *
 * Based on glibc test io/tst-getcwd-abspath.c:
 * https://sourceware.org/git/?p=glibc.git;a=commit;h=52a713fdd0a30e1bd79818e2e3c4ab44ddca1a94.
 */

#include "tst_test.h"

#include <errno.h>
#include <stdlib.h>

#define CHROOT_DIR "cve-2018-1000001"

static void setup(void)
{
	SAFE_MKDIR(CHROOT_DIR, 0755);
	SAFE_CHROOT(CHROOT_DIR);
}

static void run(void)
{
	TST_EXP_FAIL_PTR_NULL(realpath(".", NULL), ENOENT);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.tags = (const struct tst_tag[]) {
		{"CVE", "2018-1000001"},
		{}
	}
};
