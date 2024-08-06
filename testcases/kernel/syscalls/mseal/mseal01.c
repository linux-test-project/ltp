// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This is a smoke test that verifies if mseal() protects specific VMA portions
 * of a process. According to documentation, the syscall should protect memory
 * from the following actions:
 *
 * - unmapping, moving to another location, and shrinking the size, via munmap()
 *   and mremap()
 * - moving or expanding a different VMA into the current location, via mremap()
 * - modifying a VMA via mmap(MAP_FIXED)
 * - mprotect() and pkey_mprotect()
 * - destructive madvice() behaviors (e.g. MADV_DONTNEED) for anonymous memory,
 *   when users don’t have write permission to the memory
 *
 * Any of the described actions is recognized via EPERM errno.
 *
 * TODO: support both raw syscall and libc wrapper via .test_variants.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/pkey.h"

#define MEMPAGES 8
#define MEMSEAL 2

static void *mem_addr;
static int mem_size;
static int mem_offset;
static int mem_alignment;

static inline int sys_mseal(void *start, size_t len)
{
	return tst_syscall(__NR_mseal, start, len, 0);
}

static void test_mprotect(void)
{
	TST_EXP_FAIL(mprotect(mem_addr, mem_size, PROT_NONE), EPERM);
}

static void test_pkey_mprotect(void)
{
	int pkey;

	check_pkey_support();

	pkey = pkey_alloc(0, 0);
	if (pkey == -1)
		tst_brk(TBROK | TERRNO, "pkey_alloc failed");

	TST_EXP_FAIL(pkey_mprotect(
		mem_addr, mem_size,
		PROT_NONE,
		pkey),
		EPERM);

	if (pkey_free(pkey) == -1)
		tst_brk(TBROK | TERRNO, "pkey_free() error");
}

static void test_madvise(void)
{
	TST_EXP_FAIL(madvise(mem_addr, mem_size, MADV_DONTNEED), EPERM);
}

static void test_munmap(void)
{
	TST_EXP_FAIL(munmap(mem_addr, mem_size), EPERM);
}

static void test_mremap_resize(void)
{
	void *new_addr;
	size_t new_size = 2 * mem_alignment;

	new_addr = SAFE_MMAP(NULL, mem_size,
		PROT_READ,
		MAP_ANONYMOUS | MAP_PRIVATE,
		-1, 0);

	TST_EXP_FAIL_PTR_VOID(mremap(mem_addr, mem_size, new_size,
		MREMAP_MAYMOVE | MREMAP_FIXED,
		new_addr),
		EPERM);

	SAFE_MUNMAP(new_addr, new_size);
}

static void test_mmap_change_prot(void)
{
	TST_EXP_FAIL_PTR_VOID(mmap(mem_addr, mem_size,
		PROT_READ,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
		-1, 0), EPERM);
}

static struct tcase {
	void (*func_test)(void);
	int prot;
	char *message;
} tcases[] = {
	{test_mprotect, PROT_READ | PROT_WRITE, "mprotect() availability"},
	{test_pkey_mprotect, PROT_READ | PROT_WRITE, "pkey_mprotect() availability"},
	{test_madvise, PROT_READ, "madvise() availability"},
	{test_munmap, PROT_READ | PROT_WRITE, "munmap() availability from child"},
	{test_mremap_resize, PROT_READ | PROT_WRITE, "mremap() address move/resize"},
	{test_mmap_change_prot, PROT_READ | PROT_WRITE, "mmap() protection change"},
};

static void run(unsigned int n)
{
	/* the reason why we spawn a child is that mseal() will
	 * protect VMA until process will call _exit()
	 */
	if (!SAFE_FORK()) {
		struct tcase *tc = &tcases[n];

		mem_addr = SAFE_MMAP(NULL, mem_size,
			tc->prot,
			MAP_ANONYMOUS | MAP_PRIVATE,
			-1, 0);

		tst_res(TINFO, "Testing %s", tc->message);

		TST_EXP_PASS(sys_mseal(mem_addr + mem_offset, mem_alignment));

		tc->func_test();
		_exit(0);
	}
}

static void setup(void)
{
	mem_alignment = getpagesize();
	mem_size = mem_alignment * MEMPAGES;
	mem_offset = mem_alignment * MEMSEAL;
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.forks_child = 1,
};

