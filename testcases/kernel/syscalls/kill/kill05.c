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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"

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

extern int getipckey();

#define TEST_SIG SIGKILL

int main(int ac, char **av)
{
	pid_t pid;
	int status;

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();		/* global setup */

	pid = FORK_OR_VFORK();
	if (pid == -1)
		tst_brkm(TBROK, cleanup, "Fork failed");
	else if (pid == 0)
		do_master_child(av);

	if (waitpid(pid, &status, 0) == -1)
		tst_resm(TBROK | TERRNO, "waitpid failed");
	else if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		tst_resm(TFAIL, "child exited abnormally");
	else
		tst_resm(TPASS, "received expected errno(EPERM)");
	cleanup();
	tst_exit();
}

void wait_for_flag(int value)
{
	while (1) {
		if (*flag == value)
			break;
		else
			sleep(1);
	}
}

/*
 * do_master_child()
 */
void do_master_child(char **av)
{
	pid_t pid1;
	int status;

	char user1name[] = "nobody";
	char user2name[] = "bin";

	struct passwd *ltpuser1, *ltpuser2;

	tst_count = 0;

	*flag = 0;

	pid1 = FORK_OR_VFORK();

	if (pid1 == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Fork failed");

	if (pid1 == 0) {
		ltpuser1 = SAFE_GETPWNAM(NULL, user1name);
		if (setreuid(ltpuser1->pw_uid, ltpuser1->pw_uid) == -1) {
			perror("setreuid failed (in child)");
			exit(1);
		}
		*flag = 1;
#ifdef UCLINUX
		if (self_exec(av[0], "") < 0) {
			perror("self_exec failed");
			exit(1);
		}
#else
		do_child();
#endif
	}
	ltpuser2 = SAFE_GETPWNAM(NULL, user2name);
	if (setreuid(ltpuser2->pw_uid, ltpuser2->pw_uid) == -1) {
		perror("seteuid failed");
		exit(1);
	}

	/* wait until child sets its euid */
	wait_for_flag(1);

	TEST(kill(pid1, TEST_SIG));

	/* signal the child that we're done */
	*flag = 2;

	if (waitpid(pid1, &status, 0) == -1) {
		perror("waitpid failed");
		exit(1);
	}

	if (TEST_RETURN != -1) {
		printf("kill succeeded unexpectedly\n");
		exit(1);
	}

	/*
	 * Check to see if the errno was set to the expected
	 * value of 1 : EPERM
	 */
	if (TEST_ERRNO == EPERM) {
		printf("kill failed with EPERM\n");
		exit(0);
	}
	perror("kill failed unexpectedly");
	exit(1);
}

void do_child(void)
{
	wait_for_flag(2);
	exit(0);
}

void setup(void)
{
	tst_require_root();

	TEST_PAUSE;

	tst_tmpdir();

	semkey = getipckey();

	if ((shmid1 = shmget(semkey, getpagesize(), 0666 | IPC_CREAT)) == -1)
		tst_brkm(TBROK, cleanup, "Failed to setup shared memory");

	if ((flag = shmat(shmid1, 0, 0)) == (int *)-1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Failed to attach shared memory:%d", shmid1);
}

void cleanup(void)
{
	rm_shm(shmid1);

	tst_rmdir();
}
