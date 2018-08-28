/*
 * Copyright (c) 2018, Linux Test Project
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

#include <stdlib.h>
#include <unistd.h>
#include "tst_test.h"

static void run(void)
{
	do {
		sleep(1);
	} while (tst_timeout_remaining() >= 4);

	tst_res(TPASS, "Timeout remaining: %d", tst_timeout_remaining());
}

static struct tst_test test = {
	.test_all = run,
	.timeout = 5
};
