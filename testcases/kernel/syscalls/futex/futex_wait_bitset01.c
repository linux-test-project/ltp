/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
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
  * 1. Block on a bitset futex and wait for timeout, the difference between
  *    normal futex and bitset futex is that that the later have absolute timeout.
  * 2. Check that the futex waited for expected time.
  */

#include <errno.h>

#include "test.h"
#include "futextest.h"
#include "futex_wait_bitset.h"

const char *TCID="futex_wait_bitset01";
const int TST_TOTAL=1;

#define DEFAULT_TIMEOUT_US 100010

int main(int argc, char *argv[])
{
	int lc;

	tst_timer_check(CLOCK_MONOTONIC);

	tst_parse_opts(argc, argv, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++)
		verify_futex_wait_bitset(DEFAULT_TIMEOUT_US, CLOCK_MONOTONIC);

	tst_exit();
}
