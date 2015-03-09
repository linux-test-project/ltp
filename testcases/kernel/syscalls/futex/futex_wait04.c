/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * Based on futextest (futext_wait_uninitialized_heap.c)
 * written by KOSAKI Motohiro <kosaki.motohiro@jp.fujitsu.com>
 *
 * Licensed under the GNU GPLv2 or later.
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
 /*
  * Wait on uninitialized heap. It shold be zero and FUTEX_WAIT should return
  * immediately. This test tests zero page handling in futex code.
  */

#include <errno.h>

#include "test.h"
#include "safe_macros.h"
#include "futextest.h"

const char *TCID="futex_wait04";
const int TST_TOTAL=1;
static struct timespec to = {.tv_sec = 0, .tv_nsec = 10000};

static void verify_futex_wait(void)
{
	int res;
	void *buf;
	size_t pagesize = getpagesize();
	buf = SAFE_MMAP(NULL, NULL, pagesize, PROT_READ|PROT_WRITE,
                        MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);

	res = futex_wait(buf, 1, &to, 0);

	if (res == -1 && errno == EWOULDBLOCK)
		tst_resm(TPASS | TERRNO, "futex_wait() returned %i", res);
	else
		tst_resm(TFAIL | TERRNO, "futex_wait() returned %i", res);

	SAFE_MUNMAP(NULL, buf, pagesize);
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++)
		verify_futex_wait();

	tst_exit();
}
