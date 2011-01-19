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
 *	setreuid03.c
 *
 * DESCRIPTION
 *	Test setreuid() when executed by an unpriviledged user.
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
 *	  If setreuid failed as expected
 *		check that the correct errno value was set.
 *	  otherwise
 *		Issue Pass message.
 *	  Verify that the uid and euid values are still correct.
 *	Cleanup:
 *	  Print errno log and/or timing stats if option given.
 *
 * USAGE:  <for command-line>
 *	setreuid03 [-c n] [-f] [-e] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-f   : Turn off functionality Testing.
 *		-e   : Turn on errno logging.
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
 *	This test must be run by nobody.
 */

#include <pwd.h>
#include <stdlib.h>
#include "test.h"
#include "usctest.h"
#include <errno.h>

#define FAILED  1

char *TCID = "setreuid03";

int fail = -1;
int pass = 0;
uid_t neg_one = -1;
int exp_enos[] = { EPERM, 0 };

uid_t root_pw_uid, nobody_pw_uid, bin_pw_uid;
char user1name[] = "nobody";
char user2name[] = "bin";
char rootname[] = "root";

struct passwd nobody, bin, root;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	uid_t *real_uid;
	uid_t *eff_uid;
	int *exp_ret;
	struct passwd *exp_real_usr;
	struct passwd *exp_eff_usr;
	char *test_msg;
} test_data[] = {
	{
	&nobody_pw_uid, &nobody_pw_uid, &pass, &nobody, &nobody,
		    "After setreuid(nobody, nobody),"}, {
	&neg_one, &nobody_pw_uid, &pass, &nobody, &nobody,
		    "After setreuid(-1, nobody),"}, {
	&nobody_pw_uid, &neg_one, &pass, &nobody, &nobody,
		    "After setreuid(nobody, -1),"}, {
	&neg_one, &neg_one, &pass, &nobody, &nobody, "After setreuid(-1, -1),"},
	{
	&neg_one, &root_pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(-1, root),"}, {
	&root_pw_uid, &neg_one, &fail, &nobody, &nobody,
		    "After setreuid(root, -1),"}, {
	&root_pw_uid, &root_pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(root, root),"}, {
	&root_pw_uid, &nobody_pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(root, nobody),"}, {
	&root_pw_uid, &bin_pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(root, nobody),"}, {
	&bin_pw_uid, &root_pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(bin, root),"}, {
	&bin_pw_uid, &neg_one, &fail, &nobody, &nobody,
		    "After setreuid(bin, -1),"}, {
	&bin_pw_uid, &bin_pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(bin, bin,),"}, {
	&bin_pw_uid, &nobody_pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(bin, nobody),"}, {
&nobody_pw_uid, &bin_pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(nobody, bin),"},};

int TST_TOTAL = sizeof(test_data) / sizeof(test_data[0]);

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
		int i;

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			/* Set the real or effective user id */
			TEST(setreuid(*test_data[i].real_uid,
				      *test_data[i].eff_uid));

			if (TEST_RETURN == *test_data[i].exp_ret) {
				if (TEST_RETURN == neg_one) {
					if (TEST_ERRNO != EPERM) {
						tst_resm(TFAIL,
							 "setreuid(%d, %d) "
							 "did not set errno "
							 "value as expected.",
							 *test_data[i].real_uid,
							 *test_data[i].eff_uid);
						continue;
					}
					tst_resm(TPASS, "setreuid(%d, %d) "
						 "failed as expected.",
						 *test_data[i].real_uid,
						 *test_data[i].eff_uid);
				} else {
					tst_resm(TPASS, "setreuid(%d, %d) "
						 "succeeded as expected.",
						 *test_data[i].real_uid,
						 *test_data[i].eff_uid);
				}
			} else {
				tst_resm(TFAIL, "setreuid(%d, %d) "
					 "did not return as expected.",
					 *test_data[i].real_uid,
					 *test_data[i].eff_uid);
			}

			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
			}
			/*
			 * Perform functional verification if test
			 * executed without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				uid_verify(test_data[i].exp_real_usr,
					   test_data[i].exp_eff_usr,
					   test_data[i].test_msg);
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

	if (getpwnam("bin") == NULL) {
		tst_brkm(TBROK, NULL, "bin must be a valid user.");
		tst_exit();
	 }

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	root = *(getpwnam("root"));
	root_pw_uid = root.pw_uid;

	nobody = *(getpwnam("nobody"));
	nobody_pw_uid = nobody.pw_uid;

	bin = *(getpwnam("bin"));
	bin_pw_uid = bin.pw_uid;

	/* Check that the test process id is nobody */
	if (geteuid() != nobody.pw_uid) {
/*		tst_brkm(TBROK, NULL, "Must be nobody for this test!");
		tst_exit();*/
		setuid(nobody.pw_uid);
	}

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
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
	}
}
