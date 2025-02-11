// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC
 * Author: Michal Hocko <mhocko@suse.com>
 * LTP port: Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * When debugging issues with a workload using SysV shmem, Michal Hocko has
 * come up with a reproducer that shows how a series of mprotect()
 * operations can result in an elevated shm_nattch and thus leak of the
 * resource.
 *
 * The problem is caused by wrong assumptions in vma_merge() commit
 * 714965ca8252 ("mm/mmap: start distinguishing if vma can be removed in
 * mergeability test"). The shmem vmas have a vma_ops->close callback
 * that decrements shm_nattch, and we remove the vma without calling it.
 *
 * Patch: https://lore.kernel.org/all/20240222215930.14637-2-vbabka@suse.cz/
 */

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static int segment_id = -1;
static int key_id;
static int page_size;
static size_t segment_size;

static void run(void)
{
	struct shmid_ds shmid_ds;
	void *sh_mem;

	segment_id = SAFE_SHMGET(
		key_id,
		segment_size,
		IPC_CREAT | IPC_EXCL | 0600);

	sh_mem = SAFE_SHMAT(segment_id, NULL, 0);

	tst_res(TINFO, "Attached at %p. key: %d - size: %lu",
		sh_mem, segment_id, segment_size);

	SAFE_SHMCTL(segment_id, IPC_STAT, &shmid_ds);

	tst_res(TINFO, "Number of attaches: %lu", shmid_ds.shm_nattch);

	SAFE_MPROTECT(sh_mem + page_size, page_size, PROT_NONE);
	SAFE_MPROTECT(sh_mem, 2 * page_size, PROT_WRITE);

	SAFE_SHMCTL(segment_id, IPC_STAT, &shmid_ds);

	tst_res(TINFO, "Number of attaches: %lu", shmid_ds.shm_nattch);
	tst_res(TINFO, "Delete attached memory");

	SAFE_SHMDT(sh_mem);
	SAFE_SHMCTL(segment_id, IPC_STAT, &shmid_ds);

	tst_res(TINFO, "Number of attaches: %lu", shmid_ds.shm_nattch);

	SAFE_SHMCTL(segment_id, IPC_RMID, NULL);
	segment_id = -1;

	TST_EXP_EQ_LU(shmid_ds.shm_nattch, 0);
}

static void setup(void)
{
	key_id = GETIPCKEY() + 1;
	page_size = getpagesize();

	tst_res(TINFO, "Key id: %d", key_id);
	tst_res(TINFO, "Page size: %d", page_size);

	segment_size = 3 * page_size;
}

static void cleanup(void)
{
	if (segment_id != -1)
		SAFE_SHMCTL(segment_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "fc0c8f9089c2"},
		{}
	}
};
