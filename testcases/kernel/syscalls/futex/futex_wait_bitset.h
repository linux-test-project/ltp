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

#define TRESHOLD_US 100000

static void verify_futex_wait_bitset(long long wait_us, clock_t clk_id)
{
	struct timespec start, to, end;
	futex_t futex = FUTEX_INITIALIZER;
	u_int32_t bitset = 0xffffffff;
	int flags = clk_id == CLOCK_REALTIME ? FUTEX_CLOCK_REALTIME : 0;

	tst_resm(TINFO, "testing futex_wait_bitset() timeout with %s",
	         clk_id == CLOCK_REALTIME ? "CLOCK_REALTIME" : "CLOCK_MONOTONIC");

	clock_gettime(clk_id, &start);
	to = tst_timespec_add_us(start, wait_us);

	TEST(futex_wait_bitset(&futex, futex, &to, bitset, flags));

	clock_gettime(clk_id, &end);

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "futex_wait_bitset() returned %li, expected -1",
		         TEST_RETURN);
		return;
	}

	if (TEST_ERRNO == ENOSYS) {
		tst_resm(TCONF, "In this kernel, futex() does not support "
			 "FUTEX_WAIT_BITSET operation");
		return;
	}

	if (TEST_ERRNO != ETIMEDOUT) {
		tst_resm(TFAIL | TTERRNO, "expected %s",
		         tst_strerrno(ETIMEDOUT));
		return;
	}

	if (tst_timespec_lt(end, to)) {
		tst_resm(TFAIL,
		         "futex_wait_bitset() woken up prematurely %llius, expected %llius",
			 tst_timespec_diff_us(end, start), wait_us);
		return;
	}

	if (tst_timespec_diff_us(end, to) > TRESHOLD_US) {
		tst_resm(TFAIL,
		         "futex_wait_bitset() waited too long %llius, expected %llius",
			 tst_timespec_diff_us(end, start), wait_us);
		return;
	}

	tst_resm(TPASS, "futex_wait_bitset() waited %llius, expected %llius",
	         tst_timespec_diff_us(end, start), wait_us);
}
