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
 */
/*
 * swapping01 - first time swap use results in heavy swapping
 *
 * This case is used for testing upstream commit: 50a1598
 *
 * The upstream commit fixed a issue on s390/x platform that heavy
 * swapping might occur in some condition, however since the patch
 * was quite general, this testcase will be run on all supported
 * platforms to ensure no regression been introduced.
 *
 * Details of the upstream fix:
 * On x86 a page without a mapper is by definition not referenced / old.
 * The s390 architecture keeps the reference bit in the storage key and
 * the current code will check the storage key for page without a mapper.
 * This leads to an interesting effect: the first time an s390 system
 * needs to write pages to swap it only finds referenced pages. This
 * causes a lot of pages to get added and written to the swap device.
 * To avoid this behaviour change page_referenced to query the storage
 * key only if there is a mapper of the page.
 *
 * Test Strategy:
 * Try to allocate memory which size is slightly larger than current
 * available memory. After allocation done, continue loop for a while
 * and calculate the used swap size. The used swap size should be small
 * enough, else it indicates that heavy swapping is occured unexpectedly.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lapi/abisize.h"
#include "mem.h"

/* allow swapping 1 * phy_mem in maximum */
#define COE_DELTA       1
/* will try to alloc 1.3 * phy_mem */
#define COE_SLIGHT_OVER 0.3

static void init_meminfo(void);
static void do_alloc(void);
static void check_swapping(void);

static long mem_available_init;
static long swap_free_init;
static long mem_over;
static long mem_over_max;
static pid_t pid;

static void test_swapping(void)
{
#ifdef TST_ABI32
	tst_brk(TCONF, "test is not designed for 32-bit system.");
#endif

	init_meminfo();

	switch (pid = SAFE_FORK()) {
		case 0:
			do_alloc();
			exit(0);
		default:
			check_swapping();
	}
}

static void init_meminfo(void)
{
	swap_free_init = SAFE_READ_MEMINFO("SwapFree:");
	if (FILE_LINES_SCANF("/proc/meminfo", "MemAvailable: %ld",
		&mem_available_init)) {
		mem_available_init = SAFE_READ_MEMINFO("MemFree:")
			+ SAFE_READ_MEMINFO("Cached:");
	}
	mem_over = mem_available_init * COE_SLIGHT_OVER;
	mem_over_max = mem_available_init * COE_DELTA;

	/* at least 10MB available physical memory needed */
	if (mem_available_init < 10240)
		tst_brk(TCONF, "Not enough available mem to test.");

	if (swap_free_init < mem_over_max)
		tst_brk(TCONF, "Not enough swap space to test.");
}

static void do_alloc(void)
{
	long mem_count;
	void *s;

	tst_res(TINFO, "available physical memory: %ld MB",
		mem_available_init / 1024);
	mem_count = mem_available_init + mem_over;
	tst_res(TINFO, "try to allocate: %ld MB", mem_count / 1024);
	s = SAFE_MALLOC(mem_count * 1024);
	memset(s, 1, mem_count * 1024);
	tst_res(TINFO, "memory allocated: %ld MB", mem_count / 1024);
	if (raise(SIGSTOP) == -1)
		tst_brk(TBROK | TERRNO, "kill");
	free(s);
}

static void check_swapping(void)
{
	int status, i;
	long swap_free_now, swapped;

	/* wait child stop */
	SAFE_WAITPID(pid, &status, WUNTRACED);
	if (!WIFSTOPPED(status))
		tst_brk(TBROK, "child was not stopped.");

	/* Still occupying memory, loop for a while */
	i = 0;
	while (i < 10) {
		swap_free_now = SAFE_READ_MEMINFO("SwapFree:");
		sleep(1);
		if (abs(swap_free_now - SAFE_READ_MEMINFO("SwapFree:")) < 512)
			break;

		i++;
	}

	swap_free_now = SAFE_READ_MEMINFO("SwapFree:");
	swapped = swap_free_init - swap_free_now;
	if (swapped > mem_over_max) {
		kill(pid, SIGCONT);
		tst_brk(TFAIL, "heavy swapping detected: "
				"%ld MB swapped.", swapped / 1024);
	}

	tst_res(TPASS, "no heavy swapping detected, %ld MB swapped.",
		 swapped / 1024);
	kill(pid, SIGCONT);
	/* wait child exit */
	SAFE_WAITPID(pid, &status, 0);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.test_all = test_swapping,
};
