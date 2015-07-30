/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER	: sched_setparam05
 *
 *    EXECUTED BY	: root/superuser
 *
 *    TEST TITLE	: verify that sched_setparam() fails if the user does
 *			  not have proper privilages
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that sched_setparam() fails if the user does
 *	not have proper privilages
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 Fork a child
 *
 *	 CHILD:
 *	  Changes euid to "nobody" user.
 *	  Try to Change scheduling priority for parent
 *	  If call failed with errno = EPERM,
 *		Test passed
 *	  else
 *		Test failed
 *
 *	 PARENT:
 *		wait for child to finish
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  sched_setparam05 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *			where,  -c n : Run n copies concurrently.
 *				-e   : Turn on errno logging.
 *				-h   : Show help screen
 *				-f   : Turn off functional testing
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 ****************************************************************/

#include <errno.h>
#include <sched.h>
#include <pwd.h>
#include <sys/wait.h>
#include "test.h"

static void setup();
static void cleanup();

char *TCID = "sched_setparam05";
int TST_TOTAL = 1;

static struct sched_param param = { 0 };

static char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int main(int ac, char **av)
{

	int lc;
	int status;
	pid_t child_pid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		switch (child_pid = FORK_OR_VFORK()) {

		case -1:
			/* fork() failed */
			tst_resm(TFAIL, "fork() failed");
			continue;

		case 0:
			/* Child */

			/* Switch to nobody user */
			if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
				tst_brkm(TBROK, NULL, "\"nobody\" user"
					 "not present");
			}
			if (seteuid(ltpuser->pw_uid) == -1) {
				tst_resm(TWARN, "seteuid failed to "
					 "to set the effective uid to %d",
					 ltpuser->pw_uid);
				exit(1);
			}

			/*
			 * Call sched_setparam(2) with pid = getppid()
			 */
			TEST(sched_setparam(getppid(), &param));

			if ((TEST_RETURN == -1) && (TEST_ERRNO == EPERM)) {
				exit(0);
			}

			tst_resm(TWARN | TTERRNO,
				 "Test failed, sched_setparam()"
				 " returned : %ld", TEST_RETURN);
			exit(1);

		default:
			/* Parent */
			if ((waitpid(child_pid, &status, 0)) < 0) {
				tst_resm(TFAIL, "wait() failed");
				continue;
			}
			if ((WIFEXITED(status)) && (WEXITSTATUS(status) == 0)) {
				tst_resm(TPASS, "Test passed, Got EPERM");
			} else {
				tst_resm(TFAIL, "Test Failed");
			}
		}
	}

	cleanup();
	tst_exit();
}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{

	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{
}
