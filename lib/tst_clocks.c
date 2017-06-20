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

/*
 * clock_gettime() and clock_getres() functions
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>

#include "tst_clocks.h"

int tst_clock_getres(clockid_t clk_id, struct timespec *res)
{
	return syscall(SYS_clock_getres, clk_id, res);
}

int tst_clock_gettime(clockid_t clk_id, struct timespec *ts)
{
	return syscall(SYS_clock_gettime, clk_id, ts);
}
