// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>

#include "tst_test.h"

void verify_brk(void)
{
	uintptr_t cur_brk, new_brk;
	uintptr_t inc = getpagesize() * 2 - 1;
	unsigned int i;

	cur_brk = (uintptr_t)sbrk(0);

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

		TEST(brk((void *)new_brk));

		if (TST_RET == -1) {
			tst_res(TFAIL | TERRNO, "brk() failed");
			return;
		}

		cur_brk = (uintptr_t)sbrk(0);

		if (cur_brk != new_brk) {
			tst_res(TFAIL,
				"brk() failed to set address have %p expected %p",
				(void *)cur_brk, (void *)new_brk);
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
};
