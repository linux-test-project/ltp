// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Google, Inc.
 */

/*
 * Regression test for commit 3f05317d9889 ("ipc/shm: fix use-after-free of shm
 * file via remap_file_pages()").  This bug allowed the remap_file_pages()
 * syscall to use the file of a System V shared memory segment after its ID had
 * been reallocated and the file freed.  This test reproduces the bug as a NULL
 * pointer dereference in touch_atime(), although it's a race condition so it's
 * not guaranteed to work.  This test is based on the reproducer provided in the
 * fix's commit message.
 */

#include "lapi/syscalls.h"
#include "tst_test.h"
#include "tst_fuzzy_sync.h"
#include "tst_safe_pthread.h"
#include "tst_safe_sysv_ipc.h"
#include "tst_timer.h"

static struct tst_fzsync_pair fzsync_pair;

/*
 * Thread 2: repeatedly remove the shm ID and reallocate it again for a
 * new shm segment.
 */
static void *thrproc(void *unused)
{
	int id = SAFE_SHMGET(0xF00F, 4096, IPC_CREAT|0700);

	while (tst_fzsync_run_b(&fzsync_pair)) {
		tst_fzsync_start_race_b(&fzsync_pair);
		SAFE_SHMCTL(id, IPC_RMID, NULL);
		id = SAFE_SHMGET(0xF00F, 4096, IPC_CREAT|0700);
		tst_fzsync_end_race_b(&fzsync_pair);
	}
	return unused;
}

static void setup(void)
{
	/* Skip test if either remap_file_pages() or SysV IPC is unavailable */
	tst_syscall(__NR_remap_file_pages, NULL, 0, 0, 0, 0);
	tst_syscall(__NR_shmctl, 0xF00F, IPC_RMID, NULL);

	tst_fzsync_pair_init(&fzsync_pair);
}

static void do_test(void)
{
	/*
	 * Thread 1: repeatedly attach a shm segment, then remap it until the ID
	 * seems to have been removed by the other process.
	 */
	tst_fzsync_pair_reset(&fzsync_pair, thrproc);
	while (tst_fzsync_run_a(&fzsync_pair)) {
		int id;
		void *addr;

		id = SAFE_SHMGET(0xF00F, 4096, IPC_CREAT|0700);
		addr = SAFE_SHMAT(id, NULL, 0);
		tst_fzsync_start_race_a(&fzsync_pair);
		do {
			/* This is the system call that crashed */
			TEST(syscall(__NR_remap_file_pages, addr, 4096,
				     0, 0, 0));
		} while (TST_RET == 0);
		tst_fzsync_end_race_a(&fzsync_pair);

		if (TST_ERR != EIDRM && TST_ERR != EINVAL) {
			tst_brk(TBROK | TTERRNO,
				"Unexpected remap_file_pages() error");
		}

		/*
		 * Ensure that a shm segment will actually be destroyed.
		 * This call may fail on recent kernels (v4.0+) because
		 * remap_file_pages() already unmapped the shm segment.
		 */
		shmdt(addr);
	}

	tst_res(TPASS, "didn't crash");
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&fzsync_pair);
	shmctl(0xF00F, IPC_RMID, NULL);
}

static struct tst_test test = {
	.timeout = 20,
	.setup = setup,
	.test_all = do_test,
	.cleanup = cleanup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "3f05317d9889"},
		{}
	}
};
