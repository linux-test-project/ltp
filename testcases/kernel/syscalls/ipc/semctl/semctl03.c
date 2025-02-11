// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 */
/*\
 * Test for semctl() EINVAL and EFAULT errors
 */

#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "lapi/sem.h"
#include "libnewipc.h"
#include "lapi/syscalls.h"

static int sem_id = -1;
static int bad_id = -1;

static struct semid_ds sem_ds;
static union semun sem_un = {.buf = &sem_ds};
static void *semds_ptr = &sem_un;
static void *bad_ptr;
static union semun arg = {0};

static int libc_semctl(int semid, int semnum, int cmd, ...)
{
	va_list ap;

	va_start(ap, cmd);
	arg = va_arg(ap, union semun);
	va_end(ap);
	return semctl(semid, semnum, cmd, arg);
}

static int sys_semctl(int semid, int semnum, int cmd, ...)
{
	va_list ap;

	va_start(ap, cmd);
	arg = va_arg(ap, union semun);
	va_end(ap);
	return tst_syscall(__NR_semctl, semid, semnum, cmd, arg);
}

static struct tcases {
	int *sem_id;
	int ipc_cmd;
	void **buf;
	int error;
	char *message;
} tests[] = {
	{&sem_id, -1, &semds_ptr, EINVAL, "invalid IPC command"},
	{&bad_id, IPC_STAT, &semds_ptr, EINVAL, "invalid sem id"},
	{&sem_id, GETALL, &bad_ptr, EFAULT, "invalid union arg"},
	{&sem_id, IPC_SET, &bad_ptr, EFAULT, "invalid union arg"}
};

static struct test_variants
{
	int (*semctl)(int semid, int semnum, int cmd, ...);
	char *desc;
} variants[] = {
	{ .semctl = libc_semctl, .desc = "libc semctl()"},
#if (__NR_sys_semctl != __LTP__NR_INVALID_SYSCALL)
	{ .semctl = sys_semctl,  .desc = "__NR_semctl syscall"},
#endif
};

static void verify_semctl(unsigned int n)
{
	struct tcases *tc = &tests[n];
	struct test_variants *tv = &variants[tst_variant];

	if (tc->error == EFAULT && tv->semctl == libc_semctl) {
		tst_res(TCONF, "EFAULT is skipped for libc variant");
		return;
	}

	TST_EXP_FAIL(tv->semctl(*(tc->sem_id), 0, tc->ipc_cmd, *(tc->buf)),
		tc->error, "semctl() with %s", tc->message);
}

static void setup(void)
{
	static key_t semkey;
	struct test_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);

	semkey = GETIPCKEY();

	sem_id = SAFE_SEMGET(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA);

	bad_ptr = tst_get_bad_addr(NULL);
}

static void cleanup(void)
{
	if (sem_id != -1)
		SAFE_SEMCTL(sem_id, 0, IPC_RMID);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_semctl,
	.tcnt = ARRAY_SIZE(tests),
	.test_variants = ARRAY_SIZE(variants),
};
