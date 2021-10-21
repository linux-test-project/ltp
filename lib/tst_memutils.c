// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
 */

#include <unistd.h>
#include <limits.h>
#include <sys/sysinfo.h>
#include <stdlib.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

#define BLOCKSIZE (16 * 1024 * 1024)

void tst_pollute_memory(size_t maxsize, int fillchar)
{
	size_t i, map_count = 0, safety = 0, blocksize = BLOCKSIZE;
	unsigned long min_free;
	void **map_blocks;
	struct sysinfo info;

	SAFE_FILE_SCANF("/proc/sys/vm/min_free_kbytes", "%lu", &min_free);
	min_free *= 1024;
	/* Apply a margin because we cannot get below "min" watermark */
	min_free += min_free / 10;

	SAFE_SYSINFO(&info);
	safety = MAX(4096 * SAFE_SYSCONF(_SC_PAGESIZE), 128 * 1024 * 1024);
	safety = MAX(safety, min_free);
	safety /= info.mem_unit;

	if (info.freeswap > safety)
		safety = 0;

	/* Not enough free memory to avoid invoking OOM killer */
	if (info.freeram <= safety)
		return;

	if (!maxsize)
		maxsize = SIZE_MAX;

	if (info.freeram - safety < maxsize / info.mem_unit)
		maxsize = (info.freeram - safety) * info.mem_unit;

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
