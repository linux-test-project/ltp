/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
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

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_safe_net.h"

static void cleanup(void)
{
	int i;

	tst_brk(TBROK, "TBROK in cleanup");
	SAFE_OPEN("foo", O_RDWR);
	SAFE_FILE_SCANF("foo", "%i", &i);
	SAFE_TOUCH("doo/foo", 0777, NULL);
	SAFE_FOPEN("foo", "r");
	SAFE_SOCKET(AF_UNIX, SOCK_STREAM, -1);
	tst_res(TINFO, "Test still here");
}

static void do_test(void)
{
	tst_res(TPASS, "Passed");
}

static struct tst_test test = {
	.test_all = do_test,
	.cleanup = cleanup,
};
