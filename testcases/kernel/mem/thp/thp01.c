/*
 * Copyright (C) 2011-2017  Red Hat, Inc.
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

/* Description:
 *
 * This is a reproducer of CVE-2011-0999, which fixed by mainline commit
 * a7d6e4ecdb7648478ddec76d30d87d03d6e22b31:
 *
 * "Transparent hugepages can only be created if rmap is fully
 * functional. So we must prevent hugepages to be created while
 * is_vma_temporary_stack() is true."
 *
 * It will cause a panic something like this, if the patch didn't get
 * applied:
 *
 * kernel BUG at mm/huge_memory.c:1260!
 * invalid opcode: 0000 [#1] SMP
 * last sysfs file: /sys/devices/system/cpu/cpu23/cache/index2/shared_cpu_map
 * ....
 */

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem.h"

#define ARRAY_SZ	256

static int ps;
static long length;
static char *array[ARRAY_SZ];
static char *arg;
static struct rlimit rl = {
	.rlim_cur = RLIM_INFINITY,
	.rlim_max = RLIM_INFINITY,
};

static void thp_test(void)
{
	int i;
	pid_t pid;

	switch (pid = SAFE_FORK()) {
	case 0:
		memset(arg, 'c', length - 1);
		arg[length - 1] = '\0';
		array[0] = "true";
		for (i = 1; i < ARRAY_SZ - 1; i++)
			array[i] = arg;
		array[ARRAY_SZ - 1] = NULL;
		if (setrlimit(RLIMIT_STACK, &rl) == -1) {
			perror("setrlimit");
			exit(1);
		}
		if (execvp("true", array) == -1) {
			perror("execvp");
			exit(1);
		}
	default:
		tst_reap_children();
	}

	tst_res(TPASS, "system didn't crash, pass.");
}

static void setup(void)
{
	ps = sysconf(_SC_PAGESIZE);
	length = 32 * ps;
	arg = SAFE_MALLOC(length);
}

static void cleanup(void)
{
	free(arg);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = thp_test,
};
