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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
 *	  	if the conditions are correct,
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

#include "ipcshm.h"
#include "system_specific_hugepages_info.h"

char *TCID = "hugeshmctl01";
int TST_TOTAL = 4;
unsigned long huge_pages_shm_to_be_allocated;

int shm_id_1 = -1;
struct shmid_ds buf;
long save_time;

#define FIRST	0
#define SECOND	1
int stat_time;		/* set to either FIRST or SECOND for IPC_STAT tests */

void *set_shared;

#define N_ATTACH	4

pid_t pid_arr[N_ATTACH];

void sighandler(int);

/*
 * These are the various setup and check functions for the commands
 * that we are checking.
 */

/* Setup, cleanup and check routines for IPC_STAT */
void stat_setup(void), func_stat(void);
void stat_cleanup(void);

/* Setup and check routines for IPC_SET */
void set_setup(void), func_set(void);

/* Check routine for IPC_RMID */
void func_rmid(void);

struct test_case_t {
	int cmd;		/* the command to test */
	void (*func_test)();	/* the test function */
	void (*func_setup)();	/* the setup function if necessary */
} TC[] = {

	{IPC_STAT, func_stat, stat_setup},

	{IPC_STAT, func_stat, stat_setup},

	{IPC_SET, func_set, set_setup},

	{IPC_RMID, func_rmid, NULL}
};

#define NEWMODE	0066

int main(int ac, char **av)
{
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */
	int i;
	void check_functionality(void);

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

        if (get_no_of_hugepages() <= 0 || hugepages_size() <= 0)
             tst_brkm(TCONF, NULL, "Not enough available Hugepages");
        else
             huge_pages_shm_to_be_allocated = ( get_no_of_hugepages() * hugepages_size() * 1024) / 2 ;

	setup();			/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* initialize stat_time */
		stat_time = FIRST;

		/*
		 * Create a shared memory segment with read and write
		 * permissions.  Do this here instead of in setup()
		 * so that looping (-i) will work correctly.
		 */
		if ((shm_id_1 = shmget(shmkey, huge_pages_shm_to_be_allocated, SHM_HUGETLB | IPC_CREAT | IPC_EXCL |
				SHM_RW)) == -1) {
			tst_brkm(TBROK, cleanup, "couldn't create the shared"
				 " memory segment");
		}

		/* loop through the test cases */
		for (i=0; i<TST_TOTAL; i++) {

			/*
			 * if needed, set up any required conditions by
			 * calling the appropriate setup function
			 */
			if (TC[i].func_setup != NULL) {
				(*TC[i].func_setup)();
			}

			/*
			 * Use TEST macro to make the call
			 */

			TEST(shmctl(shm_id_1, TC[i].cmd, &buf));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "%s call failed - errno "
					 "= %d : %s", TCID, TEST_ERRNO,
					 strerror(TEST_ERRNO));
				continue;
			}
			if (STD_FUNCTIONAL_TEST) {
				(*TC[i].func_test)();
			} else {
				tst_resm(TPASS, "call succeeded");

				/* now perform command related cleanup */
				switch(TC[i].cmd) {
				case IPC_STAT:
					stat_cleanup();
					break;
				case IPC_RMID:
					shm_id_1 = -1;
					break;
				}
			}
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
void *
set_shmat()
{
	void *rval;

	/* attach the shared memory */
	rval = shmat(shm_id_1, 0, 0);

	if (rval == (void *)-1)
		tst_brkm(TBROK|TERRNO, cleanup, "shmat() failed.");

	return rval;
}

/*
 * stat_setup() - Set up for the IPC_STAT command with shmctl().
 *		  Make things interesting by forking some children
 *		  that will either attach or inherit the shared memory.
 */
void
stat_setup()
{
	int i, rval;
	void *test, *set_shmat();
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
		tst_brkm(TBROK, cleanup, "block SIGUSR1 error");

	for (i=0; i<N_ATTACH; i++) {
		if ((pid = fork()) == -1) {
			tst_brkm(TBROK, cleanup, "could not fork");
		}

		if (pid == 0) {		/* child */

			if (stat_time == FIRST) {
				test = set_shmat();
			} else {
				test = set_shared;
			}

			/* do an assignement for fun */
			*(int *)test = i;

			/*
			 * sigsuspend until we get a signal from stat_cleanup()
			 * use sigsuspend instead of pause to avoid children
			 * infinite sleep without getting SIGUSR1 from parent
			 */
			rval = sigsuspend(&oldmask);
			if (rval != -1)
				tst_brkm(TBROK, cleanup, "sigsuspend error");

			/* don't have to block SIGUSR1 any more, recover the mask */
			if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
				tst_brkm(TBROK, cleanup, "children setmask error");

			/* now we're back - detach the memory and exit */
			if (shmdt(test) == -1) {
				tst_resm(TBROK, "shmdt() failed - %d", errno);
			}

			tst_exit();

		} else {		/* parent */
			/* save the child's pid for cleanup later */
			pid_arr[i] = pid;
		}
	}

	/* parent doesn't have to block SIGUSR1, recover the mask */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		tst_brkm(TBROK, cleanup, "parent setmask error");

	/* sleep briefly to ensure correct execution order */
	usleep(250000);
}

/*
 * func_stat() - check the functionality of the IPC_STAT command with shmctl()
 *		 by looking at the pid of the creator, the segement size,
 *		 the number of attaches and the mode.
 */
void
func_stat()
{
	int fail = 0;
	pid_t pid;

	/* check perm, pid, nattach and size */

	pid = getpid();

	if (buf.shm_cpid != pid) {
		tst_resm(TFAIL, "creator pid is incorrect");
		fail = 1;
	}

	if (!fail && buf.shm_segsz != huge_pages_shm_to_be_allocated) {
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
		tst_resm(TFAIL, "# of attaches is incorrect - %lu",
			 (unsigned long)buf.shm_nattch);
		fail = 1;
	}

	/* use MODE_MASK to make sure we are comparing the last 9 bits */
	if (!fail && (buf.shm_perm.mode & MODE_MASK) != ((SHM_RW) & MODE_MASK)) {
		tst_resm(TFAIL, "segment mode is incorrect");
		fail = 1;
	}

	stat_cleanup();

	/* save the change time for use in the next test */
	save_time = buf.shm_ctime;

	if (fail) {
		return;
	}

	tst_resm(TPASS, "pid, size, # of attaches and mode are correct "
		 "- pass #%d", stat_time);
}

/*
 * stat_cleanup() - signal the children to clean up after themselves and
 *		    have the parent make dessert, er, um, make that remove
 *		    the shared memory that is no longer needed.
 */
void
stat_cleanup()
{
	int i;

	/* wake up the childern so they can detach the memory and exit */
	for (i=0; i<N_ATTACH; i++) {
		if (kill(pid_arr[i], SIGUSR1) == -1) {
			tst_brkm(TBROK, cleanup, "kill failed");
		}
	}

	/* remove the parent's shared memory the second time through */
	if (stat_time == SECOND) {
		if (shmdt(set_shared) == -1) {
			tst_resm(TINFO, "shmdt() failed");
		}
	}

	stat_time++;
}

/*
 * set_setup() - set up for the IPC_SET command with shmctl()
 */
void
set_setup()
{
	/* set up a new mode for the shared memory segment */
	buf.shm_perm.mode = SHM_RW | NEWMODE;

	/* sleep for one second to get a different shm_ctime value */
	sleep(1);
}

/*
 * func_set() - check the functionality of the IPC_SET command with shmctl()
 */
void
func_set()
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

	if (fail) {
		return;
	}

	tst_resm(TPASS, "new mode and change time are correct");
}

/*
 * func_rmid() - check the functionality of the IPC_RMID command with shmctl()
 */
void
func_rmid()
{
	/* Do another shmctl() - we should get EINVAL */
	if (shmctl(shm_id_1, IPC_STAT, &buf) != -1) {
		tst_brkm(TBROK, cleanup, "shmctl succeeded on expected fail");
	}

	if (errno != EINVAL) {
		tst_resm(TFAIL, "returned unexpected errno %d", errno);
	} else {
		tst_resm(TPASS, "shared memory appears to be removed");
	}

	shm_id_1 = -1;
}

/*
 * sighandler() - handle signals, in this case SIGUSR1 is the only one expected
 */
void
sighandler(sig)
{
	if (sig != SIGUSR1)
		tst_resm(TINFO, "received unexpected signal %d", sig);
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void
setup(void)
{

	tst_sig(FORK, sighandler, cleanup);

	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	/* get an IPC resource key */
	shmkey = getipckey();
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void
cleanup(void)
{
	/* if it exists, remove the shared memory segment */
	rm_shm(shm_id_1);

	tst_rmdir();

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}
