// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (C) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * Verify that shmctl() IPC_STAT and SHM_STAT reports correct data.
 *
 * The shm_nattach is excercised by:
 *
 * - forking() children that attach and detach SHM
 * - attaching the SHM before fork and letting the children detach it
 *
 * We check that the number shm_nattach is correct after each step we do.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tst_clocks.h"
#include "libnewipc.h"

#define NCHILD 20

static pid_t children[NCHILD];

static int shm_id;
static int shm_idx;
static time_t ctime_min, ctime_max;

static void *addr;

static void attach_child(void)
{
	pause();

	addr = SAFE_SHMAT(shm_id, NULL, 0);

	pause();

	SAFE_SHMDT(addr);

	pause();

	exit(0);
}

static void detach_child(void)
{
	pause();

	SAFE_SHMDT(addr);

	pause();

	exit(0);
}

static void fork_children(void (*child_func)(void))
{
	unsigned int i;

	for (i = 0; i < NCHILD; i++) {
		pid_t pid = SAFE_FORK();

		if (!pid)
			child_func();

		children[i] = pid;
	}
}

static void wait_for_children(void)
{
	unsigned int i;

	for (i = 0; i < NCHILD; i++)
		TST_PROCESS_STATE_WAIT(children[i], 'S', 0);
}

static void signal_children(void)
{
	unsigned int i;

	for (i = 0; i < NCHILD; i++)
		SAFE_KILL(children[i], SIGUSR1);
}

static void reap_children(void)
{
	unsigned int i;

	for (i = 0; i < NCHILD; i++)
		SAFE_WAITPID(children[i], NULL, 0);
}

static void check_nattch(int exp_nattch, const char *msg)
{
	struct shmid_ds ds1;
	struct shmid_ds ds2;

	SAFE_SHMCTL(shm_id, IPC_STAT, &ds1);
	SAFE_SHMCTL(shm_idx, SHM_STAT, &ds2);

	if (ds1.shm_nattch != ds2.shm_nattch) {
		tst_res(TFAIL, "IPC_STAT nattch=%li SHM_STAT nattch=%li",
			(long)ds1.shm_nattch, (long)ds2.shm_nattch);
		return;
	}

	if ((int)ds1.shm_nattch == exp_nattch) {
		tst_res(TPASS, "%s shm_nattch=%i", msg, exp_nattch);
		return;
	}

	tst_res(TFAIL, "%s shm_nattcg=%li expected %i",
	        msg, (long)ds1.shm_nattch, exp_nattch);
}

static void verify_shmstat_attach(void)
{
	fork_children(attach_child);
	wait_for_children();

	check_nattch(0, "before child shmat()");

	signal_children();
	wait_for_children();

	check_nattch(NCHILD, "after child shmat()");

	signal_children();
	wait_for_children();

	check_nattch(0, "after child shmdt()");

	signal_children();
	reap_children();
}

static void verify_shmstat_inherit(void)
{
	addr = SAFE_SHMAT(shm_id, NULL, 0);

	fork_children(detach_child);
	wait_for_children();

	check_nattch(NCHILD+1, "inherited after fork()");

	signal_children();
	wait_for_children();

	check_nattch(1, "after child shmdt()");

	SAFE_SHMDT(addr);

	check_nattch(0, "after parent shmdt()");

	signal_children();
	reap_children();
}

static void check_ds(struct shmid_ds *ds, const char *desc)
{
	pid_t pid = getpid();

	if (ds->shm_segsz != SHM_SIZE) {
		tst_res(TFAIL, "%s: shm_segsz=%zu, expected %i",
		        desc, ds->shm_segsz, SHM_SIZE);
	} else {
		tst_res(TPASS, "%s: shm_segsz=%i", desc, SHM_SIZE);
	}

	if (ds->shm_cpid != pid) {
		tst_res(TFAIL, "%s: shm_cpid=%i, expected %i",
		        desc, ds->shm_cpid, pid);
	} else {
		tst_res(TPASS, "%s: shm_cpid=%i", desc, pid);
	}

	if (ds->shm_ctime < ctime_min || ds->shm_ctime > ctime_max) {
		tst_res(TFAIL, "%s: shm_ctime=%li, expected <%li,%li>",
			desc, ds->shm_ctime, ctime_min, ctime_max);
	} else {
		tst_res(TPASS, "%s: shm_ctime=%li in range <%li,%li>",
			desc, ds->shm_ctime, ctime_min, ctime_max);
	}
}

static void shmstat_basic_check(void)
{
	struct shmid_ds ds;

	memset(&ds, 0, sizeof(ds));
	SAFE_SHMCTL(shm_id, IPC_STAT, &ds);

	check_ds(&ds, "IPC_STAT");

	memset(&ds, 0, sizeof(ds));
	SAFE_SHMCTL(shm_idx, SHM_STAT, &ds);

	check_ds(&ds, "SHM_STAT");
}

static struct tcase {
	void (*func)(void);
	const char *desc;
} tcases[] = {
	{shmstat_basic_check, "Basic checks"},
	{verify_shmstat_attach, "Children attach SHM"},
	{verify_shmstat_inherit, "Chidlren inherit SHM"},
};

static void verify_shmstat(unsigned int n)
{
	tst_res(TINFO, "%s", tcases[n].desc);
	tcases[n].func();
}

static void dummy_sighandler(int sig)
{
	(void)sig;
}

static int get_shm_idx_from_id(int shm_id)
{
	struct shm_info dummy;
	struct shmid_ds dummy_ds;
	int max_idx, i;

	max_idx = SAFE_SHMCTL(shm_id, SHM_INFO, (void*)&dummy);

	for (i = 0; i <= max_idx; i++) {
		if (shmctl(i, SHM_STAT, &dummy_ds) == shm_id)
			return i;
	}

	return -1;
}

static void setup(void)
{
	ctime_min = tst_get_fs_timestamp();
	shm_id = SAFE_SHMGET(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | SHM_RW);
	ctime_max = tst_get_fs_timestamp();

	shm_idx = get_shm_idx_from_id(shm_id);

	if (shm_idx < 0)
		tst_brk(TBROK, "Failed to get shm_id to idx mapping");

	tst_res(TINFO, "shm_id=%i maps to kernel index=%i", shm_id, shm_idx);

	SAFE_SIGNAL(SIGUSR1, dummy_sighandler);
}

static void cleanup(void)
{
	if (shm_id >= 0)
		SAFE_SHMCTL(shm_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.test = verify_shmstat,
	.tcnt = ARRAY_SIZE(tcases),
};
