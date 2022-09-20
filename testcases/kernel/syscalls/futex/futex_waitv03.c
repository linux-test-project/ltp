// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies futex_waitv syscall using shared data.
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <time.h>
#include <sys/shm.h>
#include "tst_test.h"
#include "lapi/futex.h"
#include "lapi/syscalls.h"
#include "futex2test.h"
#include "futex_utils.h"
#include "tst_safe_pthread.h"
#include "tst_safe_clocks.h"
#include "tst_safe_sysv_ipc.h"

static char *str_numfutex;
static int numfutex = 30;

static struct futex_waitv *waitv;
static unsigned int waitv_allocated;
static int *shmids;

static void setup(void)
{
	struct futex_test_variants tv = futex_variant();
	int i;

	tst_res(TINFO, "Testing variant: %s", tv.desc);
	futex_supported_by_kernel(tv.fntype);

	if (tst_parse_int(str_numfutex, &numfutex, 1, FUTEX_WAITV_MAX))
		tst_brk(TBROK, "Invalid number of futexes '%s'", str_numfutex);

	waitv = tst_alloc(sizeof(struct futex_waitv) * numfutex);
	memset(waitv, 0, sizeof(struct futex_waitv) * numfutex);
	shmids = tst_alloc(sizeof(int*) * numfutex);
	memset(shmids, 0, sizeof(int*) * numfutex);

	for (i = 0; i < numfutex; i++) {
		shmids[i] = SAFE_SHMGET(IPC_PRIVATE, 4096, IPC_CREAT | 0666);
		waitv[i].uaddr = (uintptr_t)SAFE_SHMAT(shmids[i], NULL, 0);
		waitv[i].flags = FUTEX_32;
		waitv[i].val = 0;
	}
	waitv_allocated = tst_variant + 1;
}

static void cleanup(void)
{
	int i;

	if (waitv_allocated != (tst_variant + 1))
		return;

	for (i = 0; i < numfutex; i++) {
		if (!waitv[i].uaddr)
			continue;

		SAFE_SHMDT((void *)(uintptr_t)waitv[i].uaddr);
		SAFE_SHMCTL(shmids[i], IPC_RMID, NULL);
	}
}

static void *threaded(LTP_ATTRIBUTE_UNUSED void *arg)
{
	struct futex_test_variants tv = futex_variant();

	TST_RETRY_FUNC(futex_wake(tv.fntype,
		(void *)(uintptr_t)waitv[numfutex - 1].uaddr,
		1, 0), futex_waked_someone);

	return NULL;
}

static void run(void)
{
	struct timespec to;
	pthread_t t;

	SAFE_PTHREAD_CREATE(&t, NULL, threaded, NULL);

	/* setting absolute timeout for futex2 */
	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC, &to);
	to.tv_sec += 5;

	TEST(futex_waitv(waitv, numfutex, 0, &to, CLOCK_MONOTONIC));
	if (TST_RET < 0) {
		tst_brk(TBROK | TTERRNO, "futex_waitv returned: %ld", TST_RET);
	} else if (TST_RET != numfutex - 1) {
		tst_res(TFAIL, "futex_waitv returned: %ld, expecting %d",
			TST_RET, numfutex - 1);
	}

	SAFE_PTHREAD_JOIN(t, NULL);
	tst_res(TPASS, "futex_waitv returned correctly");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "5.16",
	.test_variants = FUTEX_VARIANTS,
	.options =
		(struct tst_option[]){
			{ "n:", &str_numfutex, "Number of futex (default 30)" },
			{},
		},
};
