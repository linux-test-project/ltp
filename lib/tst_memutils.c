// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
 * Copyright (c) Linux Test Project, 2021-2023
 */

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/sysinfo.h>
#include <stdlib.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_memutils.h"
#include "tst_capability.h"
#include "lapi/syscalls.h"

#define BLOCKSIZE (16 * 1024 * 1024)

void tst_pollute_memory(size_t maxsize, int fillchar)
{
	size_t i, map_count = 0, safety = 0, blocksize = BLOCKSIZE;
	unsigned long long freeram;
	size_t min_free;
	void **map_blocks;
	struct sysinfo info;

	SAFE_FILE_SCANF("/proc/sys/vm/min_free_kbytes", "%zi", &min_free);
	min_free *= 1024;
	/* Apply a margin because we cannot get below "min" watermark */
	min_free += min_free / 10;

	SAFE_SYSINFO(&info);
	safety = MAX(4096 * SAFE_SYSCONF(_SC_PAGESIZE), 128L * 1024 * 1024);
	safety = MAX(safety, min_free);
	safety /= info.mem_unit;

	if (info.freeswap > safety)
		safety = 0;

	/*
	 * MemFree usually is lower than MemAvailable, although when setting
	 * sysctl vm.lowmem_reserve_ratio, this could reverse.
	 *
	 * Use the lower value of both for pollutable memory. Usually this
	 * means we will not evict any caches.
	 */
	freeram = MIN((long long)info.freeram, (tst_available_mem() * 1024));

	/* Not enough free memory to avoid invoking OOM killer */
	if (freeram <= safety)
		return;

	if (!maxsize)
		maxsize = SIZE_MAX;

	if (freeram - safety < maxsize / info.mem_unit)
		maxsize = (freeram - safety) * info.mem_unit;

	blocksize = MIN(maxsize, blocksize);
	map_count = maxsize / blocksize;
	map_blocks = SAFE_MALLOC(map_count * sizeof(void *));

	/*
	 * Keep allocating until the first failure. The address space may be
	 * too fragmented or just smaller than maxsize.
	 */
	for (i = 0; i < map_count; i++) {
		map_blocks[i] = mmap(NULL, blocksize, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		if (map_blocks[i] == MAP_FAILED) {
			map_count = i;
			break;
		}

		memset(map_blocks[i], fillchar, blocksize);
	}

	for (i = 0; i < map_count; i++)
		SAFE_MUNMAP(map_blocks[i], blocksize);

	free(map_blocks);
}

long long tst_available_mem(void)
{
	unsigned long long mem_available = 0;

	if (FILE_LINES_SCANF("/proc/meminfo", "MemAvailable: %llu",
		&mem_available)) {
		mem_available = SAFE_READ_MEMINFO("MemFree:")
			+ SAFE_READ_MEMINFO("Cached:");
	}

	return mem_available;
}

long long tst_available_swap(void)
{
	unsigned long long swap_available = 0;

	FILE_LINES_SCANF("/proc/meminfo", "SwapFree: %llu", &swap_available);

	return swap_available;
}

static int has_caps(void)
{
	struct tst_cap_user_header hdr = {
		.version = 0x20080522,
		.pid = tst_syscall(__NR_gettid),
	};

	struct tst_cap_user_data caps[2];

	if (tst_capget(&hdr, caps))
		tst_brk(TBROK | TERRNO, "tst_capget()");

	if (caps[0].effective & (1U << CAP_SYS_RESOURCE))
		return 1;

	return 0;
}

static int write_score(const char *path, int score)
{
	FILE *f;

	f = fopen(path, "w");
	if (!f)
		return 1;

	if (fprintf(f, "%d", score) <= 0) {
		fclose(f);
		return 1;
	}

	if (fclose(f))
		return 1;

	return 0;
}

static void set_oom_score_adj(pid_t pid, int value)
{
	int val;
	char score_path[64];

	if (access("/proc/self/oom_score_adj", F_OK) == -1) {
		tst_res(TINFO, "oom_score_adj does not exist, skipping the adjustment");
		return;
	}

	if (pid == 0) {
		sprintf(score_path, "/proc/self/oom_score_adj");
	} else {
		sprintf(score_path, "/proc/%d/oom_score_adj", pid);
		if (access(score_path, F_OK) == -1)
			tst_brk(TBROK, "%s does not exist, please check if PID is valid", score_path);
	}

	if (write_score(score_path, value)) {
		if (!has_caps())
			return;

		tst_res(TWARN, "Can't adjust score, even with capabilities!?");
		return;
	}

	FILE_SCANF(score_path, "%d", &val);

	if (val != value)
		tst_brk(TBROK, "oom_score_adj = %d, but expect %d.", val, value);
}

void tst_enable_oom_protection(pid_t pid)
{
	set_oom_score_adj(pid, -1000);
}

void tst_disable_oom_protection(pid_t pid)
{
	set_oom_score_adj(pid, 0);
}
