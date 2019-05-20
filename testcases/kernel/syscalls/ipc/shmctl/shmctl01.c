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
 *	shmctl01.c
 *
 * DESCRIPTION
 *	shmctl01 - test the IPC_STAT, IPC_SET and IPC_RMID commands as
 *		   they are used with shmctl()
 *
 * ALGORITHM
 *	loop if that option was specified
 *	create a shared memory segment with read and write permission
 *	set up any test case specific conditions
 *	call shmctl() using the TEST macro
 *	check the return code
 *	  if failure, issue a FAIL message.
 *	otherwise,
 *	  if doing functionality testing
 *		call the correct test function
 *		if the conditions are correct,
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	  otherwise
 *	    issue a PASS message
 *	call cleanup
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "ipcshm.h"
#include "safe_macros.h"

char *TCID = "shmctl01";

static int shm_id_1 = -1;
static int shm_index;
static struct shmid_ds buf;
static struct shminfo info;
static long save_time;

#define FIRST	0
#define SECOND	1
static int stat_time;

static void *set_shared;

#define N_ATTACH	4

static pid_t pid_arr[N_ATTACH];

/* Setup, cleanup and check routines for IPC_STAT */
static void stat_setup(void), func_istat(int ret);
static void stat_cleanup(void);

/* Setup and check routines for IPC_SET */
static void set_setup(void), func_set(int ret);

/* Check routine for IPC_INFO */
static void func_info(int ret);

/* Check routine for SHM_STAT */
static void func_sstat(int ret);
static void func_sstat_setup(void);

/* Check routine for SHM_LOCK */
static void func_lock(int ret);

/* Check routine for SHM_UNLOCK */
static void func_unlock(int ret);

/* Check routine for IPC_RMID */
static void func_rmid(int ret);

/* Child function */
static void do_child(void);

static struct test_case_t {
	int *shmid;
	int cmd;
	struct shmid_ds *arg;
	void (*func_test) (int);
	void (*func_setup) (void);
} TC[] = {
	{&shm_id_1, IPC_STAT, &buf, func_istat, stat_setup},
#ifndef UCLINUX
	    /*
	     * The second test is not applicable to uClinux;
	     * shared memory segments are detached on exec(),
	     * so cannot be passed to uClinux children.
	     */
	{&shm_id_1, IPC_STAT, &buf, func_istat, stat_setup},
#endif
	{&shm_id_1, IPC_SET, &buf, func_set, set_setup},
	{&shm_id_1, IPC_INFO, (struct shmid_ds *) &info, func_info, NULL},
	{&shm_index, SHM_STAT, &buf, func_sstat, func_sstat_setup},
	{&shm_id_1, SHM_LOCK, NULL, func_lock, NULL},
	{&shm_id_1, SHM_UNLOCK, NULL, func_unlock, NULL},
	{&shm_id_1, IPC_RMID, NULL, func_rmid, NULL},
};

static int TST_TOTAL = ARRAY_SIZE(TC);

#define NEWMODE	0066

#ifdef UCLINUX
#define PIPE_NAME	"shmctl01"
static char *argv0;
#endif

static int stat_i;

int main(int argc, char *argv[])
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);
#ifdef UCLINUX
	argv0 = argv[0];
	maybe_run_child(do_child, "ddd", &stat_i, &stat_time, &shm_id_1);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		stat_time = FIRST;

		/*
		 * Create a shared memory segment with read and write
		 * permissions.  Do this here instead of in setup()
		 * so that looping (-i) will work correctly.
		 */
		shm_id_1 = shmget(shmkey, SHM_SIZE,
				  IPC_CREAT | IPC_EXCL | SHM_RW);
		if (shm_id_1 == -1)
			tst_brkm(TBROK, cleanup, "couldn't create the shared"
				 " memory segment");

		for (i = 0; i < TST_TOTAL; i++) {

			/*
			 * if needed, set up any required conditions by
			 * calling the appropriate setup function
			 */
			if (TC[i].func_setup != NULL)
				(*TC[i].func_setup) ();

			TEST(shmctl(*(TC[i].shmid), TC[i].cmd, TC[i].arg));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "%s call failed - errno "
					 "= %d : %s", TCID, TEST_ERRNO,
					 strerror(TEST_ERRNO));
				continue;
			}
			(*TC[i].func_test) (TEST_RETURN);
		}
	}

	cleanup();
	tst_exit();
}

/*
 * set_shmat() - Attach the shared memory and return the pointer.  Use
 *		 this seperate routine to avoid code duplication in
 *		 stat_setup() below.
 */
void *set_shmat(void)
{
	void *rval;

	/* attach the shared memory */
	rval = shmat(shm_id_1, 0, 0);

	/*
	 * if shmat() fails, the only thing we can do is
	 * print a message to that effect.
	 */
	if (rval == (void *)-1) {
		tst_resm(TBROK, "shmat() failed - %s", strerror(errno));
		cleanup();
	}

	return rval;
}

/*
 * stat_setup() - Set up for the IPC_STAT command with shmctl().
 *		  Make things interesting by forking some children
 *		  that will either attach or inherit the shared memory.
 */
void stat_setup(void)
{
	void *set_shmat();
	pid_t pid;

	/*
	 * The first time through, let the children attach the memory.
	 * The second time through, attach the memory first and let
	 * the children inherit the memory.
	 */

	if (stat_time == SECOND)
		/*
		 * use the global "set_shared" variable here so that
		 * it can be removed in the stat_func() routine.
		 */
		set_shared = set_shmat();

	tst_old_flush();
	for (stat_i = 0; stat_i < N_ATTACH; stat_i++) {
		pid = FORK_OR_VFORK();
		if (pid == -1)
			tst_brkm(TBROK, cleanup, "could not fork");

		if (pid == 0) {
#ifdef UCLINUX
			if (self_exec(argv0, "ddd", stat_i, stat_time,
				      shm_id_1) < 0)
				tst_brkm(TBROK, cleanup, "could not self_exec");
#else
			do_child();
#endif

		} else {
			/* save the child's pid for cleanup later */
			pid_arr[stat_i] = pid;
			TST_PROCESS_STATE_WAIT(cleanup, pid, 'S');
		}
	}
}

void do_child(void)
{
	void *test;

	if (stat_time == FIRST)
		test = set_shmat();
	else
		test = set_shared;

	memcpy(test, &stat_i, sizeof(stat_i));

	/* pause until we get a signal from stat_cleanup() */
	pause();

	/* now we're back - detach the memory and exit */
	if (shmdt(test) == -1)
		tst_resm(TBROK, "shmdt() failed - %d", errno);

	tst_exit();
}

/*
 * func_istat() - check the functionality of the IPC_STAT command with shmctl()
 *		 by looking at the pid of the creator, the segement size,
 *		 the number of attaches and the mode.
 */
void func_istat(int ret)
{
	int fail = 0;
	pid_t pid;

	/* check perm, pid, nattach and size */

	pid = getpid();

	if (buf.shm_cpid != pid) {
		tst_resm(TFAIL, "creator pid is incorrect");
		fail = 1;
	}

	if (!fail && buf.shm_segsz != SHM_SIZE) {
		tst_resm(TFAIL, "segment size is incorrect");
		fail = 1;
	}

	/*
	 * The first time through, only the children attach the memory, so
	 * the attaches equal N_ATTACH + stat_time (0).  The second time
	 * through, the parent attaches the memory and the children inherit
	 * that memory so the attaches equal N_ATTACH + stat_time (1).
	 */
	if (!fail && buf.shm_nattch != N_ATTACH + stat_time) {
		tst_resm(TFAIL, "# of attaches is incorrect - %ld",
			 buf.shm_nattch);
		fail = 1;
	}

	/* use MODE_MASK to make sure we are comparing the last 9 bits */
	if (!fail && (buf.shm_perm.mode & MODE_MASK) !=
			((SHM_RW) & MODE_MASK)) {
		tst_resm(TFAIL, "segment mode is incorrect");
		fail = 1;
	}

	stat_cleanup();

	/* save the change time for use in the next test */
	save_time = buf.shm_ctime;

	if (fail)
		return;

	tst_resm(TPASS, "pid, size, # of attaches and mode are correct "
		 "- pass #%d", stat_time);
}

/*
 * stat_cleanup() - signal the children to clean up after themselves and
 *		    have the parent make dessert, er, um, make that remove
 *		    the shared memory that is no longer needed.
 */
void stat_cleanup(void)
{
	int i;

	/* wake up the childern so they can detach the memory and exit */
	for (i = 0; i < N_ATTACH; i++) {
		SAFE_KILL(cleanup, pid_arr[i], SIGUSR1);
	}

	/* remove the parent's shared memory the second time through */
	if (stat_time == SECOND) {
		if (shmdt(set_shared) == -1)
			tst_resm(TINFO, "shmdt() failed");
	}

	for (i = 0; i < N_ATTACH; i++) {
		SAFE_WAITPID(cleanup, pid_arr[i], NULL, 0);
	}

	stat_time++;
}

/*
 * set_setup() - set up for the IPC_SET command with shmctl()
 */
void set_setup(void)
{
	/* set up a new mode for the shared memory segment */
	buf.shm_perm.mode = SHM_RW | NEWMODE;

	/* sleep for one second to get a different shm_ctime value */
	sleep(1);
}

/*
 * func_set() - check the functionality of the IPC_SET command with shmctl()
 */
void func_set(int ret)
{
	int fail = 0;

	/* first stat the shared memory to get the new data */
	if (shmctl(shm_id_1, IPC_STAT, &buf) == -1) {
		tst_resm(TBROK, "stat failed in func_set()");
		return;
	}

	if ((buf.shm_perm.mode & MODE_MASK) !=
			((SHM_RW | NEWMODE) & MODE_MASK)) {
		tst_resm(TFAIL, "new mode is incorrect");
		fail = 1;
	}

	if (!fail && save_time >= buf.shm_ctime) {
		tst_resm(TFAIL, "change time is incorrect");
		fail = 1;
	}

	if (fail)
		return;

	tst_resm(TPASS, "new mode and change time are correct");
}

static void func_info(int ret)
{
	if (info.shmmin != 1)
		tst_resm(TFAIL, "value of shmmin is incorrect");
	else
		tst_resm(TPASS, "get correct shared memory limits");
}

static void func_sstat(int ret)
{
	if (ret >= 0)
		tst_resm(TPASS, "get correct shared memory id for index: %d",
			shm_index);
	else
		tst_resm(TFAIL, "shared memory id is incorrect, index: %d",
			shm_index);
}

static void func_sstat_setup(void)
{
	struct shm_info tmp;
	int ret;

	ret = shmctl(shm_id_1, SHM_INFO, (void *)&tmp);
	if (ret < 0)
		tst_resm(TFAIL|TERRNO, "shmctl(SHM_INFO)");
	else
		shm_index = ret;
}

static void func_lock(int ret)
{
	if (shmctl(shm_id_1, IPC_STAT, &buf) == -1) {
		tst_resm(TBROK, "stat failed in func_lock()");
		return;
	}

	if (buf.shm_perm.mode & SHM_LOCKED)
		tst_resm(TPASS, "SHM_LOCK is set");
	else
		tst_resm(TFAIL, "SHM_LOCK is cleared");
}

static void func_unlock(int ret)
{
	if (shmctl(shm_id_1, IPC_STAT, &buf) == -1) {
		tst_resm(TBROK, "stat failed in func_unlock()");
		return;
	}

	if (buf.shm_perm.mode & SHM_LOCKED)
		tst_resm(TFAIL, "SHM_LOCK is set");
	else
		tst_resm(TPASS, "SHM_LOCK is cleared");
}


/*
 * func_rmid() - check the functionality of the IPC_RMID command with shmctl()
 */
void func_rmid(int ret)
{
	/* Do another shmctl() - we should get EINVAL */
	if (shmctl(shm_id_1, IPC_STAT, &buf) != -1)
		tst_brkm(TBROK, cleanup, "shmctl succeeded on expected fail");

	if (errno != EINVAL)
		tst_resm(TFAIL, "returned unexpected errno %d", errno);
	else
		tst_resm(TPASS, "shared memory appears to be removed");

	shm_id_1 = -1;
}

/*
 * sighandler() - handle signals, in this case SIGUSR1 is the only one expected
 */
void sighandler(int sig)
{
	if (sig != SIGUSR1)
		tst_resm(TBROK, "received unexpected signal %d", sig);
}

void setup(void)
{
	tst_sig(FORK, sighandler, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	shmkey = getipckey();
}

void cleanup(void)
{
	rm_shm(shm_id_1);

	tst_rmdir();
}
