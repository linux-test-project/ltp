// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2011-2017  Red Hat, Inc.
 */

/*\
 * [Description]
 *
 * This is a reproducer of CVE-2011-0999, which fixed by mainline commit
 * a7d6e4ecdb76 ("thp: prevent hugepages during args/env copying into the user stack")
 *
 * "Transparent hugepages can only be created if rmap is fully
 * functional. So we must prevent hugepages to be created while
 * is_vma_temporary_stack() is true."
 *
 * It will cause a panic something like this, if the patch didn't get
 * applied:
 *
 * ```
 * kernel BUG at mm/huge_memory.c:1260!
 * invalid opcode: 0000 [#1] SMP
 * last sysfs file: /sys/devices/system/cpu/cpu23/cache/index2/shared_cpu_map
 * ```
 *
 * Due to commit da029c11e6b1 which reduced the stack size considerably, we
 * now perform a binary search to find the largest possible argument we can
 * use. Only the first iteration of the test performs the search; subsequent
 * iterations use the result of the search which is stored in some shared
 * memory.
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_minmax.h"

#define ARGS_SZ	(256 * 32)

static struct bisection {
	long left;
	long right;
	long mid;
} *bst;

static char *args[ARGS_SZ];
static char *arg;

static void thp_test(void)
{
	long prev_left;
	int pid;

	while (bst->right - bst->left > 1) {
		pid_t pid = SAFE_FORK();

		if (!pid) {
			/* We set mid to left assuming exec will succeed. If
			 * exec fails with E2BIG (and thus returns) then we
			 * restore left and set right to mid instead.
			 */
			prev_left = bst->left;
			bst->mid = (bst->left + bst->right) / 2;
			bst->left = bst->mid;
			args[bst->mid] = NULL;

			TEST(execvp("true", args));
			if (TST_ERR != E2BIG)
				tst_brk(TBROK | TTERRNO, "execvp(\"true\", ...)");
			bst->left = prev_left;
			bst->right = bst->mid;
			exit(0);
		}

		tst_reap_children();
		tst_res(TINFO, "left: %ld, right: %ld, mid: %ld",
			bst->left, bst->right, bst->mid);
	}

	/* We end with mid == right or mid == left where right - left =
	 * 1. Regardless we must use left because right is only set to values
	 * which are too large.
	 */
	pid = SAFE_FORK();
	if (pid == 0) {
		args[bst->left] = NULL;
		TEST(execvp("true", args));
		if (TST_ERR != E2BIG)
			tst_brk(TBROK | TTERRNO, "execvp(\"true\", ...)");
		exit(0);
	}
	tst_reap_children();

	tst_res(TPASS, "system didn't crash.");
}

static void setup(void)
{
	struct rlimit rl = {
		.rlim_cur = RLIM_INFINITY,
		.rlim_max = RLIM_INFINITY,
	};
	int i;
	long arg_len, arg_count;

	bst = SAFE_MMAP(NULL, sizeof(*bst),
			   PROT_READ | PROT_WRITE,
			   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	bst->left = 0;
	bst->right = ARGS_SZ;

	arg_len = sysconf(_SC_PAGESIZE);
	arg = SAFE_MALLOC(arg_len);
	memset(arg, 'c', arg_len - 1);
	arg[arg_len - 1] = '\0';

	args[0] = "true";
	arg_count = ARGS_SZ;
	tst_res(TINFO, "Using %ld args of size %ld", arg_count, arg_len);
	for (i = 1; i < arg_count; i++)
		args[i] = arg;

	SAFE_SETRLIMIT(RLIMIT_STACK, &rl);
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
	.tags = (const struct tst_tag[]) {
		{"linux-git", "a7d6e4ecdb76"},
		{"CVE", "2011-0999"},
		{}
	}
};
