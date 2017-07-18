/*
 * Copyright (c) 2015-2017 Red Hat, Inc.
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
 * DESCRIPTION
 *
 *   There is a race condition if we map a same file on different processes.
 *   Region tracking is protected by mmap_sem and hugetlb_instantiation_mutex.
 *   When we do mmap, we don't grab a hugetlb_instantiation_mutex, but only
 *   mmap_sem (exclusively).  This doesn't prevent other tasks from modifying
 *   the region structure, so it can be modified by two processes concurrently.
 *
 *   This bug was fixed on stable kernel by commits:
 *       f522c3ac00(mm, hugetlb: change variable name reservations to resv)
 *       9119a41e90(mm, hugetlb: unify region structure handling)
 *       7b24d8616b(mm, hugetlb: fix race in region tracking)
 *       1406ec9ba6(mm, hugetlb: improve, cleanup resv_map parameters)
 *
 * AUTHOR:
 *    Herton R. Krzesinski <herton@redhat.com>
 *    Li Wang <liwang@redhat.com>
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include "mem.h"
#include "hugetlb.h"
#include "lapi/mmap.h"

static long hpage_size;
static long hugepages;

struct mp {
	char *addr;
	int sz;
};

#define ARSZ 50
#define LOOP 5

static void setup(void)
{
	check_hugepage();

	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;
	orig_hugepages = get_sys_tune("nr_hugepages");

	hugepages = (ARSZ + 1) * LOOP;

	if (hugepages * SAFE_READ_MEMINFO("Hugepagesize:") > SAFE_READ_MEMINFO("MemTotal:"))
		tst_brk(TCONF, "System RAM is not enough to test.");

	set_sys_tune("nr_hugepages", hugepages, 1);
}

static void cleanup(void)
{
	set_sys_tune("nr_hugepages", orig_hugepages, 0);
}

static void *thr(void *arg)
{
	struct mp *mmap_sz = arg;
	int i, lim, a, b, c;

	srand(time(NULL));
	lim = rand() % 10;
	for (i = 0; i < lim; i++) {
		a = rand() % mmap_sz->sz;
		for (c = 0; c <= a; c++) {
			b = rand() % mmap_sz->sz;
			*(mmap_sz->addr + b * hpage_size) = rand();
		}
	}
	return NULL;
}

static void do_mmap(unsigned int j LTP_ATTRIBUTE_UNUSED)
{
	int i, sz = ARSZ + 1;
	void *addr, *new_addr;
	struct mp mmap_sz[ARSZ];
	pthread_t tid[ARSZ];

	addr = mmap(NULL, sz * hpage_size,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
			-1, 0);

	if (addr == MAP_FAILED) {
		if (errno == ENOMEM) {
			tst_brk(TCONF,
				"Cannot allocate hugepage, memory too fragmented?");
		}

		tst_brk(TBROK | TERRNO, "Cannot allocate hugepage");
	}

	for (i = 0; i < ARSZ; ++i, --sz) {
		mmap_sz[i].sz = sz;
		mmap_sz[i].addr = addr;

		TEST(pthread_create(&tid[i], NULL, thr, &mmap_sz[i]));
		if (TEST_RETURN)
			tst_brk(TBROK | TRERRNO,
					"pthread_create failed");

		new_addr = mmap(addr, (sz - 1) * hpage_size,
				PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_FIXED,
				-1, 0);

		if (new_addr == MAP_FAILED)
			tst_brk(TFAIL | TERRNO, "mmap failed");

		addr = new_addr;
	}

	for (i = 0; i < ARSZ; ++i) {
		TEST(pthread_join(tid[i], NULL));
		if (TEST_RETURN)
			tst_brk(TBROK | TRERRNO,
					"pthread_join failed");
	}

	if (munmap(addr, sz * hpage_size) == -1)
		tst_brk(TFAIL | TERRNO, "huge munmap failed");

	tst_res(TPASS, "No regression found.");
}

static struct tst_test test = {
	.min_kver = "2.6.32",
	.needs_root = 1,
	.tcnt = LOOP,
	.needs_tmpdir = 1,
	.test = do_mmap,
	.setup = setup,
	.cleanup = cleanup,
};
