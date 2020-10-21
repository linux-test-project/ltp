// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *
 * 06/30/2001   Port to Linux   nsharoff@us.ibm.com
 * 11/11/2002   Port to LTP     dbarrera@us.ibm.com
 * 10/21/2020   Convert to new api xuyang2018.jy@cn.fujitsu.com
 * Get and manipulate a message queue.
 * Same as msgstress02 but gets the actual msgmni value under procfs.
 */

#define _XOPEN_SOURCE 500
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "tst_test.h"
#include "libnewipc.h"
#include "tst_safe_sysv_ipc.h"
#include "msgstress_common.h"

#define MAXNREPS	1000
#define MAXNPROCS	 1000000	/* This value is set to an arbitrary high limit. */
#define MAXNKIDS	10

static key_t keyarray[MAXNPROCS];
static int pidarray[MAXNPROCS];
static int rkidarray[MAXNKIDS];
static int wkidarray[MAXNKIDS];
static int tid;
static int nprocs, nreps, nkids, MSGMNI;
static int maxnprocs;
static void cleanup(void);

static void dotest(key_t, int);
static void dotest_iteration(int off);

static char *opt_maxnprocs;
static char *opt_nkids;
static char *opt_nreps;

static struct tst_option options[] = {
	{"n:", &opt_maxnprocs, "-n N     Number of processes"},
	{"c:", &opt_nkids, "-c -N    Number of read/write child pairs"},
	{"l:", &opt_nreps, "-l N     Number of iterations"},
	{NULL, NULL, NULL}
};

static void cleanup_msgqueue(int i, int tid)
{
	/*
	 * Decrease the value of i by 1 because it is getting incremented
	 * even if the fork is failing.
	 */
	i--;
	/* Kill all children & free message queue. */
	for (; i >= 0; i--) {
		(void)kill(rkidarray[i], SIGKILL);
		(void)kill(wkidarray[i], SIGKILL);
	}
	SAFE_MSGCTL(tid, IPC_RMID, 0);
}

static void verify_msgstress(void)
{
	int i, j, ok;

	srand48((unsigned)getpid() + (unsigned)(getppid() << 16));
	tid = -1;

	/* Set up array of unique keys for use in allocating message queues */
	for (i = 0; i < MSGMNI; i++) {
		ok = 1;
		do {
			/* Get random key */
			keyarray[i] = (key_t) lrand48();
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
	/* Fork a number of processes, each of which will
	 * create a message queue with several (nkids) reader/writer
	 * pairs which will read and write a number (iterations)
	 * of random length messages with specific values (keys).
	 *
	 * We do not fork more than maxnprocs at a time and
	 * we fork until all the message queues get used.
	 */

	if (MSGMNI <= maxnprocs) {
		nprocs = MSGMNI;
		dotest_iteration(0);
	} else {
		for (i = 0; i < (MSGMNI / maxnprocs); i++) {
			nprocs = maxnprocs;
			dotest_iteration(i * maxnprocs);
		}

		nprocs = MSGMNI % maxnprocs;
		dotest_iteration(i * maxnprocs);
	}

	tst_res(TPASS, "Test ran successfully!");

	cleanup();
}

static void dotest_iteration(int off)
{
	key_t key;
	int i, count;
	pid_t pid;

	memset(pidarray, 0, sizeof(pidarray));

	for (i = 0; i < nprocs; i++) {
		key = keyarray[off + i];
		pid = SAFE_FORK();

		/* Child does this */
		if (pid == 0) {
			dotest(key, i);
			exit(0);
		}
		pidarray[i] = pid;
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
	/* Make sure proper number of children exited */
	if (count != nprocs)
		tst_brk(TFAIL, "Wrong number of children exited, Saw %d, Expected %d",
				count, nprocs);
}


static void dotest(key_t key, int child_process)
{
	int id, pid;
	int i, count;

	id = SAFE_MSGGET(key, IPC_CREAT | S_IRUSR | S_IWUSR);
	tid = id;
	for (i = 0; i < nkids; i++) {
		pid = fork();
		if (pid < 0) {
			tst_res(TFAIL, "Fork failure in the first child of child group %d\n",
				child_process);
			cleanup_msgqueue(i, tid);
			return;
		}
		/* First child does this */
		if (pid == 0) {
			do_reader(key, tid, getpid(), child_process, nreps);
			exit(0);
		}
		rkidarray[i] = pid;
		pid = fork();
		if (pid < 0) {
			tst_res(TFAIL, "Fork failure in the first child of child group %d\n",
				child_process);
			cleanup_msgqueue(i, tid);
			return;
		}
		/* Second child does this */
		if (pid == 0) {
			do_writer(key, tid, rkidarray[i], child_process, nreps);
			exit(0);
		}
		wkidarray[i] = pid;
	}
	/* Parent does this */
	count = 0;
	while (1) {
		if (wait(NULL) > 0) {
			count++;
		} else {
			if (errno != EINTR)
				break;
		}
	}
	/* Make sure proper number of children exited */
	if (count != (nkids * 2)) {
		tst_res(TFAIL, "Wrong number of children exited in child group %d, saw %d, expected %d\n",
			child_process, count, (nkids * 2));
	}
	SAFE_MSGCTL(id, IPC_RMID, NULL);
}

static void setup(void)
{
	int nr_msgqs, free_pids;

	tid = -1;
	SAFE_FILE_SCANF("/proc/sys/kernel/msgmni", "%d", &nr_msgqs);

	MSGMNI = nr_msgqs - GET_USED_QUEUES();
	if (MSGMNI <= 0)
		tst_brk(TCONF, "Max number of message queues already used, "
			"cannot create more.");

	tst_res(TINFO, "Found %d available message queues", MSGMNI);

	free_pids = tst_get_free_pids();
	if (free_pids < 0) {
		tst_brk(TBROK, "Can't obtain free_pid count");
	} else if (!free_pids) {
		tst_brk(TBROK, "No free pids");
	}

	if (opt_nreps) {
		nreps = SAFE_STRTOL(opt_nreps, 1, INT_MAX);
		nreps = min(nreps, MAXNREPS);
	} else {
		nreps = MAXNREPS;
	}

	if (opt_nkids) {
		nkids = SAFE_STRTOL(opt_nkids, 1, INT_MAX);
		nkids = min(nkids, MAXNKIDS);
	} else {
		nkids = MAXNKIDS;
	}


	/* We don't use more than a half of available pids.
	 * For each child we fork up to 2*maxnkids grandchildren. */
	maxnprocs = (free_pids / 2) / (1 + 2 * nkids);

	if (!maxnprocs)
		tst_brk(TBROK, "Not enough free pids");

	if (opt_maxnprocs)
		maxnprocs = SAFE_STRTOL(opt_maxnprocs, 1, INT_MAX);

	SAFE_SIGNAL(SIGTERM, SIG_IGN);
	tst_res(TINFO, "Using upto %d pids, %d processes %d read/write pairs, %d repeats",
		free_pids/2, maxnprocs, nkids, nreps);
}

static void cleanup(void)
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
