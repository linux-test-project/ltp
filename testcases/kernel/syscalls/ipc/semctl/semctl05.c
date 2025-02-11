// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 */
/*
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 */
/*\
 * Test for semctl() ERANGE error
 */

#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"
#include "lapi/sem.h"
#include "libnewipc.h"

static int sem_id = -1;

#define BIGV	65535		/* a number ((2^16)-1) that should be larger */
				/* than the maximum for a semaphore value    */

unsigned short big_arr[] = { BIGV, BIGV, BIGV, BIGV, BIGV, BIGV, BIGV, BIGV,
	BIGV, BIGV
};

static struct tcases {
	int count;
	int cmd;
	union semun t_arg;
	char *message;
} tests[] = {
	{5, SETVAL, {.val = -1}, "the value to set is less than zero"},
	{0, SETALL, {.array = big_arr}, "the value to set are too large"},
	{5, SETVAL, {.val = BIGV}, "the value to set is too large"}
};

static void verify_semctl(unsigned int n)
{
	struct tcases *tc = &tests[n];

	TST_EXP_FAIL(semctl(sem_id, tc->count, tc->cmd, tc->t_arg), ERANGE,
		     "semctl() with %s", tc->message);
}

static void setup(void)
{
	static key_t semkey;

	semkey = GETIPCKEY();

	sem_id = SAFE_SEMGET(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA);
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
};
