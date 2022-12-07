// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

static void verify_brk(void)
{
	void *cur_brk, *new_brk;
	size_t inc = getpagesize() * 2 - 1;
	unsigned int i;

	if (tst_variant) {
		tst_res(TINFO, "Testing syscall variant");
		cur_brk = (void *)tst_syscall(__NR_brk, 0);
	} else {
		tst_res(TINFO, "Testing libc variant");
		cur_brk = (void *)sbrk(0);

		if (cur_brk == (void *)-1)
			tst_brk(TCONF, "sbrk() not implemented");

		/*
		 * Check if brk itself is implemented: updating to the current break
		 * should be a no-op.
		 */
		if (brk(cur_brk) != 0)
			tst_brk(TCONF, "brk() not implemented");
	}

	for (i = 0; i < 33; i++) {
		switch (i % 3) {
		case 0:
			new_brk = cur_brk + inc;
		break;
		case 1:
			new_brk = cur_brk;
		break;
		case 2:
			new_brk = cur_brk - inc;
		break;
		}

		if (tst_variant) {
			cur_brk = (void *)tst_syscall(__NR_brk, new_brk);
		} else {
			TST_EXP_PASS_SILENT(brk(new_brk), "brk()");
			cur_brk = sbrk(0);
		}

		if (cur_brk != new_brk) {
			tst_res(TFAIL,
				"brk() failed to set address have %p expected %p",
				cur_brk, new_brk);
			return;
		}

		/* Try to write to the newly allocated heap */
		if (i % 3 == 0)
			*((char *)cur_brk) = 0;
	}

	tst_res(TPASS, "brk() works fine");
}

static struct tst_test test = {
	.test_all = verify_brk,
	.test_variants = 2,
};
