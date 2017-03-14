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
	long free;
	long nproc;
	long dummy;

	SAFE_FILE_LINES_SCANF("/proc/meminfo", "MemFree: %ld", &free);
	if (FILE_LINES_SCANF("/proc/stat", "processes %ld", &nproc))
		tst_brk(TBROK, "Could not parse processes");
	tst_res(TPASS, "Free: %ld, nproc: %ld", free, nproc);

	if (FILE_LINES_SCANF("/proc/stat", "non-existent %ld", &dummy))
		tst_res(TPASS, "non-existent not found");
	SAFE_FILE_LINES_SCANF("/proc/stat", "non-existent %ld", &dummy);
}

static struct tst_test test = {
	.test_all = do_test,
};
