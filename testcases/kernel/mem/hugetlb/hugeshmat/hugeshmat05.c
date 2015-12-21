/*
 * Copyright (c) 2015 Red Hat, Inc.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * DESCRIPTION
 *	shmget()/shmat() fails to allocate huge pages shared memory segment
 *	with EINVAL if its size is not in the range [ N*HUGE_PAGE_SIZE - 4095,
 *	N*HUGE_PAGE_SIZE ]. This is a problem in the memory segment size round
 *	up algorithm. The requested size is rounded up to PAGE_SIZE (4096), but
 *	if this roundup does not match HUGE_PAGE_SIZE (2Mb) boundary - the
 *	allocation fails.
 *
 *	This bug is present in all RHEL6 versions, but not in RHEL7. It looks
 *	like this was fixed in mainline kernel > v3.3 by the following patches:
 *
 *	091d0d5 (shm: fix null pointer deref when userspace specifies
 *		 invalid hugepage size)
 *	af73e4d (hugetlbfs: fix mmap failure in unaligned size request)
 *	42d7395 (mm: support more pagesizes for MAP_HUGETLB/SHM_HUGETLB)
 *	40716e2 (hugetlbfs: fix alignment of huge page requests)
 *
 * AUTHORS
 *	Vladislav Dronov <vdronov@redhat.com>
 *	Li Wang <liwang@redhat.com>
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "test.h"
#include "mem.h"
#include "hugetlb.h"

char *TCID = "hugeshmat05";
int TST_TOTAL = 1;

static long page_size;
static long hpage_size;
static long hugepages;

#define N 4

void setup(void)
{
	tst_require_root();
	check_hugepage();

	orig_hugepages = get_sys_tune("nr_hugepages");
	page_size = getpagesize();
	hpage_size = read_meminfo("Hugepagesize:") * 1024;

	hugepages = N + 1;
	set_sys_tune("nr_hugepages", hugepages, 1);

	TEST_PAUSE;
}

void cleanup(void)
{
	set_sys_tune("nr_hugepages", orig_hugepages, 0);
}

void shm_test(int size)
{
	int shmid;
	char *shmaddr;

	shmid = shmget(IPC_PRIVATE, size, 0600 | IPC_CREAT | SHM_HUGETLB);
	if (shmid < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "shmget failed");

	shmaddr = shmat(shmid, 0, 0);
	if (shmaddr == (char *)-1) {
		shmctl(shmid, IPC_RMID, NULL);
		tst_brkm(TFAIL | TERRNO, cleanup,
			 "Bug: shared memory attach failure.");
	}

	shmaddr[0] = 1;
	tst_resm(TINFO, "allocated %d huge bytes", size);

	if (shmdt((const void *)shmaddr) != 0) {
		shmctl(shmid, IPC_RMID, NULL);
		tst_brkm(TFAIL | TERRNO, cleanup, "Detach failure.");
	}

	shmctl(shmid, IPC_RMID, NULL);
}

int main(int ac, char **av)
{
	int lc;
	unsigned int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	const int tst_sizes[] = {
		N * hpage_size - page_size,
		N * hpage_size - page_size - 1,
		hpage_size,
		hpage_size + 1
	};

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < ARRAY_SIZE(tst_sizes); ++i)
			shm_test(tst_sizes[i]);

		tst_resm(TPASS, "No regression found.");
	}

	cleanup();
	tst_exit();
}
