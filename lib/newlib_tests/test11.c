/*
 * Copyright (c) 2016 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*
 * Test for segfault.
 */

#include "tst_test.h"

static void do_test(void)
{
	volatile char *ptr = NULL;

	*ptr = 0;

	tst_res(TPASS, "Not reached");
}

static struct tst_test test = {
	.test_all = do_test,
};
