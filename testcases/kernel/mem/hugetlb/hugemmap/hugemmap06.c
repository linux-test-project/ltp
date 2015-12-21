/*
 *  Copyright (c) 2015 Red Hat, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "test.h"
#include "mem.h"
#include "hugetlb.h"
#include "lapi/mmap.h"

char *TCID = "hugemmap06";
int TST_TOTAL = 5;

static long hpage_size;
static long hugepages;

struct mp {
	char *addr;
	int sz;
};

#define ARSZ 50

void setup(void)
{
	tst_require_root();
	check_hugepage();

	/* MAP_HUGETLB check */
	if ((tst_kvercmp(2, 6, 32)) < 0) {
		tst_brkm(TCONF, NULL, "This test can only run on kernels "
			"that are 2.6.32 or higher");
	}

	hpage_size = read_meminfo("Hugepagesize:") * 1024;
	orig_hugepages = get_sys_tune("nr_hugepages");

	hugepages = (ARSZ + 1) * TST_TOTAL;

	if (hugepages * read_meminfo("Hugepagesize:") > read_meminfo("MemTotal:"))
		tst_brkm(TCONF, NULL, "System RAM is not enough to test.");

	set_sys_tune("nr_hugepages", hugepages, 1);

	TEST_PAUSE;
}

void cleanup(void)
{
	set_sys_tune("nr_hugepages", orig_hugepages, 0);
}

void *thr(void *arg)
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

void do_mmap(void)
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
			tst_brkm(TCONF, cleanup,
				"Cannot allocate hugepage, memory too fragmented?");
		}

		tst_brkm(TBROK | TERRNO, cleanup, "Cannot allocate hugepage");
	}

	for (i = 0; i < ARSZ; ++i, --sz) {
		mmap_sz[i].sz = sz;
		mmap_sz[i].addr = addr;

		TEST(pthread_create(&tid[i], NULL, thr, &mmap_sz[i]));
		if (TEST_RETURN)
			tst_brkm(TBROK | TRERRNO, cleanup,
					"pthread_create failed");

		new_addr = mmap(addr, (sz - 1) * hpage_size,
				PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_FIXED,
				-1, 0);

		if (new_addr == MAP_FAILED)
			tst_brkm(TFAIL | TERRNO, cleanup, "mmap failed");

		addr = new_addr;
	}

	for (i = 0; i < ARSZ; ++i) {
		TEST(pthread_join(tid[i], NULL));
		if (TEST_RETURN)
			tst_brkm(TBROK | TRERRNO, cleanup,
					"pthread_join failed");
	}

	if (munmap(addr, sz * hpage_size) == -1)
		tst_brkm(TFAIL | TERRNO, cleanup, "huge munmap failed");
}

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			do_mmap();

		tst_resm(TPASS, "No regression found.");
	}

	cleanup();
	tst_exit();
}
