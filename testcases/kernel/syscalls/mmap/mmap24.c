// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that :manpage:`mmap(2)` with the MAP_32BIT flag restricts all mappings
 * to the first 2GB of the process address space (< 0x80000000).
 *
 * MAP_32BIT is supported only on x86-64 for 64-bit programs.
 *
 * Use PROT_NONE to test address placement without allocating data pages
 * or charging private writable memory against the commit limit.
 *
 * [Algorithm]
 *
 * - Repeatedly reserve 32MB chunks with MAP_32BIT and PROT_NONE until ENOMEM
 * - Verify that every returned address satisfies (addr + size) <= 0x80000000
 * - Verify that at least one chunk was mapped and failure errno is ENOMEM
 * - Before releasing any chunks, try the same mapping without MAP_32BIT
 * - Report TCONF if the unrestricted mapping also fails with ENOMEM,
 *   otherwise verify that it succeeds
 * - Unmap all allocated chunks in cleanup
 */

#include "tst_test.h"
#include "lapi/mmap.h"

#define ADDR_LIMIT 0x80000000UL
#define CHUNK_SZ (32UL * TST_MB)
#define MAX_CHUNKS 64

static void *addrs[MAX_CHUNKS];
static size_t num_chunks;

static void cleanup(void)
{
	size_t i;

	for (i = 0; i < num_chunks; i++) {
		if (addrs[i]) {
			SAFE_MUNMAP(addrs[i], CHUNK_SZ);
			addrs[i] = NULL;
		}
	}
	num_chunks = 0;
}

static void run(void)
{
	size_t i;
	void *addr;

	for (i = 0; i < MAX_CHUNKS; i++) {
		TESTPTR(mmap(NULL, CHUNK_SZ, PROT_NONE,
			     MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0));
		addr = TST_RET_PTR;

		if (addr == MAP_FAILED) {
			if (TST_ERR != ENOMEM) {
				tst_res(TFAIL | TTERRNO,
					"mmap() failed with unexpected errno");
				goto out;
			}
			break;
		}

		addrs[num_chunks++] = addr;

		if ((unsigned long)addr + CHUNK_SZ > ADDR_LIMIT) {
			tst_res(TFAIL, "mapping at %p + %lu exceeds 2GB limit",
				addr, CHUNK_SZ);
			goto out;
		}
	}

	if (i == MAX_CHUNKS) {
		tst_res(TFAIL, "MAP_32BIT did not fail with ENOMEM");
		goto out;
	}

	/* Keep the chunks mapped so the control sees the same resource usage. */
	TESTPTR(mmap(NULL, CHUNK_SZ, PROT_NONE,
		     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
	if (TST_RET_PTR == MAP_FAILED) {
		if (TST_ERR == ENOMEM)
			tst_brk(TCONF | TTERRNO,
				"Unrestricted mmap() also failed; result inconclusive");
		tst_brk(TBROK | TTERRNO,
			"Unrestricted mmap() failed unexpectedly");
	}
	SAFE_MUNMAP(TST_RET_PTR, CHUNK_SZ);

	if (!num_chunks)
		tst_res(TFAIL, "failed to map any chunk with MAP_32BIT");
	else
		tst_res(TPASS,
			"Mapped %lu MB across %zu chunks within 2GB before ENOMEM",
			(num_chunks * CHUNK_SZ) / TST_MB, num_chunks);

out:
	cleanup();
}

static struct tst_test test = {
	.cleanup = cleanup,
	.test_all = run,
	.supported_archs = (const char *const []){
		"x86_64",
		NULL
	},
};
