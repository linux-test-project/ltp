// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015-2017 Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2018-2023
 *
 * Authors:
 * Herton R. Krzesinski <herton@redhat.com>
 * Li Wang <liwang@redhat.com>
 */

/*\
 * [Description]
 *
 * There is a race condition if we map a same file on different processes.
 * Region tracking is protected by mmap_sem and hugetlb_instantiation_mutex.
 * When we do mmap, we don't grab a hugetlb_instantiation_mutex, but only
 * mmap_sem (exclusively).  This doesn't prevent other tasks from modifying
 * the region structure, so it can be modified by two processes concurrently.
 *
 * This bug was fixed on stable kernel by commits:
 *
 * f522c3ac00a4 (mm, hugetlb: change variable name reservations to resv)
 * 9119a41e9091 (mm, hugetlb: unify region structure handling)
 * 7b24d8616be3 (mm, hugetlb: fix race in region tracking)
 * 1406ec9ba6c6 (mm, hugetlb: improve, cleanup resv_map parameters)
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include "hugetlb.h"
#include "lapi/mmap.h"

static long hpage_size;

struct mp {
	char *addr;
	int sz;
};

#define ARSZ 50
#define LOOP 5

static void setup(void)
{
	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;
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
			tst_brk(TCONF, "Cannot allocate hugepage, memory too fragmented?");
		}

		tst_brk(TBROK | TERRNO, "Cannot allocate hugepage");
	}

	for (i = 0; i < ARSZ; ++i, --sz) {
		mmap_sz[i].sz = sz;
		mmap_sz[i].addr = addr;

		TEST(pthread_create(&tid[i], NULL, thr, &mmap_sz[i]));
		if (TST_RET)
			tst_brk(TBROK | TRERRNO, "pthread_create failed");

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
		if (TST_RET)
			tst_brk(TBROK | TRERRNO, "pthread_join failed");
	}

	if (munmap(addr, sz * hpage_size) == -1)
		tst_brk(TFAIL | TERRNO, "huge munmap failed");

	tst_res(TPASS, "No regression found");
}

static struct tst_test test = {
	.needs_root = 1,
	.tcnt = LOOP,
	.needs_tmpdir = 1,
	.test = do_mmap,
	.setup = setup,
	.hugepages = {(ARSZ + 1) * LOOP, TST_NEEDS},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "f522c3ac00a4"},
		{"linux-git", "9119a41e9091"},
		{"linux-git", "7b24d8616be3"},
		{"linux-git", "1406ec9ba6c6"},
		{}
	}
};
