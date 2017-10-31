/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * Basic unit test for the tst_strstatus() function.
 */

#include "tst_test.h"

static struct tcase {
	int status;
	const char *str;
} tcases[] = {
	{0x0100, "exited with 1"},
	{0x0001, "killed by SIGHUP"},
	{0x137f, "is stopped"},
	{0xffff, "is resumed"},
	{0xff, "invalid status 0xff"},
};

static void do_test(unsigned int n)
{
	const char *str_status = tst_strstatus(tcases[n].status);

	if (strcmp(str_status, tcases[n].str))
		tst_res(TFAIL, "%s != %s", str_status, tcases[n].str);
	else
		tst_res(TPASS, "%s", str_status);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(tcases),
};
