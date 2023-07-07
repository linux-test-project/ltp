/*
 * Copyright (C) 2012-2017  Red Hat, Inc.
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
 *
 * Description:
 *
 * The case is designed to test min_free_kbytes tunable.
 *
 * The tune is used to control free memory, and system always
 * reserve min_free_kbytes memory at least.
 *
 * Since the tune is not too large or too little, which will
 * lead to the system hang, so I choose two cases, and test them
 * on all overcommit_memory policy, at the same time, compare
 * the current free memory with the tunable value repeatedly.
 *
 * a) default min_free_kbytes with all overcommit memory policy
 * b) 2x default value with all overcommit memory policy
 * c) 5% of MemFree or %2 MemTotal with all overcommit memory policy
 */

#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "lapi/abisize.h"
#include "mem.h"

#define MAP_SIZE (1UL<<20)

volatile int end;
static long default_tune = -1;
static unsigned long total_mem;

static void test_tune(unsigned long overcommit_policy);
static int eatup_mem(unsigned long overcommit_policy);
static void check_monitor(void);
static void sighandler(int signo LTP_ATTRIBUTE_UNUSED);

static void min_free_kbytes_test(void)
{
	int pid, status;
	struct sigaction sa;

	sa.sa_handler = sighandler;
	if (sigemptyset(&sa.sa_mask) < 0)
		tst_brk(TBROK | TERRNO, "sigemptyset");
	sa.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa, NULL) < 0)
		tst_brk(TBROK | TERRNO, "sigaction");

	pid = SAFE_FORK();
	if (pid == 0) {
		/* startup the check monitor */
		check_monitor();
		exit(0);
	}

	test_tune(2);
	test_tune(0);
	test_tune(1);

	SAFE_KILL(pid, SIGUSR1);
	SAFE_WAITPID(pid, &status, WUNTRACED | WCONTINUED);

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		tst_res(TFAIL,
			 "check_monitor child exit with status: %d", status);

	tst_res(TPASS, "min_free_kbytes test pass");
}

static void test_tune(unsigned long overcommit_policy)
{
	int status;
	int pid[3];
	int ret, i;
	unsigned long tune, memfree, memtotal;

	set_sys_tune("overcommit_memory", overcommit_policy, 1);

	for (i = 0; i < 3; i++) {
		/* case1 */
		if (i == 0)
			set_sys_tune("min_free_kbytes", default_tune, 1);
		/* case2 */
		else if (i == 1) {
			set_sys_tune("min_free_kbytes", 2 * default_tune, 1);
			/* case3 */
		} else {
			memfree = SAFE_READ_MEMINFO("MemFree:");
			memtotal = SAFE_READ_MEMINFO("MemTotal:");
			tune = memfree / 20;
			if (tune > (memtotal / 50))
				tune = memtotal / 50;

			set_sys_tune("min_free_kbytes", tune, 1);
		}

		fflush(stdout);
		switch (pid[i] = fork()) {
		case -1:
			tst_brk(TBROK | TERRNO, "fork");
		case 0:
			ret = eatup_mem(overcommit_policy);
			exit(ret);
		}

		SAFE_WAITPID(pid[i], &status, WUNTRACED | WCONTINUED);

		if (overcommit_policy == 2) {
			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
				tst_res(TFAIL,
					 "child unexpectedly failed: %d",
					 status);
		} else if (overcommit_policy == 1) {
			if (!WIFSIGNALED(status) || WTERMSIG(status) != SIGKILL)
#ifdef TST_ABI32
			{
				if (total_mem < 3145728UL)
#endif
					tst_res(TFAIL,
						 "child unexpectedly failed: %d",
						 status);
#ifdef TST_ABI32
				/* in 32-bit system, a process allocate about 3Gb memory at most */
				else
					tst_res(TINFO, "Child can't allocate "
						 ">3Gb memory in 32bit system");
			}
#endif
		} else {
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) != 0) {
					tst_res(TFAIL, "child unexpectedly "
						 "failed: %d", status);
				}
			} else if (!WIFSIGNALED(status) ||
				   WTERMSIG(status) != SIGKILL) {
				tst_res(TFAIL,
					 "child unexpectedly failed: %d",
					 status);
			}
		}
	}
}

static int eatup_mem(unsigned long overcommit_policy)
{
	int ret = 0;
	unsigned long memfree;
	void *addrs;

	memfree = SAFE_READ_MEMINFO("MemFree:");
	printf("memfree is %lu kB before eatup mem\n", memfree);
	while (1) {
		addrs = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE,
			     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if (addrs == MAP_FAILED) {
			if (overcommit_policy != 1 && errno != ENOMEM) {
				perror("mmap");
				ret = -1;
			}
			break;
		}
		memset(addrs, 1, MAP_SIZE);
	}
	memfree = SAFE_READ_MEMINFO("MemFree:");
	printf("memfree is %lu kB after eatup mem\n", memfree);

	return ret;
}

static void check_monitor(void)
{
	unsigned long tune;
	unsigned long memfree;

	while (end) {
		memfree = SAFE_READ_MEMINFO("MemFree:");
		tune = get_sys_tune("min_free_kbytes");

		if (memfree < tune) {
			tst_res(TINFO, "MemFree is %lu kB, "
				 "min_free_kbytes is %lu kB", memfree, tune);
			tst_res(TFAIL, "MemFree < min_free_kbytes");
		}

		sleep(2);
	}
}

static void sighandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	end = 1;
}

static void setup(void)
{
	if (get_sys_tune("panic_on_oom")) {
		tst_brk(TCONF,
			"panic_on_oom is set, disable it to run these testcases");
	}

	total_mem = SAFE_READ_MEMINFO("MemTotal:") + SAFE_READ_MEMINFO("SwapTotal:");

	default_tune = get_sys_tune("min_free_kbytes");
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.max_runtime = TST_UNLIMITED_RUNTIME,
	.setup = setup,
	.test_all = min_free_kbytes_test,
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/vm/overcommit_memory", NULL, TST_SR_TBROK},
		{"/proc/sys/vm/min_free_kbytes", NULL, TST_SR_TBROK},
		{}
	},
};
