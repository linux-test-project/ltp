/*
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
 *
 ********************************************************************
 * Copyright (C) 2012 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * ********************************************************************
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "mem.h"

#define MAP_SIZE (1UL<<20)

volatile int end;
char *TCID = "min_free_kbytes";
int TST_TOTAL = 1;
static unsigned long default_tune;
static unsigned long orig_overcommit;
static unsigned long total_mem;

static void test_tune(unsigned long overcommit_policy);
static int eatup_mem(unsigned long overcommit_policy);
static void check_monitor(void);
static void sighandler(int signo LTP_ATTRIBUTE_UNUSED);

int main(int argc, char *argv[])
{
	int lc, pid, status;
	struct sigaction sa;

	tst_parse_opts(argc, argv, NULL, NULL);

	sa.sa_handler = sighandler;
	if (sigemptyset(&sa.sa_mask) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "sigemptyset");
	sa.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa, NULL) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "sigaction");

	setup();

	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");

	case 0:
		/* startup the check monitor */
		check_monitor();
		exit(0);
	}

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		test_tune(2);
		test_tune(0);
		test_tune(1);
	}

	if (kill(pid, SIGUSR1) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "kill %d", pid);
	if (waitpid(pid, &status, WUNTRACED | WCONTINUED) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "waitpid");
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		tst_resm(TFAIL,
			 "check_monitor child exit with status: %d", status);

	cleanup();
	tst_exit();
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
			memfree = read_meminfo("MemFree:");
			memtotal = read_meminfo("MemTotal:");
			tune = memfree / 20;
			if (tune > (memtotal / 50))
				tune = memtotal / 50;

			set_sys_tune("min_free_kbytes", tune, 1);
		}

		fflush(stdout);
		switch (pid[i] = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork");
		case 0:
			ret = eatup_mem(overcommit_policy);
			exit(ret);
		}

		if (waitpid(pid[i], &status, WUNTRACED | WCONTINUED) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "waitpid");

		if (overcommit_policy == 2) {
			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
				tst_resm(TFAIL,
					 "child unexpectedly failed: %d",
					 status);
		} else if (overcommit_policy == 1) {
			if (!WIFSIGNALED(status) || WTERMSIG(status) != SIGKILL)
#if __WORDSIZE == 32
			{
				if (total_mem < 3145728UL)
#endif
					tst_resm(TFAIL,
						 "child unexpectedly failed: %d",
						 status);
#if __WORDSIZE == 32
				/* in 32-bit system, a process allocate about 3Gb memory at most */
				else
					tst_resm(TINFO, "Child can't allocate "
						 ">3Gb memory in 32bit system");
			}
#endif
		} else {
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) != 0) {
					tst_resm(TFAIL, "child unexpectedly "
						 "failed: %d", status);
				}
			} else if (!WIFSIGNALED(status) ||
				   WTERMSIG(status) != SIGKILL) {
				tst_resm(TFAIL,
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

	memfree = read_meminfo("MemFree:");
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
	memfree = read_meminfo("MemFree:");
	printf("memfree is %lu kB after eatup mem\n", memfree);

	return ret;
}

static void check_monitor(void)
{
	unsigned long tune;
	unsigned long memfree;

	while (end) {
		memfree = read_meminfo("MemFree:");
		tune = get_sys_tune("min_free_kbytes");

		if (memfree < tune) {
			tst_resm(TINFO, "MemFree is %lu kB, "
				 "min_free_kbytes is %lu kB", memfree, tune);
			tst_resm(TFAIL, "MemFree < min_free_kbytes");
		}

		sleep(2);
	}
}

static void sighandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	end = 1;
}

void setup(void)
{
	tst_require_root();
	if (get_sys_tune("panic_on_oom")) {
		tst_brkm(TCONF, NULL,
			"panic_on_oom is set, disable it to run these testcases");
	}

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	total_mem = read_meminfo("MemTotal:") + read_meminfo("SwapTotal:");

	default_tune = get_sys_tune("min_free_kbytes");
	orig_overcommit = get_sys_tune("overcommit_memory");
}

void cleanup(void)
{
	set_sys_tune("min_free_kbytes", default_tune, 0);
	set_sys_tune("overcommit_memory", orig_overcommit, 0);
}
