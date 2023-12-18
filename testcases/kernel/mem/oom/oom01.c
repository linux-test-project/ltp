/*
 * Out Of Memory (OOM)
 *
 * The program is designed to cope with unpredictable like amount and
 * system physical memory, swap size and other VMM technology like KSM,
 * memcg, memory hotplug and so on which may affect the OOM
 * behaviours. It simply increase the memory consumption 3G each time
 * until all the available memory is consumed and OOM is triggered.
 *
 * Copyright (C) 2010-2017  Red Hat, Inc.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem.h"

static void verify_oom(void)
{
	/* we expect mmap to fail before OOM is hit */
	set_sys_tune("overcommit_memory", 2, 1);
	oom(NORMAL, 0, ENOMEM, 0);

	/* with overcommit_memory set to 0 or 1 there's no
	 * guarantee that mmap fails before OOM */
	set_sys_tune("overcommit_memory", 0, 1);
	oom(NORMAL, 0, ENOMEM, 1);

	set_sys_tune("overcommit_memory", 1, 1);
	testoom(0, 0, ENOMEM, 1);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.max_runtime = TST_UNLIMITED_RUNTIME,
	.test_all = verify_oom,
	.skip_in_compat = 1,
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/vm/overcommit_memory", NULL, TST_SR_TBROK},
		{}
	},
};
