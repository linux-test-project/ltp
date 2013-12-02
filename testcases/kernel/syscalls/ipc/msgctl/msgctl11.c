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
 * Same as msgctl09 but gets the actual msgmni value under procfs.
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
#include "usctest.h"
#include "ipcmsg.h"
#include "../lib/libmsgctl.h"
#include "system_specific_process_info.h"

char *TCID = "msgctl11";
int TST_TOTAL = 1;

#define MAXNREPS	1000
#ifndef CONFIG_COLDFIRE
#define MAXNPROCS	 1000000	/* This value is set to an arbitrary high limit. */
#else
#define MAXNPROCS	 100000	/* Coldfire can't deal with 1000000 */
#endif
#define MAXNKIDS	10

static int maxnkids = MAXNKIDS;	/* Used if pid_max is exceeded */
static key_t keyarray[MAXNPROCS];
static int pidarray[MAXNPROCS];
static int rkidarray[MAXNKIDS];
static int wkidarray[MAXNKIDS];
static int tid;
static int nprocs, nreps, nkids, MSGMNI;
static int procstat;

void setup(void);
void cleanup(void);

static void term(int);
static int dotest(key_t, int);
static void cleanup_msgqueue(int i, int tid);

#ifdef UCLINUX
static char *argv0;
static key_t key_uclinux;
static int i_uclinux;
static int pid_uclinux;
static int child_process_uclinux;
static int rkid_uclinux;

static void do_child_1_uclinux();
static void do_child_2_uclinux();
static void do_child_3_uclinux();
#endif

int main(int argc, char **argv)
{
	int i, j, ok, pid;
	int count, status;

#ifdef UCLINUX
	char *msg;

	argv0 = argv[0];

	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	maybe_run_child(&do_child_1_uclinux, "ndd", 1, &key_uclinux,
			&i_uclinux);
	maybe_run_child(&do_child_2_uclinux, "nddd", 2, &key_uclinux,
			&pid_uclinux, &child_process_uclinux);
	maybe_run_child(&do_child_3_uclinux, "nddd", 3, &key_uclinux,
			&rkid_uclinux, &child_process_uclinux);
#endif

	setup();

	if (argc == 1) {
		/* Set default parameters */
		nreps = MAXNREPS;
		nprocs = MSGMNI;
		nkids = maxnkids;
	} else if (argc == 4) {
		if (atoi(argv[1]) > MAXNREPS) {
			tst_resm(TCONF,
				 "Requested number of iterations too large, setting to Max. of %d",
				 MAXNREPS);
			nreps = MAXNREPS;
		} else {
			nreps = atoi(argv[1]);
		}
		if (atoi(argv[2]) > MSGMNI) {
			tst_resm(TCONF,
				 "Requested number of processes too large, setting to Max. of %d",
				 MSGMNI);
			nprocs = MSGMNI;
		} else {
			nprocs = atoi(argv[2]);
		}
		if (atoi(argv[3]) > maxnkids) {
			tst_resm(TCONF,
				 "Requested number of read/write pairs too large; setting to Max. of %d",
				 maxnkids);
			nkids = maxnkids;
		} else {
			nkids = atoi(argv[3]);
		}
	} else {
		tst_resm(TCONF,
			 " Usage: %s [ number of iterations  number of processes number of read/write pairs ]",
			 argv[0]);
		tst_exit();
	}

	procstat = 0;
	srand48((unsigned)getpid() + (unsigned)(getppid() << 16));
	tid = -1;

	/* Setup signal handleing routine */
	if (sigset(SIGTERM, term) == SIG_ERR) {
		tst_resm(TFAIL, "Sigset SIGTERM failed");
		tst_exit();
	}
	/* Set up array of unique keys for use in allocating message
	 * queues
	 */
	for (i = 0; i < nprocs; i++) {
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
	/* Fork a number of processes (nprocs), each of which will
	 * create a message queue with several (nkids) reader/writer
	 * pairs which will read and write a number (iterations)
	 * of random length messages with specific values (keys).
	 */

	for (i = 0; i < nprocs; i++) {
		fflush(stdout);
		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_resm(TFAIL,
				 "\tFork failed (may be OK if under stress)");
			tst_exit();
		}
		/* Child does this */
		if (pid == 0) {
#ifdef UCLINUX
			if (self_exec(argv[0], "ndd", 1, keyarray[i], i) < 0) {
				tst_resm(TFAIL, "\tself_exec failed");
				tst_exit();
			}
#else
			procstat = 1;
			exit(dotest(keyarray[i], i));
#endif
		}
		pidarray[i] = pid;
	}

	count = 0;
	while (1) {
		if ((wait(&status)) > 0) {
			if (status >> 8 != PASS) {
				tst_resm(TFAIL, "Child exit status = %d",
					 status >> 8);
				tst_exit();
			}
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
	if (count != nprocs) {
		tst_resm(TFAIL,
			 "Wrong number of children exited, Saw %d, Expected %d",
			 count, nprocs);
		tst_exit();
	}

	tst_resm(TPASS, "msgctl11 ran successfully!");

	cleanup();

	return (0);

}

#ifdef UCLINUX
static void do_child_1_uclinux(void)
{
	procstat = 1;
	exit(dotest(key_uclinux, i_uclinux));
}

static void do_child_2_uclinux(void)
{
	procstat = 2;
	exit(doreader(key_uclinux, tid, pid_uclinux,
			child_process_uclinux, nreps));
}

static void do_child_3_uclinux(void)
{
	procstat = 2;
	exit(dowriter(key_uclinux, tid, rkid_uclinux,
			child_process_uclinux, nreps));
}
#endif

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
		tst_resm(TFAIL | TERRNO, "Msgctl error in cleanup");
		tst_exit();
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
		fflush(stdout);
		if ((pid = FORK_OR_VFORK()) < 0) {
			printf("Fork failure in the first child of child group %d\n",
				child_process);
			cleanup_msgqueue(i, tid);
			return FAIL;
		}
		/* First child does this */
		if (pid == 0) {
#ifdef UCLINUX
			if (self_exec(argv0, "nddd", 2, key, getpid(),
				      child_process) < 0) {
				printf("self_exec failed\n");
				cleanup_msgqueue(i, tid);
				return FAIL;
			}
#else
			procstat = 2;
			exit(doreader(key, tid, getpid(),
					child_process, nreps));
#endif
		}
		rkidarray[i] = pid;
		fflush(stdout);
		if ((pid = FORK_OR_VFORK()) < 0) {
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
#ifdef UCLINUX
			if (self_exec(argv0, "nddd", 3, key, rkidarray[i],
				      child_process) < 0) {
				printf("\tFork failure in the first child of child group %d\n",
					child_process);
				/*
				 * Kill the reader child process
				 */
				(void)kill(rkidarray[i], SIGKILL);

				cleanup_msgqueue(i, tid);
				return FAIL;
			}
#else
			procstat = 2;
			exit(dowriter(key, tid, rkidarray[i],
					child_process, nreps));
#endif
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
static void term(int sig)
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
		fflush(stdout);
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
		cleanup();

	MSGMNI = nr_msgqs - get_used_msgqueues();
	if (MSGMNI <= 0) {
		tst_resm(TBROK,
			 "Max number of message queues already used, cannot create more.");
		cleanup();
	}

	free_pids = get_free_pids();
	if (free_pids < 0) {
		tst_resm(TBROK, "Can't obtain free_pid count");
		tst_exit();
	}

	else if (!free_pids) {
		tst_resm(TBROK, "No free pids");
		tst_exit();
	}

	if ((MSGMNI * MAXNKIDS * 2) > (free_pids / 2)) {
		maxnkids = ((free_pids / 4) / MSGMNI);
		if (!maxnkids) {
			tst_resm(TBROK, "Not enough free pids");
			tst_exit();
		}
	}

	tst_resm(TINFO, "Using upto %d pids", free_pids / 2);
}

void cleanup(void)
{
	int status;

	TEST_CLEANUP;

	/*
	 * Remove the message queue from the system
	 */
#ifdef DEBUG
	tst_resm(TINFO, "Removing the message queue");
#endif
	fflush(stdout);
	(void)msgctl(tid, IPC_RMID, NULL);
	if ((status = msgctl(tid, IPC_STAT, NULL)) != -1) {
		(void)msgctl(tid, IPC_RMID, NULL);
		tst_resm(TFAIL, "msgctl(tid, IPC_RMID) failed");

	}

	fflush(stdout);
	tst_rmdir();
}
