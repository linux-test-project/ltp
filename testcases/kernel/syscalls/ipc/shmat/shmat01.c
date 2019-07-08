// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*
 * DESCRIPTION
 *
 * 1) shmat() chooses a suitable (unused) address when shmaddr is NULL.
 * 2) shmat() attaches shm segment to the shmaddr when shmaddr is a
 *    page-aligned address.
 * 3) shmat() attaches shm segment to the address equal to shmaddr rounded
 *    down to the nearest multiple of SHMLBA when shmaddr is a page-unaligned
 *    address and shmflg is set to SHM_RND.
 * 4) shmat() attaches shm segment to the shmaddr for reading when shmflg
 *    is set to SHM_RDONLY.
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

#define ALIGN_DOWN(in_addr) ((void *)(((uintptr_t)in_addr / SHMLBA) * SHMLBA))

static int shm_id = -1;
static key_t shm_key;
static void *null_addr;
static void *aligned_addr;
static void *unaligned_addr;

static struct test_case_t {
	void **shmaddr;
	int flag;
	int exp_status;
	char *desp;
} tcases[] = {
	{&null_addr, 0, 0, "NULL address"},
	{&aligned_addr, 0, 0, "aligned address"},
	{&unaligned_addr, SHM_RND, 0, "unaligned address with SHM_RND"},
	{&aligned_addr, SHM_RDONLY, SIGSEGV,
	"aligned address with SHM_READONLY, and got SIGSEGV on write"}
};

static void *expected_addr(void *in_addr, void *out_addr)
{
	if (!in_addr)
		return out_addr;

	return ALIGN_DOWN(in_addr);
}

static void do_child(int *in_addr, int expect_crash)
{
	if (expect_crash)
		tst_no_corefile(1);

	*in_addr = 10;

	exit(0);
}

static int expected_status(int status, int exp_status)
{
	if (!exp_status && WIFEXITED(status))
		return 0;

	if (exp_status && WIFSIGNALED(status) && WTERMSIG(status) == exp_status)
		return 0;

	return 1;
}

static void verify_shmat(unsigned int n)
{
	int *addr;
	pid_t pid;
	int status;
	struct shmid_ds buf;

	struct test_case_t *tc = &tcases[n];

	addr = shmat(shm_id, *tc->shmaddr, tc->flag);
	if (addr == (void *)-1) {
		tst_res(TFAIL | TERRNO, "shmat() failed");
		return;
	}

	SAFE_SHMCTL(shm_id, IPC_STAT, &buf);

	if (buf.shm_nattch != 1) {
		tst_res(TFAIL, "number of attaches was incorrect");
		goto end;
	}

	if (buf.shm_segsz != INT_SIZE) {
		tst_res(TFAIL, "segment size was incorrect");
		goto end;
	}

	if (expected_addr(*tc->shmaddr, addr) != addr) {
		tst_res(TFAIL,
			"shared memory address %p is not correct, expected %p",
			addr, expected_addr(*tc->shmaddr, addr));
		goto end;
	}

	pid = SAFE_FORK();
	if (!pid)
		do_child(addr, tc->exp_status == SIGSEGV);
	else
		SAFE_WAITPID(pid, &status, 0);

	if (expected_status(status, tc->exp_status))
		tst_res(TFAIL, "shmat() failed to attach %s", tc->desp);
	else
		tst_res(TPASS, "shmat() succeeded to attach %s", tc->desp);

end:
	SAFE_SHMDT(addr);
}

static void setup(void)
{
	aligned_addr = PROBE_FREE_ADDR();
	unaligned_addr = aligned_addr + SHMLBA - 1;

	shm_key = GETIPCKEY();

	shm_id = SAFE_SHMGET(shm_key, INT_SIZE, SHM_RW | IPC_CREAT | IPC_EXCL);
}

static void cleanup(void)
{
	if (shm_id != -1)
		SAFE_SHMCTL(shm_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_shmat,
	.tcnt = ARRAY_SIZE(tcases)
};
