/*
 *
 *   Copyright (c) International Business Machines  Corp., 2004
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	hugeshmctl01.c
 *
 * DESCRIPTION
 *	hugeshmctl01 - test the IPC_STAT, IPC_SET and IPC_RMID commands as
 *		   they are used with shmctl()
 *
 * ALGORITHM
 *	loop if that option was specified
 *	create a large shared memory segment with read and write permission
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
 *
 * USAGE:  <for command-line>
 *  hugeshmctl01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated by Robbie Williamson
 *
 * RESTRICTIONS
 *	none
 */

#include "test.h"
#include "safe_macros.h"
#include "mem.h"
#include "hugetlb.h"

char *TCID = "hugeshmctl01";
int TST_TOTAL = 4;

#define FIRST		0
#define SECOND		1
#define N_ATTACH	4
#define NEWMODE		0066

static size_t shm_size;
static int shm_id_1 = -1;
static struct shmid_ds buf;
static time_t save_time;
static int stat_time;
static void *set_shared;
static pid_t pid_arr[N_ATTACH];

static void sighandler(int sig);
static void stat_setup(void);
static void stat_cleanup(void);
static void set_setup(void);
static void func_stat(void);
static void func_set(void);
static void func_rmid(void);
static void *set_shmat(void);

static long hugepages = 128;
static option_t options[] = {
	{"s:", &sflag, &nr_opt},
	{NULL, NULL, NULL}
};

struct test_case_t {
	int cmd;
	void (*func_test) (void);
	void (*func_setup) (void);
} TC[] = {
	{
	IPC_STAT, func_stat, stat_setup}, {
	IPC_STAT, func_stat, stat_setup}, {
	IPC_SET, func_set, set_setup}, {
	IPC_RMID, func_rmid, NULL}
};

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, options, NULL);

	if (sflag)
		hugepages = SAFE_STRTOL(NULL, nr_opt, 0, LONG_MAX);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		/* initialize stat_time */
		stat_time = FIRST;

		/*
		 * Create a shared memory segment with read and write
		 * permissions.  Do this here instead of in setup()
		 * so that looping (-i) will work correctly.
		 */
		shm_id_1 = shmget(shmkey, shm_size,
				  SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW);
		if (shm_id_1 == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "shmget #main");

		for (i = 0; i < TST_TOTAL; i++) {
			/*
			 * if needed, set up any required conditions by
			 * calling the appropriate setup function
			 */
			if (TC[i].func_setup != NULL)
				(*TC[i].func_setup) ();

			if (shmctl(shm_id_1, TC[i].cmd, &buf) == -1) {
				tst_resm(TFAIL | TERRNO, "shmctl #main");
				continue;
			}
			(*TC[i].func_test) ();
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

	rval = shmat(shm_id_1, 0, 0);
	if (rval == (void *)-1)
		tst_brkm(TBROK | TERRNO, cleanup, "set shmat");

	return rval;
}

/*
 * stat_setup() - Set up for the IPC_STAT command with shmctl().
 *		  Make things interesting by forking some children
 *		  that will either attach or inherit the shared memory.
 */
static void stat_setup(void)
{
	int i, rval;
	void *test;
	pid_t pid;
	sigset_t newmask, oldmask;

	/*
	 * The first time through, let the children attach the memory.
	 * The second time through, attach the memory first and let
	 * the children inherit the memory.
	 */

	if (stat_time == SECOND) {
		/*
		 * use the global "set_shared" variable here so that
		 * it can be removed in the stat_func() routine.
		 */
		set_shared = set_shmat();
	}

	/*
	 * Block SIGUSR1 before children pause for a signal
	 * Doing so to avoid the risk that the parent cleans up
	 * children by calling stat_cleanup() before children call
	 * call pause() so that children sleep forever(this is a
	 * side effect of the arbitrary usleep time below).
	 * In FIRST, children call shmat. If children sleep forever,
	 * those attached shm can't be released so some other shm
	 * tests will fail a lot.
	 */
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "block SIGUSR1 error");

	for (i = 0; i < N_ATTACH; i++) {
		switch (pid = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork");
		case 0:
			test = (stat_time == FIRST) ? set_shmat() : set_shared;

			/* do an assignement for fun */
			*(int *)test = i;

			/*
			 * sigsuspend until we get a signal from stat_cleanup()
			 * use sigsuspend instead of pause to avoid children
			 * infinite sleep without getting SIGUSR1 from parent
			 */
			rval = sigsuspend(&oldmask);
			if (rval != -1)
				tst_brkm(TBROK | TERRNO, cleanup, "sigsuspend");

			/*
			 * don't have to block SIGUSR1 any more,
			 * recover the mask
			 */
			if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
				tst_brkm(TBROK | TERRNO, cleanup,
					 "child sigprocmask");

			/* now we're back - detach the memory and exit */
			if (shmdt(test) == -1)
				tst_brkm(TBROK | TERRNO, cleanup,
					 "shmdt in stat_setup()");
			exit(0);
		default:
			/* save the child's pid for cleanup later */
			pid_arr[i] = pid;
		}
	}

	/* parent doesn't have to block SIGUSR1, recover the mask */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		tst_brkm(TBROK, cleanup, "parent sigprocmask");

	usleep(250000);
}

/*
 * func_stat() - check the functionality of the IPC_STAT command with shmctl()
 *		 by looking at the pid of the creator, the segement size,
 *		 the number of attaches and the mode.
 */
static void func_stat(void)
{
	pid_t pid;

	/* check perm, pid, nattach and size */
	pid = getpid();

	if (buf.shm_cpid != pid) {
		tst_resm(TFAIL, "creator pid is incorrect");
		goto fail;
	}

	if (buf.shm_segsz != shm_size) {
		tst_resm(TFAIL, "segment size is incorrect");
		goto fail;
	}

	/*
	 * The first time through, only the children attach the memory, so
	 * the attaches equal N_ATTACH + stat_time (0).  The second time
	 * through, the parent attaches the memory and the children inherit
	 * that memory so the attaches equal N_ATTACH + stat_time (1).
	 */
	if (buf.shm_nattch != N_ATTACH + stat_time) {
		tst_resm(TFAIL, "# of attaches is incorrect - %lu",
			 (unsigned long)buf.shm_nattch);
		goto fail;
	}

	/* use MODE_MASK to make sure we are comparing the last 9 bits */
	if ((buf.shm_perm.mode & MODE_MASK) != ((SHM_RW) & MODE_MASK)) {
		tst_resm(TFAIL, "segment mode is incorrect");
		goto fail;
	}

	tst_resm(TPASS, "pid, size, # of attaches and mode are correct "
		 "- pass #%d", stat_time);

fail:
	stat_cleanup();

	/* save the change time for use in the next test */
	save_time = buf.shm_ctime;
}

/*
 * stat_cleanup() - signal the children to clean up after themselves and
 *		    have the parent make dessert, er, um, make that remove
 *		    the shared memory that is no longer needed.
 */
static void stat_cleanup(void)
{
	int i;

	/* wake up the childern so they can detach the memory and exit */
	for (i = 0; i < N_ATTACH; i++)
		if (kill(pid_arr[i], SIGUSR1) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "kill with SIGUSR1");

	/* remove the parent's shared memory the second time through */
	if (stat_time == SECOND)
		if (shmdt(set_shared) == -1)
			tst_resm(TBROK | TERRNO, "shmdt in stat_cleanup()");
	stat_time++;
}

/*
 * set_setup() - set up for the IPC_SET command with shmctl()
 */
static void set_setup(void)
{
	/* set up a new mode for the shared memory segment */
	buf.shm_perm.mode = SHM_RW | NEWMODE;

	/* sleep for one second to get a different shm_ctime value */
	sleep(1);
}

/*
 * func_set() - check the functionality of the IPC_SET command with shmctl()
 */
static void func_set(void)
{
	/* first stat the shared memory to get the new data */
	if (shmctl(shm_id_1, IPC_STAT, &buf) == -1) {
		tst_resm(TBROK | TERRNO, "shmctl in func_set()");
		return;
	}

	if ((buf.shm_perm.mode & MODE_MASK) != ((SHM_RW | NEWMODE) & MODE_MASK)) {
		tst_resm(TFAIL, "new mode is incorrect");
		return;
	}

	if (save_time >= buf.shm_ctime) {
		tst_resm(TFAIL, "change time is incorrect");
		return;
	}

	tst_resm(TPASS, "new mode and change time are correct");
}

/*
 * func_rmid() - check the functionality of the IPC_RMID command with shmctl()
 */
static void func_rmid(void)
{
	/* Do another shmctl() - we should get EINVAL */
	if (shmctl(shm_id_1, IPC_STAT, &buf) != -1)
		tst_brkm(TBROK, cleanup, "shmctl in func_rmid() "
			 "succeeded unexpectedly");
	if (errno != EINVAL)
		tst_resm(TFAIL | TERRNO, "shmctl in func_rmid() failed "
			 "unexpectedly - expect errno=EINVAL, got");
	else
		tst_resm(TPASS, "shmctl in func_rmid() failed as expected, "
			 "shared memory appears to be removed");
	shm_id_1 = -1;
}

static void sighandler(int sig)
{
	if (sig != SIGUSR1)
		tst_resm(TFAIL, "received unexpected signal %d", sig);
}

void setup(void)
{
	long hpage_size;

	tst_require_root();
	check_hugepage();
	tst_sig(FORK, sighandler, cleanup);
	tst_tmpdir();

	orig_hugepages = get_sys_tune("nr_hugepages");
	set_sys_tune("nr_hugepages", hugepages, 1);
	hpage_size = read_meminfo("Hugepagesize:") * 1024;

	shm_size = hpage_size * hugepages / 2;
	update_shm_size(&shm_size);
	shmkey = getipckey(cleanup);

	TEST_PAUSE;
}

void cleanup(void)
{
	rm_shm(shm_id_1);

	set_sys_tune("nr_hugepages", orig_hugepages, 0);

	tst_rmdir();
}
