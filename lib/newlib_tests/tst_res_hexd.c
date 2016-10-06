/*
 * Copyright (c) 2016 Linux Test Project
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

#include <stdio.h>
#include "tst_test.h"

static void do_test(void)
{
	char tmp[] = "Hello from tst_res_hexd";

	tst_res_hexd(TPASS, tmp, sizeof(tmp), "%s%d", "dump", 1);
}

static struct tst_test test = {
	.tid = "tst_res_hexd",
	.test_all = do_test,
};
