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
#include "tst_sys_conf.h"

static const char * const save_restore[] = {
	"?/proc/nonexistent",
	"!/proc/sys/kernel/numa_balancing",
	"/proc/sys/kernel/core_pattern",
	NULL,
};

static void setup(void)
{
	SAFE_FILE_PRINTF("/proc/sys/kernel/core_pattern", "changed");
	tst_sys_conf_dump();
}

static void run(void)
{
	tst_res(TPASS, "OK");
}

static struct tst_test test = {
	.needs_root = 1,
	.test_all = run,
	.setup = setup,
	.save_restore = save_restore,
};
