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
 *	setreuid04.c
 *
 * DESCRIPTION
 *	Test that root can change the real and effective uid to an
 *	unpriviledged user.
 *
 * ALGORITHM
 *
 *	Setup:
 *	  Setup signal handling
 *	  Get user information.
 *	  Pause for SIGUSER1 if option specified.
 *
 *	Setup test values.
 *	Loop if the proper options are given.
 *	For each test set execute the system call
 *	  Check that we received the expected result.
 *	  Verify that the uid and euid values are still correct.
 *	Cleanup:
 *	  Print errno log and/or timing stats if option given.
 *
 * USAGE:  <for command-line>
 *	 setreuid04 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	This test must be run by root.
 *	nobody must be a valid user.
 *	This test cannot be run with the i/I option.
 */

#include <errno.h>
#include <stdlib.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"
#include <sys/wait.h>

char *TCID = "setreuid04";

uid_t neg_one = -1;

/* flag to tell parent if child passed or failed. */
int flag = 0;

uid_t root_pw_uid, nobody_pw_uid;
char user1name[] = "nobody";
char rootname[] = "root";

struct passwd nobody, root;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	uid_t *real_uid;
	uid_t *eff_uid;
	struct passwd *exp_real_usr;
	struct passwd *exp_eff_usr;
	char *test_msg;
} test_data[] = {
	{
	&neg_one, &neg_one, &root, &root, "After setreuid(-1, nobody),"}, {
&nobody_pw_uid, &nobody_pw_uid, &nobody, &nobody,
		    "After setreuid(-1, -1),"},};

/*int TST_TOTAL = sizeof(test_data)/sizeof(test_data[0]);*/
int TST_TOTAL = 2;

void setup(void);
void cleanup(void);
void uid_verify(struct passwd *, struct passwd *, char *);

int main(int ac, char **av)
{
	int lc;
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	 }

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int i, pid, status;

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork failed");
		 } else if (pid == 0) {	/* child */

			for (i = 0; i < TST_TOTAL; i++) {

				/* Set the real or effective user id */
				TEST(setreuid(*test_data[i].real_uid,
					      *test_data[i].eff_uid));

				if (TEST_RETURN != -1) {
					tst_resm(TPASS, "setreuid(%d, %d) "
						 "succeeded as expected.",
						 *test_data[i].real_uid,
						 *test_data[i].eff_uid);
				} else {
					tst_resm(TFAIL, "setreuid(%d, %d) "
						 "did not return as expected.",
						 *test_data[i].real_uid,
						 *test_data[i].eff_uid);
					flag = -1;
				}

				/*
				 * Perform functional verification if test
				 * executed without (-f) option.
				 */
				if (STD_FUNCTIONAL_TEST) {
					uid_verify(test_data[i].exp_real_usr,
						   test_data[i].exp_eff_usr,
						   test_data[i].test_msg);
				} else {
					tst_resm(TINFO, "Call succeeded.");
				}
			}
			exit(flag);
		} else {	/* parent */
			waitpid(pid, &status, 0);
			if (WEXITSTATUS(status) != 0) {
				tst_resm(TFAIL, "test failed within "
					 "child process.");
			}
		}
	}
	cleanup();
	tst_exit();

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	if (getpwnam("nobody") == NULL) {
		tst_brkm(TBROK, NULL, "nobody must be a valid user.");
		tst_exit();
	 }

	/* Check that the test process id is root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be root for this test!");
		tst_exit();
	}

	root = *(getpwnam("root"));
	root_pw_uid = root.pw_uid;

	nobody = *(getpwnam("nobody"));
	nobody_pw_uid = nobody.pw_uid;

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

 }

void uid_verify(struct passwd *ru, struct passwd *eu, char *when)
{
	if ((getuid() != ru->pw_uid) || (geteuid() != eu->pw_uid)) {
		tst_resm(TINFO, "ERROR: %s real uid = %d; effective uid = %d",
			 when, getuid(), geteuid());
		tst_resm(TINFO, "Expected: real uid = %d; effective uid = %d",
			 ru->pw_uid, eu->pw_uid);
		flag = -1;
	}
}
