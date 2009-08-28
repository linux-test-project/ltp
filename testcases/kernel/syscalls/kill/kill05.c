/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	kill05.c
 *
 * DESCRIPTION
 *	Test case to check that kill() fails when passed a pid owned by another
 *	user.
 *
 * ALGORITHM
 *	call setup
 *	loop if the -i option was given
 *	setup a shared memory segment to for a flag which will notify
 *	ltpuser1's process that life is not worth living in a continuous loop.
 *	fork a child and set the euid to ltpuser1
 *	set the parents euid to ltpuser2
 *	execute the kill system call on ltpuser1's pid
 *	check the return value
 *	if return value is not -1
 *		issue a FAIL message, break remaining tests and cleanup
 *      if we are doing functional testing
 *              if the errno was set to 1 (Operation not permitted)
 *                      issue a PASS message
 *              otherwise
 *                      issue a FAIL message
 *	call cleanup
 *
 * USAGE
 *  kill05 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 *      26/02/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 *      - Fix wrong return value check on shmat system call (leading to
 *        segfault in case of error with this syscall).
 *      - Fix deletion of IPC memory segment. Segment was not correctly
 *        deleted due to the change of uid during the test.
 *
 * RESTRICTIONS
 *	This test must be run as root.
 *	Looping with the -i option does not work correctly.
 */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <malloc.h>
#include <pwd.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

extern void rm_shm(int);

void cleanup(void);
void setup(void);
void do_child(void);
void do_master_child(char **av);

char *TCID = "kill05";
int TST_TOTAL = 1;
int shmid1 = -1;
extern key_t semkey;
int *flag;

int exp_enos[] = { EPERM, 0 };

extern int Tst_count;
extern int getipckey();

#define TEST_SIG SIGKILL

int main(int ac, char **av)
{
	char *msg;		/* message returned from parse_opts */
	pid_t pid;
	int status;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();		/* global setup */

	pid = FORK_OR_VFORK();
	if (pid < 0)
		tst_brkm(TBROK, cleanup, "Fork failed");

	if (pid == 0) {
		do_master_child(av);
		return (0);
	} else {
		waitpid(pid, &status, 0);
	}

	cleanup();
	 /*NOTREACHED*/ return 0;
}

/*
 * do_master_child()
 */
void do_master_child(char **av)
{
	int lc;			/* loop counter */

	pid_t pid1;
	int status;

	char user1name[] = "nobody";
	char user2name[] = "bin";

	extern struct passwd *my_getpwnam(char *);

	struct passwd *ltpuser1, *ltpuser2;

	ltpuser1 = my_getpwnam(user1name);
	ltpuser2 = my_getpwnam(user2name);

	TEST_EXP_ENOS(exp_enos);

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		*flag = 0;

		/*
		 * Fork a process and set the euid so that it is
		 * different from this one.
		 */

		pid1 = FORK_OR_VFORK();

		if (pid1 < 0) {
			tst_brkm(TBROK, cleanup, "Fork failed");
		}

		if (pid1 == 0) {	/* child */
			if (setreuid(ltpuser1->pw_uid, ltpuser1->pw_uid) == -1) {
				tst_resm(TWARN, "setreuid failed in child");
			}
#ifdef UCLINUX
			if (self_exec(av[0], "") < 0) {
				tst_brkm(TBROK, cleanup,
					 "self_exec of child failed");
			}
#else
			do_child();
#endif
		} else {	/* parent */
			if (setreuid(ltpuser2->pw_uid, ltpuser2->pw_uid) == -1) {
				tst_resm(TWARN, "seteuid failed in child");
			}

			TEST(kill(pid1, TEST_SIG));

			/* signal the child that we're done */
			*flag = 1;

			waitpid(pid1, &status, 0);

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "%s failed - errno = "
					 "%d : %s Expected a return "
					 "value of -1 got %ld", TCID, TEST_ERRNO,
					 strerror(TEST_ERRNO), TEST_RETURN);

				continue;
			}
		}

		/*
		 * Check to see if the errno was set to the expected
		 * value of 1 : EPERM
		 */
		TEST_ERROR_LOG(TEST_ERRNO);

		if (TEST_ERRNO == EPERM) {
			tst_resm(TPASS, "errno set to %d : %s, as "
				 "expected", TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TFAIL, "errno set to %d : %s expected "
				 "%d : %s", TEST_ERRNO,
				 strerror(TEST_ERRNO), 1, strerror(1));
		}
	}
}

/*
 * do_child()
 */
void do_child()
{
	pid_t my_pid;

	my_pid = getpid();
	while (1) {
		if (*flag == 1) {
			exit(0);
		} else {
			sleep(1);
		}
	}
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* Check that the process is owned by root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, cleanup, "Test must be run as root");
	}

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Make a temp directory and cd to it.
	 * Usefull to be sure getipckey generated different IPC keys.
	 */
	tst_tmpdir();

	/* get an IPC resource key */
	semkey = getipckey();

	if ((shmid1 = shmget(semkey, (int)getpagesize(),
			     0666 | IPC_CREAT)) == -1) {
		tst_brkm(TBROK, cleanup, "Failed to setup shared memory");
	}

	/*flag = (int *)shmat(shmid1, 0, 0); */
	if ((flag = (int *)shmat(shmid1, 0, 0)) == (int *)-1) {
		tst_brkm(TBROK|TERRNO, cleanup,
			 "Failed to attatch shared memory:%d", shmid1);
	}
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	/* Remove the temporary directory */
	tst_rmdir();

	/*
	 * if it exists, remove the shared memory
	 */
	rm_shm(shmid1);

	/* exit with return code appropriate for results */
	tst_exit();
}
