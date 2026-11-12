// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Verify that munmap() fails with ENOMEM after partially unmapping an
 * existing map, while having the maximum amount of maps already allocated.
 */

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "lapi/mmap.h"

#define MEMSIZE 3
#define MAP_MAX_COUNT 65530

static uintptr_t base = 0x10000000UL;
static size_t page_sz;
static long max_map_count;
static int map_count;
static void **maps;
static void *victim;

static void run(void)
{
	TST_EXP_FAIL(munmap(victim + page_sz, page_sz), ENOMEM);
}

static int count_vmas(void)
{
	char buf[4096];
	int fd, lines = 0;
	ssize_t len;

	/* Avoid stdio, malloc() may need a new mapping we can't get */
	fd = SAFE_OPEN("/proc/self/maps", O_RDONLY);

	while ((len = SAFE_READ(0, fd, buf, sizeof(buf))) > 0) {
		for (ssize_t i = 0; i < len; i++)
			lines += buf[i] == '\n';
	}

	SAFE_CLOSE(fd);
	return lines;
}

/*
 * Gate VMAs ([vsyscall] on x86, [vectors] on 32-bit ARM) are listed in
 * /proc/self/maps but are not counted against vm.max_map_count.
 */
static int is_gate_vma(const char *line)
{
	char name[BUFSIZ];

	if (sscanf(line, "%*p-%*p %*4s %*p %*2d:%*2d %*d %s", name) != 1)
		return 0;

	switch (tst_arch.type) {
	case TST_X86:
	case TST_X86_64:
		return !strcmp(name, "[vsyscall]");
	case TST_ARM:
		return tst_kernel_bits() == 32 && !strcmp(name, "[vectors]");
	default:
		return 0;
	}
}

static int count_gate_vmas(void)
{
	FILE *fp = SAFE_FOPEN("/proc/self/maps", "r");
	char line[BUFSIZ];
	int gates = 0;

	while (fgets(line, sizeof(line), fp))
		gates += is_gate_vma(line);

	SAFE_FCLOSE(fp);
	return gates;
}

static void setup(void)
{
	uintptr_t addr = base;
	int vmas, gate_vmas, mmap_errno = 0;

	page_sz = SAFE_SYSCONF(_SC_PAGESIZE);

	/* The limit stays unchanged if the sysctl is read-only */
	SAFE_FILE_SCANF("/proc/sys/vm/max_map_count", "%ld", &max_map_count);
	if (max_map_count <= 0 || max_map_count > MAP_MAX_COUNT) {
		tst_brk(TCONF, "Read-only vm.max_map_count %ld not in 1..%d",
			max_map_count, MAP_MAX_COUNT);
	}
	gate_vmas = count_gate_vmas();
	vmas = count_vmas() - gate_vmas;
	if (vmas >= max_map_count) {
		tst_brk(TCONF, "max_map_count %ld <= %d VMAs already in use",
			max_map_count, vmas);
	}

	/* Map the region to be split while there is still room for it */
	victim = SAFE_MMAP(NULL, MEMSIZE * page_sz, PROT_NONE,
			   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	maps = SAFE_CALLOC(max_map_count, sizeof(*maps));

	/*
	 * Use single page, back to back mappings with alternating protection
	 * so that adjacent mappings are never merged. This needs only one page
	 * of address space per VMA, which also fits on 32-bit systems with
	 * small user address space (e.g. ARM CONFIG_VMSPLIT_1G).
	 */
	while (map_count < max_map_count) {
		int prot = (addr / page_sz) % 2 ? PROT_READ : PROT_NONE;
		void *p = mmap((void *) addr, page_sz, prot,
			     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
			     -1, 0);
		if (p == MAP_FAILED && errno == EEXIST)
			goto next_addr;
		if (p == MAP_FAILED) {
			mmap_errno = errno;
			break;
		}
		maps[map_count++] = p;
next_addr:
		addr += page_sz;
	}

	if (map_count == max_map_count)
		tst_brk(TBROK, "Mapped all %d regions, expected less", map_count);
	if (map_count == 0) {
		errno = mmap_errno;
		tst_brk(TBROK | TERRNO, "Mapped 0 regions");
	}

	tst_res(TINFO, "Mapped %d regions", map_count);

	/*
	 * munmap() refuses the split only when the process has at least
	 * max_map_count VMAs. Gate VMAs are not part of that count.
	 */
	vmas = count_vmas() - gate_vmas;
	if (vmas < max_map_count) {
		errno = mmap_errno;
		tst_brk(TBROK | TERRNO,
			"VMA limit not reached: mmap(%p) failed at %d/%ld VMAs",
			(void *)addr, vmas, max_map_count);
	}
}

static void cleanup(void)
{
	for (int i = 0; i < map_count; i++) {
		if (maps[i] == NULL)
			break;
		SAFE_MUNMAP((void *)(maps[i]), page_sz);
	}
	free(maps);

	if (victim)
		SAFE_MUNMAP(victim, MEMSIZE * page_sz);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.min_kver = "4.17",
	.ulimit = (const struct tst_ulimit_val[]) {
		{RLIMIT_AS, RLIM_INFINITY},
		{}
	},
	.save_restore = (const struct tst_path_val[]){
		{ "/proc/sys/vm/max_map_count", TST_TO_STR(MAP_MAX_COUNT), TST_SR_SKIP },
		{},
	},
};
