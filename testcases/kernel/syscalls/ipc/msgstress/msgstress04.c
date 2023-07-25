/*
 * Copyright (c) International Business Machines  Corp., 2002
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * 06/30/2001   Port to Linux   nsharoff@us.ibm.com
 * 11/11/2002   Port to LTP     dbarrera@us.ibm.com
 */

/*
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
#include "test.h"
#include "ipcmsg.h"
#include "libmsgctl.h"

char *TCID = "msgstress04";
int TST_TOTAL = 1;

#define MAXNREPS	1000
#ifndef CONFIG_COLDFIRE
#define MAXNPROCS	 1000000	/* This value is set to an arbitrary high limit. */
#else
#define MAXNPROCS	 100000	/* Coldfire can't deal with 1000000 */
#endif
#define MAXNKIDS	10
#define DEFNKIDS	2

static int maxnkids = MAXNKIDS;	/* Used if pid_max is exceeded */
static key_t keyarray[MAXNPROCS];
static int pidarray[MAXNPROCS];
static int rkidarray[MAXNKIDS];
static int wkidarray[MAXNKIDS];
static int tid;
static int nprocs, nreps, nkids, MSGMNI;
static int maxnprocs;
static int procstat;

void setup(void);
void cleanup(void);

static void term(int);
static int dotest(key_t, int);
static void dotest_iteration(int off);
static void cleanup_msgqueue(int i, int tid);

static char *opt_maxnprocs;
static char *opt_nkids;
static char *opt_nreps;

static option_t options[] = {
	{"n:", NULL, &opt_maxnprocs},
	{"c:", NULL, &opt_nkids},
	{"l:", NULL, &opt_nreps},
	{NULL, NULL, NULL},
};

static void usage(void)
{
	printf("  -n      Number of processes\n");
	printf("  -c      Number of read/write child pairs\n");
	printf("  -l      Number of iterations\n");
}


int main(int argc, char **argv)
{
	int i, j, ok;

	tst_parse_opts(argc, argv, options, usage);

	setup();

	nreps = MAXNREPS;
	nkids = MAXNKIDS;

	if (opt_nreps) {
		nreps = atoi(opt_nreps);
		if (nreps > MAXNREPS) {
			tst_resm(TINFO,
				 "Requested number of iterations too large, "
				 "setting to Max. of %d", MAXNREPS);
			nreps = MAXNREPS;
		}
	}

	if (opt_nkids) {
		nkids = atoi(opt_nkids);
		if (nkids > MAXNKIDS) {
			tst_resm(TINFO,
				 "Requested number of read/write pairs too "
				 "large, setting to Max. of %d", MAXNKIDS);
			nkids = MAXNKIDS;
		}
	}


	if (opt_maxnprocs) {
		if (atoi(opt_maxnprocs) > maxnprocs) {
			tst_resm(TINFO,
				 "Requested number of processes too large, "
				 "setting to Max. of %d", MSGMNI);
		} else {
			maxnprocs = atoi(opt_maxnprocs);
		}
	}

	procstat = 0;
	srand48((unsigned)getpid() + (unsigned)(getppid() << 16));
	tid = -1;

	/* Setup signal handling routine */
	if (sigset(SIGTERM, term) == SIG_ERR)
		tst_brkm(TFAIL, cleanup, "Sigset SIGTERM failed");

	/* Set up array of unique keys for use in allocating message
	 * queues
	 */
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

	tst_resm(TPASS, "Test ran successfully!");

	cleanup();
	tst_exit();
}

static void dotest_iteration(int off)
{
	key_t key;
	int i, count, status;
	pid_t pid;

	memset(pidarray, 0, sizeof(pidarray));

	for (i = 0; i < nprocs; i++) {
		key = keyarray[off + i];

		if ((pid = tst_fork()) < 0)
			tst_brkm(TFAIL, cleanup,
				 "Fork failed (may be OK if under stress)");

		/* Child does this */
		if (pid == 0) {
			procstat = 1;
			exit(dotest(key, i));
		}
		pidarray[i] = pid;
	}

	count = 0;
	while (1) {
		if ((wait(&status)) > 0) {
			if (status >> 8 != PASS)
				tst_brkm(TFAIL, cleanup,
					"Child exit status = %d", status >> 8);
			count++;
		} else {
			if (errno != EINTR) {
				break;
			}
#ifdef DEBUG
			tst_resm(TINFO, "Signal detected during wait");
#endif
		}
	}
	/* Make sure proper number of children exited */
	if (count != nprocs)
		tst_brkm(TFAIL, cleanup,
			 "Wrong number of children exited, Saw %d, Expected %d",
			 count, nprocs);
}

static void cleanup_msgqueue(int i, int tid)
{
	/*
	 * Decrease the value of i by 1 because it
	 * is getting incremented even if the fork
	 * is failing.
	 */

	i--;
	/*
	 * Kill all children & free message queue.
	 */
	for (; i >= 0; i--) {
		(void)kill(rkidarray[i], SIGKILL);
		(void)kill(wkidarray[i], SIGKILL);
	}

	if (msgctl(tid, IPC_RMID, 0) < 0) {
		printf("Msgctl error in cleanup_msgqueue %d\n", errno);
		exit(FAIL);
	}
}

static int dotest(key_t key, int child_process)
{
	int id, pid;
	int i, count, status, exit_status;

	sighold(SIGTERM);
	if ((id = msgget(key, IPC_CREAT | S_IRUSR | S_IWUSR)) < 0) {
		printf("msgget() error in child %d: %s\n",
			child_process, strerror(errno));
		return FAIL;
	}
	tid = id;
	sigrelse(SIGTERM);

	exit_status = PASS;

	for (i = 0; i < nkids; i++) {
		if ((pid = tst_fork()) < 0) {
			printf("Fork failure in the first child of child group %d\n",
				child_process);
			cleanup_msgqueue(i, tid);
			return FAIL;
		}
		/* First child does this */
		if (pid == 0) {
			procstat = 2;
			exit(doreader(key, tid, getpid(),
					child_process, nreps));
		}
		rkidarray[i] = pid;
		if ((pid = tst_fork()) < 0) {
			printf("Fork failure in the second child of child group %d\n",
				child_process);
			/*
			 * Kill the reader child process
			 */
			(void)kill(rkidarray[i], SIGKILL);

			cleanup_msgqueue(i, tid);
			return FAIL;
		}
		/* Second child does this */
		if (pid == 0) {
			procstat = 2;
			exit(dowriter(key, tid, rkidarray[i],
					child_process, nreps));
		}
		wkidarray[i] = pid;
	}
	/* Parent does this */
	count = 0;
	while (1) {
		if ((wait(&status)) > 0) {
			if (status >> 8 != PASS) {
				printf("Child exit status = %d from child group %d\n",
					status >> 8, child_process);
				for (i = 0; i < nkids; i++) {
					kill(rkidarray[i], SIGTERM);
					kill(wkidarray[i], SIGTERM);
				}
				if (msgctl(tid, IPC_RMID, 0) < 0) {
					printf("msgctl() error: %s\n",
						strerror(errno));
				}
				return FAIL;
			}
			count++;
		} else {
			if (errno != EINTR) {
				break;
			}
		}
	}
	/* Make sure proper number of children exited */
	if (count != (nkids * 2)) {
		printf("Wrong number of children exited in child group %d, saw %d, expected %d\n",
			child_process, count, (nkids * 2));
		if (msgctl(tid, IPC_RMID, 0) < 0) {
			printf("msgctl() error: %s\n", strerror(errno));
		}
		return FAIL;
	}
	if (msgctl(id, IPC_RMID, 0) < 0) {
		printf("msgctl() failure in child group %d: %s\n",
			child_process, strerror(errno));
		return FAIL;
	}
	return exit_status;
}

/* ARGSUSED */
static void term(int sig LTP_ATTRIBUTE_UNUSED)
{
	int i;

	if (procstat == 0) {
#ifdef DEBUG
		tst_resm(TINFO, "SIGTERM signal received, test killing kids");
#endif
		for (i = 0; i < nprocs; i++) {
			if (pidarray[i] > 0) {
				if (kill(pidarray[i], SIGTERM) < 0) {
					tst_resm(TBROK,
						 "Kill failed to kill child %d",
						 i);
					exit(FAIL);
				}
			}
		}
		return;
	}

	if (procstat == 2) {
		exit(PASS);
	}

	if (tid == -1) {
		exit(FAIL);
	}
	for (i = 0; i < nkids; i++) {
		if (rkidarray[i] > 0)
			kill(rkidarray[i], SIGTERM);
		if (wkidarray[i] > 0)
			kill(wkidarray[i], SIGTERM);
	}
}

void setup(void)
{
	int nr_msgqs, free_pids;

	tst_tmpdir();
	/* You will want to enable some signal handling so you can capture
	 * unexpected signals like SIGSEGV.
	 */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* One cavet that hasn't been fixed yet.  TEST_PAUSE contains the code to
	 * fork the test with the -c option.  You want to make sure you do this
	 * before you create your temporary directory.
	 */
	TEST_PAUSE;

	nr_msgqs = get_max_msgqueues();
	if (nr_msgqs < 0)
		tst_brkm(TBROK, cleanup, "get_max_msgqueues() failed");

	MSGMNI = nr_msgqs - get_used_msgqueues();
	if (MSGMNI <= 0)
		tst_brkm(TBROK, cleanup,
			 "Max number of message queues already used, cannot create more.");

	tst_resm(TINFO, "Found %d available message queues", MSGMNI);

	free_pids = tst_get_free_pids(cleanup);
	/* We don't use more than a half of available pids.
	 * For each child we fork up to 2*maxnkids grandchildren. */
	maxnprocs = (free_pids / 2) / (1 + 2 * maxnkids);

	if (!maxnprocs)
		tst_brkm(TBROK, cleanup, "Not enough free pids");

	tst_resm(TINFO, "Using upto %d pids", free_pids / 2);
}

void cleanup(void)
{
	int status;

	/*
	 * Remove the message queue from the system
	 */
#ifdef DEBUG
	tst_resm(TINFO, "Removing the message queue");
#endif
	(void)msgctl(tid, IPC_RMID, NULL);
	if ((status = msgctl(tid, IPC_STAT, NULL)) != -1) {
		(void)msgctl(tid, IPC_RMID, NULL);
		tst_resm(TFAIL, "msgctl(tid, IPC_RMID) failed");

	}

	tst_rmdir();
}
