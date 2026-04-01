// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Check that memory marked with MADV_FREE is freed on memory pressure.
 *
 * o Fork a child and move it into a memory cgroup
 *
 * o Allocate pages and fill them with a pattern
 *
 * o Madvise pages with MADV_FREE
 *
 * o Check that madvised pages were not freed immediately
 *
 * o Write to some of the madvised pages again, these must not be freed
 *
 * o Set memory limits
 *   - memory.max = 8MB
 *   - memory.swap.max = 16MB
 *
 *   The reason for doubling the memory.max is to have safe margin
 *   for forking the memory hungy child etc. And the reason to setting
 *   memory.swap.max to twice of that is to give the system chance
 *   to try to free some memory before cgroup OOM kicks in and kills
 *   the memory hungry child.
 *
 * o Run a memory hungry child that allocates memory in loop until it's
 *   killed by cgroup OOM
 *
 * o Once the child is killed the MADV_FREE pages that were not written to
 *   should be freed, the test passes if there is at least one
 */

#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "tst_test.h"
#include "lapi/mmap.h"

static size_t page_size;
static int sleep_between_faults;

static int swap_accounting_enabled;

#define PAGES 128
#define TOUCHED_PAGE1 0
#define TOUCHED_PAGE2 10

#define MEM_LIMIT (8 * 1024 * 1024)
#define SWAP_LIMIT (2 * MEM_LIMIT)

static void memory_pressure_child(void)
{
	size_t i, page_size = getpagesize();
	char *ptr;

	for (;;) {
		ptr = mmap(NULL, 500 * page_size, PROT_READ | PROT_WRITE,
			   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		for (i = 0; i < 500; i++) {
			ptr[i * page_size] = i % 100;
			usleep(sleep_between_faults);
		}

		/* If swap accounting is disabled exit after process swapped out 100MB */
		if (!swap_accounting_enabled) {
			int swapped;

			SAFE_FILE_LINES_SCANF("/proc/self/status", "VmSwap: %d", &swapped);

			if (swapped > 100 * 1024)
				exit(0);
		}

	}

	abort();
}

static int count_freed(char *ptr)
{
	int i, ret = 0;

	for (i = 0; i < PAGES; i++) {
		if (!ptr[i * page_size])
			ret++;
	}

	return ret;
}

static int check_page_baaa(char *ptr)
{
	unsigned int i;

	if (ptr[0] != 'b') {
		tst_res(TINFO, "%p unexpected %c (%i) at 0 expected 'b'",
			ptr, isprint(ptr[0]) ? ptr[0] : ' ', ptr[0]);
		return 1;
	}

	for (i = 1; i < page_size; i++) {
		if (ptr[i] != 'a') {
			tst_res(TINFO,
				"%p unexpected %c (%i) at %i expected 'a'",
				ptr, isprint(ptr[i]) ? ptr[i] : ' ',
				ptr[i], i);
			return 1;
		}
	}

	return 0;
}

static int check_page(char *ptr, char val)
{
	unsigned int i;

	for (i = 0; i < page_size; i++) {
		if (ptr[i] != val) {
			tst_res(TINFO,
				"%p unexpected %c (%i) at %i expected %c (%i)",
				ptr, isprint(ptr[i]) ? ptr[i] : ' ', ptr[i], i,
				isprint(val) ? val : ' ', val);
			return 1;
		}
	}

	return 0;
}

static void child(void)
{
	size_t i;
	char *ptr;
	int status, pid, retries = 0;

	SAFE_CG_PRINTF(tst_cg, "cgroup.procs", "%d", getpid());

	ptr = SAFE_MMAP(NULL, PAGES * page_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	for (i = 0; i < PAGES * page_size; i++)
		ptr[i] = 'a';

	if (madvise(ptr, PAGES * page_size, MADV_FREE)) {
		if (errno == EINVAL)
			tst_brk(TCONF | TERRNO, "MADV_FREE is not supported");

		tst_brk(TBROK | TERRNO, "MADV_FREE failed");
	}

	if (ptr[page_size] != 'a')
		tst_res(TFAIL, "MADV_FREE pages were freed immediately");
	else
		tst_res(TPASS, "MADV_FREE pages were not freed immediately");

	ptr[TOUCHED_PAGE1 * page_size] = 'b';
	ptr[TOUCHED_PAGE2 * page_size] = 'b';

	SAFE_CG_PRINTF(tst_cg, "memory.max", "%d", MEM_LIMIT);
	tst_res(TINFO, "Setting memory.max to %d bytes", MEM_LIMIT);

	if (swap_accounting_enabled) {
		SAFE_CG_PRINTF(tst_cg, "memory.swap.max", "%d", SWAP_LIMIT);
		tst_res(TINFO, "Setting memory.swap.max to %d bytes", SWAP_LIMIT);
	} else {
		tst_res(TINFO, "memory.swap.max is unavailable, running without SWAP_LIMIT");
	}

	do {
		sleep_between_faults++;

		pid = SAFE_FORK();
		if (!pid)
			memory_pressure_child();

		tst_res(TINFO, "Memory pressure process %i started, try %i", pid, retries);

		SAFE_WAIT(&status);
	} while (retries++ < 10 && count_freed(ptr) == 0);

	char map[PAGES+1];
	unsigned int freed = 0;
	unsigned int corrupted = 0;

	for (i = 0; i < PAGES; i++) {
		char exp_val;

		if (ptr[i * page_size]) {
			exp_val = 'a';
			map[i] = 'p';
		} else {
			exp_val = 0;
			map[i] = '_';
			freed++;
		}

		if (i != TOUCHED_PAGE1 && i != TOUCHED_PAGE2) {
			if (check_page(ptr + i * page_size, exp_val)) {
				map[i] = '?';
				corrupted++;
			}
		} else {
			if (check_page_baaa(ptr + i * page_size)) {
				map[i] = '?';
				corrupted++;
			}
		}
	}
	map[PAGES] = '\0';

	/* Only show memory map if there are issues or for debugging */
	if (corrupted || freed == 0)
		tst_res(TINFO, "Memory map: %s (p=present, _=freed, ?=corrupted)", map);

	if (freed)
		tst_res(TPASS, "Pages MADV_FREE were freed on low memory (%u/%u freed)", freed, PAGES);
	else
		tst_res(TFAIL, "No MADV_FREE page was freed on low memory");

	if (corrupted)
		tst_res(TFAIL, "Found %u corrupted page(s)", corrupted);
	else
		tst_res(TPASS, "All pages have expected content");

	SAFE_MUNMAP(ptr, PAGES * page_size);

	exit(0);
}

static void run(void)
{
	pid_t pid;
	int status;

retry:
	pid = SAFE_FORK();

	if (!pid)
		child();

	SAFE_WAIT(&status);

	/*
	 * Rarely cgroup OOM kills both children not only the one that allocates
	 * memory in loop, hence we retry here if that happens.
	 */
	if (WIFSIGNALED(status)) {
		tst_res(TINFO, "Both children killed, retrying...");
		goto retry;
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) == TCONF)
		tst_brk(TCONF, "MADV_FREE is not supported");

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		tst_brk(TBROK, "Child %s", tst_strstatus(status));
}

static void setup(void)
{
	long swap_total;

	if (SAFE_CG_HAS(tst_cg, "memory.swap.max"))
		swap_accounting_enabled = 1;
	else
		tst_res(TINFO, "Swap accounting is disabled");

	SAFE_FILE_LINES_SCANF("/proc/meminfo", "SwapTotal: %ld", &swap_total);
	if (swap_total <= 0)
		tst_brk(TCONF, "MADV_FREE does not work without swap");

	page_size = getpagesize();
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
	.needs_cgroup_ctrls = (const char *const []){ "memory", NULL },
};
