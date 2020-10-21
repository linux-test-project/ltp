// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 *
 * 06/30/2001   Port to Linux   nsharoff@us.ibm.com
 * 11/06/2002   Port to LTP     dbarrera@us.ibm.com
 * 10/09/2020   Convert to new api xuyang2018.jy@cn.fujitsu.com
 *
 * Get and manipulate a message queue.
 * Same as msgstress01 but gets the actual msgmni value under procf.
 */

#define _XOPEN_SOURCE 500
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "tst_test.h"
#include "libnewipc.h"
#include "tst_safe_sysv_ipc.h"
#include "msgstress_common.h"

#define MAXNPROCS       1000
#define MAXNREPS	1000

static key_t keyarray[MAXNPROCS];
static int tid;
static int MSGMNI, nprocs, nreps;
static char *opt_nprocs;
static char *opt_nreps;
static void cleanup(void);

static struct tst_option options[] = {
	{"n:", &opt_nprocs, "-n N     Number of processes"},
	{"l:", &opt_nreps, "-l N     Number of iterations"},
	{NULL, NULL, NULL}
};

static void dotest(key_t key, int child_process)
{
	int pid;

	tid = SAFE_MSGGET(key, IPC_CREAT | S_IRUSR | S_IWUSR);

	pid = SAFE_FORK();
	if (pid == 0) {
		do_reader(key, tid, 1, child_process, nreps);
		exit(0);
	}

	do_writer(key, tid, 1, child_process, nreps);
	SAFE_WAIT(NULL);
	SAFE_MSGCTL(tid, IPC_RMID, NULL);
}

static void verify_msgstress(void)
{
	int i, j, ok, pid;
	int count;

	srand(getpid());
	tid = -1;

	/* Set up array of unique keys for use in allocating message queues */
	for (i = 0; i < nprocs; i++) {
		ok = 1;
		do {
			keyarray[i] = (key_t) rand();
			/* Make sure key is unique and not private */
			if (keyarray[i] == IPC_PRIVATE) {
				ok = 0;
				continue;
			}
			for (j = 0; j < i; j++) {
				if (keyarray[j] == keyarray[i]) {
					ok = 0;
					break;
				}
				ok = 1;
			}
		} while (ok == 0);
	}

	/*
	 * Fork a number of processes, each of which will
	 * create a message queue with one reader/writer
	 * pair which will read and write a number (iterations)
	 * of random length messages with specific values.
	 */
	for (i = 0; i < nprocs; i++) {
		pid = SAFE_FORK();
		if (pid == 0) {
			dotest(keyarray[i], i);
			exit(0);
		}
	}

	count = 0;
	while (1) {
		if (wait(NULL) > 0) {
			count++;
		} else {
			if (errno != EINTR)
				break;
		}
	}
	if (count != nprocs)
		tst_brk(TFAIL, "Wrong number of children exited, Saw %d, Expected %d",
			count, nprocs);

	tst_res(TPASS, "Test ran successfully!");
	cleanup();
}

static void setup(void)
{
	int nr_msgqs;

	SAFE_FILE_SCANF("/proc/sys/kernel/msgmni", "%d", &nr_msgqs);

	MSGMNI = nr_msgqs - GET_USED_QUEUES();
	if (MSGMNI > MAXNPROCS)
		MSGMNI = MAXNPROCS;
	if (MSGMNI <= 0)
		tst_brk(TCONF, "Max number of message queues already used, "
			"cannot create more.");

	if (opt_nreps) {
		nreps = SAFE_STRTOL(opt_nreps, 1, INT_MAX);
		nreps = min(nreps, MAXNREPS);
	} else {
		nreps = MAXNREPS;
	}

	if (opt_nprocs) {
		nprocs = SAFE_STRTOL(opt_nprocs, 1, INT_MAX);
		nprocs = min(nprocs, MSGMNI);
	} else {
		nprocs = MSGMNI;
	}

	SAFE_SIGNAL(SIGTERM, SIG_IGN);
	tst_res(TINFO, "Number of message queues is %d, process is %d, "
		"iterations is %d", MSGMNI, nprocs, nreps);
}

void cleanup(void)
{
	if (tid >= 0)
		SAFE_MSGCTL(tid, IPC_RMID, NULL);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.options = options,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.test_all = verify_msgstress,
};
