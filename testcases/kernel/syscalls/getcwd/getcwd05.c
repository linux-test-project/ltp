// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2018 Petr Vorel <pvorel@suse.cz>
 * Based on the reproducer posted upstream so other copyrights may apply.
 *
 * Author: Dmitry V. Levin <ldv@altlinux.org>
 * LTP conversion from glibc source: Petr Vorel <pvorel@suse.cz>
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

static void run(unsigned int i)
{
	int fail = 0;

	if (i) {
		tst_res(TINFO, "testing realpath()");
		TESTPTR(realpath(".", NULL));
	} else {
		tst_res(TINFO, "testing getcwd()");
		TESTPTR(getcwd(NULL, 0));
	}

	if (TST_ERR != ENOENT) {
		tst_res(TFAIL | TTERRNO, "returned unexpected errno");
		fail = 1;
	}

	if (TST_RET_PTR != NULL) {
		tst_res(TFAIL, "syscall didn't return NULL: '%s'",
				(char *)TST_RET_PTR);
		fail = 1;
	}

	if (!fail)
		tst_res(TPASS, "bug not reproduced");
}

static struct tst_test test = {
	.test = run,
	.tcnt = 2,
	.setup = setup,
	.needs_root = 1,
	.needs_tmpdir = 1,
};
