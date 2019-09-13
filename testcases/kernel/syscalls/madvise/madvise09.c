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
 * o Check that madvised pages were not freed immediatelly
 *
 * o Write to some of the madvised pages again, these must not be freed
 *
 * o Set memory limits
 *   - limit_in_bytes = 8MB
 *   - memsw.limit_in_bytes = 16MB
 *
 *   The reason for doubling the limit_in_bytes is to have safe margin
 *   for forking the memory hungy child etc. And the reason to setting
 *   memsw.limit_in_bytes to twice of that is to give the system chance
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

#define MEMCG_PATH "/sys/fs/cgroup/memory/"

static char cgroup_path[PATH_MAX];
static char tasks_path[PATH_MAX];
static char limit_in_bytes_path[PATH_MAX];
static char memsw_limit_in_bytes_path[PATH_MAX];

static size_t page_size;
static int sleep_between_faults;

static int swap_accounting_enabled;

#define PAGES 128
#define TOUCHED_PAGE1 0
#define TOUCHED_PAGE2 10

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

static void setup_cgroup_paths(int pid)
{
	snprintf(cgroup_path, sizeof(cgroup_path),
		 MEMCG_PATH "ltp_madvise09_%i/", pid);
	snprintf(tasks_path, sizeof(tasks_path), "%s/tasks", cgroup_path);
	snprintf(limit_in_bytes_path, sizeof(limit_in_bytes_path),
		 "%s/memory.limit_in_bytes", cgroup_path);
	snprintf(memsw_limit_in_bytes_path, sizeof(memsw_limit_in_bytes_path),
		 "%s/memory.memsw.limit_in_bytes", cgroup_path);
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
	unsigned int usage, old_limit, old_memsw_limit;
	int status, pid, retries = 0;

	SAFE_MKDIR(cgroup_path, 0777);
	SAFE_FILE_PRINTF(tasks_path, "%i", getpid());

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
		tst_res(TFAIL, "MADV_FREE pages were freed immediatelly");
	else
		tst_res(TPASS, "MADV_FREE pages were not freed immediatelly");

	ptr[TOUCHED_PAGE1 * page_size] = 'b';
	ptr[TOUCHED_PAGE2 * page_size] = 'b';

	usage = 8 * 1024 * 1024;
	tst_res(TINFO, "Setting memory limits to %u %u", usage, 2 * usage);

	SAFE_FILE_SCANF(limit_in_bytes_path, "%u", &old_limit);

	if (swap_accounting_enabled)
		SAFE_FILE_SCANF(memsw_limit_in_bytes_path, "%u", &old_memsw_limit);

	SAFE_FILE_PRINTF(limit_in_bytes_path, "%u", usage);

	if (swap_accounting_enabled)
		SAFE_FILE_PRINTF(memsw_limit_in_bytes_path, "%u", 2 * usage);

	do {
		sleep_between_faults++;

		pid = SAFE_FORK();
		if (!pid)
			memory_pressure_child();

		tst_res(TINFO, "Memory hungry child %i started, try %i", pid, retries);

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

	tst_res(TINFO, "Memory map: %s", map);

	if (freed)
		tst_res(TPASS, "Pages MADV_FREE were freed on low memory");
	else
		tst_res(TFAIL, "No MADV_FREE page was freed on low memory");

	if (corrupted)
		tst_res(TFAIL, "Found corrupted page");
	else
		tst_res(TPASS, "All pages have expected content");

	if (swap_accounting_enabled)
		SAFE_FILE_PRINTF(memsw_limit_in_bytes_path, "%u", old_memsw_limit);

	SAFE_FILE_PRINTF(limit_in_bytes_path, "%u", old_limit);

	SAFE_MUNMAP(ptr, PAGES);

	exit(0);
}

static void cleanup(void)
{
	if (cgroup_path[0] && !access(cgroup_path, F_OK))
		rmdir(cgroup_path);
}

static void run(void)
{
	pid_t pid;
	int status;

retry:
	pid = SAFE_FORK();

	if (!pid) {
		setup_cgroup_paths(getpid());
		child();
	}

	setup_cgroup_paths(pid);
	SAFE_WAIT(&status);
	cleanup();

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
	long int swap_total;

	if (access(MEMCG_PATH, F_OK)) {
		tst_brk(TCONF, "'" MEMCG_PATH
			"' not present, CONFIG_MEMCG missing?");
	}

	if (!access(MEMCG_PATH "memory.memsw.limit_in_bytes", F_OK))
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
	.cleanup = cleanup,
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
};
