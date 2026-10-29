// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * semop05 - test for EINTR and EIDRM errors
 *
 * Copyright (c) International Business Machines  Corp., 2001
 *	03/2001 - Written by Wayne Boyer
 *	14/03/2008 Matthieu Fertré (Matthieu.Fertre@irisa.fr)
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "tse_newipc.h"
#include "lapi/sem.h"
#include "semop.h"

static key_t semkey;
static int sem_id = -1;
static struct tst_ts timeout;

struct test_case_t {
	union semun semunptr;
	short op;
	short flg;
	short num;
	int error;
} tc[] = {
	{{1}, 0, 0, 2, EIDRM},
	{{0}, -1, 0, 3, EIDRM},
	{{1}, 0, 0, 4, EINTR},
	{{0}, -1, 0, 5, EINTR}
};

static void do_child(int i)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct sembuf s_buf = {
		.sem_op = tc[i].op,
		.sem_flg = tc[i].flg,
		.sem_num = tc[i].num,
	};

	TEST(call_semop(tv, sem_id, &s_buf, 1, tst_ts_get(&timeout)));
	if (TST_RET != -1) {
		tst_res(TFAIL, "call succeeded when error expected");
		exit(0);
	}

	if (TST_ERR == tc[i].error)
		tst_res(TPASS | TTERRNO, "expected failure");
	else
		tst_res(TFAIL | TTERRNO, "unexpected failure");

	exit(0);
}

static void sighandler(int sig)
{
	if (sig != SIGHUP)
		tst_brk(TBROK, "unexpected signal %d received", sig);
}

static void setup(void)
{
	struct time64_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	semop_supported_by_kernel(tv);

	timeout.type = tv->ts_type;
	tst_ts_set_sec(&timeout, 2);
	tst_ts_set_nsec(&timeout, 10000000);

	SAFE_SIGNAL(SIGHUP, sighandler);
	semkey = GETIPCKEY();

	sem_id = semget(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA);
	if (sem_id == -1)
		tst_brk(TBROK | TERRNO, "couldn't create semaphore in setup");
}

static void cleanup(void)
{
	if (sem_id != -1) {
		if (semctl(sem_id, 0, IPC_RMID) == -1)
			tst_res(TWARN, "semaphore deletion failed.");
	}
}

static void run(unsigned int i)
{
	pid_t pid;

	if (semctl(sem_id, tc[i].num, SETVAL, tc[i].semunptr) == -1)
		tst_brk(TBROK | TERRNO, "semctl() failed");

	pid = SAFE_FORK();

	if (pid == 0) {
		do_child(i);
	} else {
		TST_PROCESS_STATE_WAIT(pid, 'S', 0);

		/*
		 * If we are testing for EIDRM then remove
		 * the semaphore, else send a signal that
		 * must be caught as we are testing for
		 * EINTR.
		 */
		if (tc[i].error == EIDRM) {
			/* remove the semaphore resource */
			cleanup();
		} else {
			SAFE_KILL(pid, SIGHUP);
		}

		waitpid(pid, NULL, 0);
	}

	if (tc[i].error == EINTR)
		return;

	sem_id = semget(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA);
	if (sem_id == -1)
		tst_brk(TBROK | TERRNO, "couldn't recreate semaphore");
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tc),
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
};
