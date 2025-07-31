// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Verify that munmap() fails with ENOMEM after partially unmapping an
 * existing map, while having the maximum amount of maps already allocated.
 */

#include "tst_test.h"
#include "lapi/mmap.h"

#define PAD 2 /* avoid adjacent mapping merges */
#define MEMSIZE 3
#define MAP_MAX_COUNT 65530

static uintptr_t base = 0x100000000UL;
static size_t page_sz;
static unsigned long vma_size;
static int map_count;
static void **maps;

static void run(void)
{
	TST_EXP_FAIL(munmap(maps[2] + page_sz, page_sz), ENOMEM);
}

static void setup(void)
{
	uintptr_t addr = base;

	page_sz = SAFE_SYSCONF(_SC_PAGESIZE);
	vma_size = MEMSIZE * page_sz;

	maps = SAFE_MALLOC(MAP_MAX_COUNT * sizeof(char *));
	for (int i = 0; i < MAP_MAX_COUNT; i++)
		maps[i] = NULL;

	while (1) {
		void *p = mmap((void *) addr,
			     vma_size, PROT_NONE,
			     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
			     -1, 0);
		if (p == MAP_FAILED && errno == EEXIST)
			goto next_addr;
		if (p == MAP_FAILED)
			break;
		maps[map_count++] = p;
next_addr:
		addr += PAD * vma_size;
	}

	if (map_count == MAP_MAX_COUNT)
		tst_brk(TBROK, "Mapped all %d regions, expected less", map_count);
	if (map_count == 0)
		tst_brk(TBROK, "Mapped 0 regions");

	tst_res(TINFO, "Mapped %d regions", map_count);
}

static void cleanup(void)
{
	for (int i = 0; i < map_count; i++) {
		if (maps[i] == NULL)
			break;
		SAFE_MUNMAP((void *)(maps[i]), vma_size);
	}
	free(maps);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.min_kver = "4.17",
	.save_restore = (const struct tst_path_val[]){
		{ "/proc/sys/vm/max_map_count", TST_TO_STR(MAP_MAX_COUNT), TST_SR_SKIP },
		{},
	},
};
