// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *	12/20/2002	Port to LTP	robbiew@us.ibm.com
 *	06/30/2001	Port to Linux	nsharoff@us.ibm.com
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Create a shared memory segment and fork a child. Both
 * parent and child spin in a loop attaching and detaching
 * the segment. After completing the specified number of
 * iterations, the child exits and the parent deletes the
 * segment.
 */

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"

#define SHMSIZE 0x32768

static char *str_iter_n;
static long long iter_n = 500;
static int shmid;

static void do_shm_cycle(long long iter_n)
{
	char *addr;

	for (int i = 0; i < iter_n; i++) {
		addr = SAFE_SHMAT(shmid, NULL, 0);
		addr[0] = 'a';
		SAFE_SHMDT(addr);
	}
}

static void run(void)
{
	key_t key;

	key = GETIPCKEY();

	shmid = SAFE_SHMGET(key, SHMSIZE, IPC_CREAT | 0666);
	if (!SAFE_FORK()) {
		do_shm_cycle(iter_n);
		_exit(0);
	}

	do_shm_cycle(iter_n);
	tst_reap_children();
	SAFE_SHMCTL(shmid, IPC_RMID, NULL);

	tst_res(TPASS, "Attached and detached %lld times", iter_n * 2);
}

static void setup(void)
{
	if (tst_parse_filesize(str_iter_n, &iter_n, 1, LLONG_MAX))
		tst_brk(TBROK, "Invalid amount of iterations: %s", str_iter_n);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.forks_child = 1,
	.options = (struct tst_option[]) {
		{"n:", &str_iter_n, "Amount of iterations (default 500)"},
		{}
	},
};
