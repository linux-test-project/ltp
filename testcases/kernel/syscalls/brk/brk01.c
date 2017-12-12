/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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

		if (TEST_RETURN == -1) {
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
