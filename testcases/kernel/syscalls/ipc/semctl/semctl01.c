/*
 * Copyright (c) International Business Machines  Corp., 2001
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
 */

/*
 * NAME
 *	semctl01.c
 *
 * DESCRIPTION
 *	semctl01 - test the 13 possible semctl() commands
 *
 * ALGORITHM
 *	create a semaphore set with read and alter permissions
 *	loop if that option was specified
 *	  loop through the test cases
 *	    do any setup required for the test case
 *	    make the semctl() call using the TEST() macro
 *	    check the return code
 *	      if failure, issue a FAIL message.
 *	    otherwise,
 *	      if doing functionality testing
 *		call the appropriate test function
 *		if correct,
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	call cleanup
 */


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/wait.h>
#include "ipcsem.h"
#include "safe_macros.h"

char *TCID = "semctl01";

static int sem_id_1 = -1;
static int sem_index;

/*
 * These are the various setup and check functions for the 10 different
 * commands that are available for the semctl() call.
 */
static void func_stat(void);
static void set_setup(void), func_set(void);
static void func_gall(void);
static void cnt_setup(int), func_cnt(int);
static void pid_setup(void), func_pid(int);
static void func_gval(int);
static void sall_setup(void), func_sall(void);
static void func_sval(void);
static void func_rmid(void);
static void child_cnt(void);
static void child_pid(void);
static void func_iinfo(int);
static void func_sinfo(void);
static void func_sstat(int);

static struct semid_ds buf;
static struct seminfo ipc_buf;
static unsigned short array[PSEMS];
static struct sembuf sops;

#define INCVAL 2
#define NEWMODE	066
#define NCHILD	5
#define SEM2	2
#define SEM4	4
#define ONE	1
#ifdef _XLC_COMPILER
#define SEMUN_CAST
#else
#define SEMUN_CAST (union semun)
#endif

static int pid_arr[NCHILD];

#ifdef UCLINUX
#define PIPE_NAME	"semctl01"
static char *argv0;
static int sem_op;
#endif

static struct test_case_t {
	int *semid;
	int semnum;
	int cmd;
	void (*func_test) ();
	union semun arg;
	void (*func_setup) ();
} TC[] = {
	{&sem_id_1, 0, IPC_STAT, func_stat, SEMUN_CAST & buf, NULL},
	{&sem_id_1, 0, IPC_SET, func_set, SEMUN_CAST & buf, set_setup},
	{&sem_id_1, 0, GETALL, func_gall, SEMUN_CAST array, NULL},
	{&sem_id_1, SEM4, GETNCNT, func_cnt, SEMUN_CAST & buf, cnt_setup},
	{&sem_id_1, SEM2, GETPID, func_pid, SEMUN_CAST & buf, pid_setup},
	{&sem_id_1, SEM2, GETVAL, func_gval, SEMUN_CAST & buf, NULL},
	{&sem_id_1, SEM4, GETZCNT, func_cnt, SEMUN_CAST & buf, cnt_setup},
	{&sem_id_1, 0, SETALL, func_sall, SEMUN_CAST array, sall_setup},
	{&sem_id_1, SEM4, SETVAL, func_sval, SEMUN_CAST INCVAL, NULL},
	{&sem_id_1, 0, IPC_INFO, func_iinfo, SEMUN_CAST & ipc_buf, NULL},
	{&sem_id_1, 0, SEM_INFO, func_sinfo, SEMUN_CAST & ipc_buf, NULL},
	{&sem_index, 0, SEM_STAT, func_sstat, SEMUN_CAST & buf, NULL},
	{&sem_id_1, 0, IPC_RMID, func_rmid, SEMUN_CAST & buf, NULL},
};

int TST_TOTAL = ARRAY_SIZE(TC);

static void kill_all_children(void)
{
	int j, status;

	for (j = 0; j < NCHILD; j++) {
		SAFE_KILL(cleanup, pid_arr[j], SIGKILL);
	}

	/*
	 * make sure children finished before we proceed with next testcase
	 */
	for (j = 0; j < NCHILD; j++) {
		SAFE_WAIT(cleanup, &status);
	}
}

int main(int argc, char *argv[])
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

#ifdef UCLINUX
	argv0 = argv[0];
	maybe_run_child(&child_pid, "nd", 1, &sem_id_1);
	maybe_run_child(&child_cnt, "ndd", 2, &sem_id_1, &sem_op);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			/*
			 * Set up any conditions if needed
			 */
			if (TC[i].func_setup != NULL) {
				/* call the setup function */
				switch (TC[i].cmd) {
				case GETNCNT:
					(*TC[i].func_setup) (-ONE);
					break;
				case GETZCNT:
					(*TC[i].func_setup) (0);
					break;
				default:
					(*TC[i].func_setup) ();
					break;
				}
			}

			TEST(semctl(*(TC[i].semid), TC[i].semnum, TC[i].cmd,
				    TC[i].arg));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "%s call failed - errno = %d "
					 ": %s", TCID, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				/*
				 * call the appropriate test function
				 * and pass the return value where it
				 * is needed to perform certain tests.
				 */
				switch (TC[i].cmd) {
				case GETNCNT:
				case GETZCNT:
				case GETPID:
				case GETVAL:
				case IPC_INFO:
				case SEM_STAT:
					(*TC[i].func_test) (TEST_RETURN);
					break;
				default:
					(*TC[i].func_test) ();
					break;
				}
			}

			/*
			 * If testing GETNCNT or GETZCNT, clean up the children.
			 */
			switch (TC[i].cmd) {
			case GETNCNT:
			case GETZCNT:
				kill_all_children();
				break;
			}
		}
		/*
		 * recreate the semaphore resource if looping
		 */
		if (TEST_LOOPING(lc)) {
			sem_id_1 = semget(semkey, PSEMS,
					  IPC_CREAT | IPC_EXCL | SEM_RA);
			if (sem_id_1 == -1)
				tst_brkm(TBROK, cleanup,
					 "couldn't recreate " "semaphore");
		}
	}

	cleanup();

	tst_exit();
}

/*
 * func_stat() - check the functionality of the IPC_STAT command with semctl()
 */
static void func_stat(void)
{
	/* check the number of semaphores and the ipc_perm.mode value */
	if (buf.sem_nsems == PSEMS && buf.sem_perm.mode == (SEM_RA))
		tst_resm(TPASS, "buf.sem_nsems and buf.sem_perm.mode"
			 " are correct");
	else
		tst_resm(TFAIL, "semaphore STAT info is incorrect");
}

/*
 * set_setup() - set up for the IPC_SET command with semctl()
 */
static void set_setup(void)
{
	/* set up a new mode for the semaphore set */
	buf.sem_perm.mode = SEM_RA | NEWMODE;
}

/*
 * func_set() - check the functionality of the IPC_SET command with semctl()
 */
static void func_set(void)
{
	/* first stat the semaphore to get the new data */
	if (semctl(sem_id_1, 0, IPC_STAT, (union semun)&buf) == -1) {
		tst_resm(TBROK, "stat failed in func_set()");
		return;
	}

	/* check that the new mode is what we set */
	if (buf.sem_perm.mode == (SEM_RA | NEWMODE))
		tst_resm(TPASS, "buf.sem_perm.mode is correct");
	else
		tst_resm(TFAIL, "semaphore mode info is incorrect");
}

/*
 * func_gall() - check the functionality of the GETALL command with semctl()
 */
static void func_gall(void)
{
	int i;

	/* the initial value of the primitive semaphores should be zero */
	for (i = 0; i < PSEMS; i++) {
		if (array[i] != 0) {
			tst_resm(TFAIL, "semaphore %d has unexpected value", i);
			return;
		}
	}
	tst_resm(TPASS, "semaphores have expected values");
}

/*
 * cnt_setup() - set up for the GETNCNT and GETZCNT commands with semctl()
 */
static void cnt_setup(int opval)
{
	int pid, i;

	sops.sem_num = SEM4;
	sops.sem_flg = 0;

	/*
	 * if seting up for GETZCNT, the semaphore value needs to be positive
	 */
	if (opval == 0) {
		/* initialize the semaphore value to ONE */
		sops.sem_op = ONE;
		if (semop(sem_id_1, &sops, 1) == -1)
			tst_brkm(TBROK, cleanup, "semop #1 failed - cnt_setup");
	}

	/* set the correct operation */
	sops.sem_op = opval;
	for (i = 0; i < NCHILD; i++) {
		/* fork five children to wait */
		pid = FORK_OR_VFORK();
		if (pid == -1)
			tst_brkm(TBROK, cleanup, "fork failed in cnt_setup");

		if (pid == 0) {
#ifdef UCLINUX
			sem_op = sops.sem_op;
			if (self_exec(argv0, "ndd", 2, sem_id_1, sem_op) < 0)
				tst_brkm(TBROK, cleanup, "self_exec failed "
					 "in cnt_setup");
#else
			child_cnt();
#endif
		} else {
			TST_PROCESS_STATE_WAIT(cleanup, pid, 'S');
			/* save the pid so we can kill it later */
			pid_arr[i] = pid;
		}
	}
}

static void child_cnt(void)
{
#ifdef UCLINUX
	sops.sem_op = (short int)sem_op;
#endif

	sops.sem_num = SEM4;
	sops.sem_flg = 0;

	/*
	 * Do a semop that will cause the child to sleep.
	 * The child process will be killed in the func_ncnt
	 * routine which should cause an error to be return
	 * by the semop() call.
	 */
	if (semop(sem_id_1, &sops, 1) != -1)
		tst_resm(TBROK, "semop succeeded - cnt_setup");

	exit(0);
}

/*
 * func_cnt() - check the functionality of the GETNCNT and GETZCNT commands
 *	        with semctl()
 */
static void func_cnt(int rval)
{

	if (rval == NCHILD)
		tst_resm(TPASS, "number of sleeping processes is correct");
	else
		tst_resm(TFAIL, "number of sleeping processes is not correct");
}

/*
 * pid_setup() - set up for the GETPID command with semctl()
 */
static void pid_setup(void)
{
	int pid;

	/*
	 * Fork a child to do a semop that will pass.
	 */
	pid = FORK_OR_VFORK();
	if (pid == -1)
		tst_brkm(TBROK, cleanup, "fork failed in pid_setup()");

	if (pid == 0) {		/* child */
#ifdef UCLINUX
		if (self_exec(argv0, "nd", 1, sem_id_1) < 0)
			tst_brkm(TBROK, cleanup, "self_exec failed "
				 "in pid_setup()");
#else
		child_pid();
#endif
	} else {
		pid_arr[SEM2] = pid;
		TST_PROCESS_STATE_WAIT(cleanup, pid, 'Z');
	}
}

static void child_pid(void)
{
	sops.sem_num = SEM2;
	sops.sem_op = ONE;
	sops.sem_flg = 0;
	/*
	 * Do a semop that will increment the semaphore.
	 */
	if (semop(sem_id_1, &sops, 1) == -1)
		tst_resm(TBROK, "semop failed - pid_setup");
	exit(0);
}

/*
 * func_pid() - check the functionality of the GETPID command with semctl()
 */
static void func_pid(int rval)
{
	/* compare the rval (pid) to the saved pid from the setup */
	if (rval == pid_arr[SEM2])
		tst_resm(TPASS, "last pid value is correct");
	else
		tst_resm(TFAIL, "last pid value is not correct");
}

/*
 * func_gval() - check the functionality of the GETVAL command with semctl()
 */
static void func_gval(int rval)
{
	/*
	 * This is a simple test.  The semaphore value should be equal
	 * to ONE as it was set in the last test (GETPID).
	 */
	if (rval == 1)
		tst_resm(TPASS, "semaphore value is correct");
	else
		tst_resm(TFAIL, "semaphore value is not correct");
}

/*
 * all_setup() - set up for the SETALL command with semctl()
 */
static void sall_setup(void)
{
	int i;

	for (i = 0; i < PSEMS; i++) {
		/* initialize the array values to 3 */
		array[i] = 3;
	}
}

/*
 * func_sall() - check the functionality of the SETALL command with semctl()
 */
static void func_sall(void)
{
	int i;
	unsigned short rarray[PSEMS];

	/*
	 * do a GETALL and compare the values to those set above
	 */

	if (semctl(sem_id_1, 0, GETALL, (union semun)rarray) == -1)
		tst_brkm(TBROK, cleanup, "semctl failed in func_sall");

	for (i = 0; i < PSEMS; i++) {
		if (array[i] != rarray[i]) {
			tst_resm(TFAIL, "semaphore values are not correct");
			return;
		}
	}

	tst_resm(TPASS, "semaphore values are correct");
}

/*
 * func_sval() - check the functionality of the SETVAL command with semctl()
 */
static void func_sval(void)
{
	int semv;
	union semun arr;

	/*
	 * do a GETVAL and compare it to the value set above
	 */

	semv = semctl(sem_id_1, SEM4, GETVAL, arr);
	if (semv == -1)
		tst_brkm(TBROK, cleanup, "semctl failed in func_sval");

	if (semv != INCVAL)
		tst_resm(TFAIL, "semaphore value is not what was set");
	else
		tst_resm(TPASS, "semaphore value is correct");
}

/*
 * func_rmid() - check the functionality of the IPC_RMID command with semctl()
 */
static void func_rmid(void)
{

	/*
	 * do a semop() - we should get EINVAL
	 */
	if (semop(sem_id_1, &sops, 1) != -1)
		tst_resm(TFAIL, "semop succeeded on expected fail");

	if (errno != EINVAL)
		tst_resm(TFAIL, "returned errno - %d - is not expected", errno);
	else
		tst_resm(TPASS, "semaphore appears to be removed");

	sem_id_1 = -1;
}

static void func_iinfo(int hidx)
{
	if (hidx >= 0) {
		sem_index = hidx;
		tst_resm(TPASS, "the highest index is correct");
	} else {
		sem_index = 0;
		tst_resm(TFAIL, "the highest index is incorrect");
	}
}

static void func_sinfo(void)
{
	if (ipc_buf.semusz < 1)
		tst_resm(TFAIL, "number of semaphore sets is incorrect");
	else
		tst_resm(TPASS, "number of semaphore sets is correct");
}

static void func_sstat(int semidx)
{
	if (semidx >= 0)
		tst_resm(TPASS, "id of the semaphore set is correct");
	else
		tst_resm(TFAIL, "id of the semaphore set is incorrect");
}

void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	TST_CHECKPOINT_INIT(tst_rmdir);

	/* get an IPC resource key */
	semkey = getipckey();

	/* create a semaphore set with read and alter permissions */
	sem_id_1 = semget(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA);
	if (sem_id_1 == -1)
		tst_brkm(TBROK, cleanup, "couldn't create semaphore in setup");
}

void cleanup(void)
{
	/* if it exists, remove the semaphore resource */
	rm_sema(sem_id_1);

	tst_rmdir();
}
