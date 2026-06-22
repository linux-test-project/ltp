// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (C) 2003-2023 Linux Test Project, Inc.
 * Author: 2001 Paul Larson <plars@us.ibm.com>
 * Modified: 2001 Manoj Iyer <manjo@ausin.ibm.com>
 */

/*\
 * Creates a semaphore and two processes.  The processes
 * each go through a loop where they semdown, delay for a
 * random amount of time, and semup, so they will almost
 * always be fighting for control of the semaphore.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "lapi/sem.h"
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"

#define LOOPS 1000
#define SEED 123

static void semup(int semid)
{
	struct sembuf semops;

	semops.sem_num = 0;
	semops.sem_op = 1;
	semops.sem_flg = SEM_UNDO;

	SAFE_SEMOP(semid, &semops, 1);
}

static void semdown(int semid)
{
	struct sembuf semops;

	semops.sem_num = 0;
	semops.sem_op = -1;
	semops.sem_flg = SEM_UNDO;

	SAFE_SEMOP(semid, &semops, 1);
}

static void mainloop(int semid)
{
	int i;

	for (i = 0; i < LOOPS; i++) {
		semdown(semid);
		usleep(1 + ((100.0 * rand()) / RAND_MAX));
		semup(semid);
	}
}

static void run(void)
{
	int semid;
	union semun semunion;
	pid_t pid;

	/* set up the semaphore */
	semid = SAFE_SEMGET((key_t) 9142, 1, 0666 | IPC_CREAT);

	semunion.val = 1;

	SAFE_SEMCTL(semid, 0, SETVAL, semunion);

	tst_res(TINFO, "srand seed is %d", SEED);
	srand(SEED);

	pid = SAFE_FORK();

	if (pid) {
		mainloop(semid);
		tst_reap_children();
		TST_EXP_POSITIVE(semctl(semid, 0, IPC_RMID, semunion));
	} else {
		mainloop(semid);
	}
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
};
