/*
 * Copyright (C) 2012  Red Hat, Inc.
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
#include "test.h"
#include "mem.h"

char *TCID = "swapping01";
int TST_TOTAL = 1;

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

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

#if __WORDSIZE == 32
	tst_brkm(TCONF, NULL, "test is not designed for 32-bit system.");
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		init_meminfo();

		switch (pid = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork");
		case 0:
			do_alloc();
			exit(0);
		default:
			check_swapping();
		}
	}
	cleanup();
	tst_exit();
}

static void init_meminfo(void)
{
	swap_free_init = read_meminfo("SwapFree:");
	if (FILE_LINES_SCANF(cleanup, "/proc/meminfo", "MemAvailable: %ld",
		&mem_available_init)) {
		mem_available_init = read_meminfo("MemFree:")
			+ read_meminfo("Cached:");
	}
	mem_over = mem_available_init * COE_SLIGHT_OVER;
	mem_over_max = mem_available_init * COE_DELTA;

	/* at least 10MB available physical memory needed */
	if (mem_available_init < 10240)
		tst_brkm(TCONF, cleanup, "Not enough available mem to test.");

	if (swap_free_init < mem_over_max)
		tst_brkm(TCONF, cleanup, "Not enough swap space to test.");
}

static void do_alloc(void)
{
	long mem_count;
	void *s;

	tst_resm(TINFO, "available physical memory: %ld MB",
		mem_available_init / 1024);
	mem_count = mem_available_init + mem_over;
	tst_resm(TINFO, "try to allocate: %ld MB", mem_count / 1024);
	s = malloc(mem_count * 1024);
	if (s == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "malloc");
	memset(s, 1, mem_count * 1024);
	tst_resm(TINFO, "memory allocated: %ld MB", mem_count / 1024);
	if (raise(SIGSTOP) == -1)
		tst_brkm(TBROK | TERRNO, tst_exit, "kill");
	free(s);
}

static void check_swapping(void)
{
	int status, i;
	long swap_free_now, swapped;

	/* wait child stop */
	if (waitpid(pid, &status, WUNTRACED) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "waitpid");
	if (!WIFSTOPPED(status))
		tst_brkm(TBROK, cleanup, "child was not stopped.");

	/* Still occupying memory, loop for a while */
	i = 0;
	while (i < 10) {
		swap_free_now = read_meminfo("SwapFree:");
		sleep(1);
		if (abs(swap_free_now - read_meminfo("SwapFree:")) < 512)
			break;

		i++;
	}

	swap_free_now = read_meminfo("SwapFree:");
	swapped = swap_free_init - swap_free_now;
	if (swapped > mem_over_max) {
		kill(pid, SIGCONT);
		tst_brkm(TFAIL, cleanup, "heavy swapping detected: "
				"%ld MB swapped.", swapped / 1024);
	}

	tst_resm(TPASS, "no heavy swapping detected, %ld MB swapped.",
		 swapped / 1024);
	kill(pid, SIGCONT);
	/* wait child exit */
	if (waitpid(pid, &status, 0) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "waitpid");
}

void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

void cleanup(void)
{
}
