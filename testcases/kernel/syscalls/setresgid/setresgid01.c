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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER   : setresgid01
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Checking functionality of setresgid(2)
 *
 *    TEST CASE TOTAL   : 5
 *
 *    AUTHOR            : Madhu T L <madhu.tarikere@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *      Verify that,
 *	1. setresgid(2) is successful for setresgid(-1, -1, -1)
 *	2. setresgid(2) is successful for setresgid(-1, -1, nobody)
 *	3. setresgid(2) is successful for setresgid(-1, nobody, -1)
 *	4. setresgid(2) is successful for setresgid(nobody, -1, -1)
 *	5. setresgid(2) is successful for setresgid(root, root, root)
 *
 *      Setup:
 *	  Setup signal handling.
 *	  Test caller is superuser
 *	  Check existence of root and nobody user id's
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return value and functionality, if success,
 *		 Issue PASS message
 *	Otherwise,
 *		Issue FAIL message
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  setresgid01 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
 *		where,  -c n : Run n copies concurrently.
 *			-e   : Turn on errno logging.
 *			-f   : Turn off functional testing
 *			-h   : Show help screen
 *			-i n : Execute test n times.
 *			-I x : Execute test for x seconds.
 *			-p   : Pause for SIGUSR1 before starting
 *			-P x : Pause for x seconds between iterations.
 *			-t   : Turn on syscall timing.
 *
 ****************************************************************/

#define _GNU_SOURCE 1
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

#define EXP_RET_VAL	0


struct test_case_t {		/* test case structure */
	uid_t *rgid;		/* real GID */
	uid_t *egid;		/* effective GID */
	uid_t *sgid;		/* saved GID */
	struct passwd *exp_rgid;	/* Expected real GID */
	struct passwd *exp_egid;	/* Expected effective GID */
	struct passwd *exp_sgid;	/* Expected saved GID */
	char *desc;		/* Test description */
};

char *TCID = "setresgid01";
static int testno;
static struct passwd nobody, root;
static uid_t nobody_gid, root_gid, neg = -1;

static int test_functionality(uid_t, uid_t, uid_t);
static void setup(void);
static void cleanup(void);

/* Don't change order of these test cases */
static struct test_case_t tdat[] = {
	{&neg, &neg, &neg, &root, &root, &root,
	 "setresgid(-1, -1, -1)"},
	{&neg, &neg, &nobody.pw_gid, &root, &root, &nobody,
	 "setresgid(-1, -1, nobody)"},
	{&neg, &nobody.pw_gid, &neg, &root, &nobody, &nobody,
	 "setresgid(-1, nobody, -1)"},
	{&nobody.pw_gid, &neg, &neg, &nobody, &nobody, &nobody,
	 "setresgid(nobody, -1, -1)"},
	{&root.pw_gid, &root.pw_gid, &root.pw_gid, &root, &root, &root,
	 "setresgid(root, root, root)"},
};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, NULL, NULL)) !=
	    NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		for (testno = 0; testno < TST_TOTAL; ++testno) {

			TEST(setresgid(*tdat[testno].rgid, *tdat[testno].egid,
				       *tdat[testno].sgid));

			if (TEST_RETURN == EXP_RET_VAL) {
				if (!test_functionality
				    (tdat[testno].exp_rgid->pw_gid,
				     tdat[testno].exp_egid->pw_gid,
				     tdat[testno].exp_sgid->pw_gid)) {

					tst_resm(TPASS, "Test for %s "
						 "successful",
						 tdat[testno].desc);
				} else {
					tst_resm(TFAIL, "Functionality test "
						 "for %s failed",
						 tdat[testno].desc);
				}
			} else {
				tst_resm(TFAIL, "Test for %s failed; returned"
					 " %ld (expected %d), errno %d (expected"
					 " 0)", tdat[testno].desc,
					 TEST_RETURN, EXP_RET_VAL, TEST_ERRNO);
			}
		}
	}
	cleanup();

	tst_exit();
}

static int test_functionality(uid_t exp_rgid, uid_t exp_egid, uid_t exp_sgid)
{
	uid_t cur_rgid, cur_egid, cur_sgid;

	/*
	 * Perform functional verification, if STD_FUNCTIONAL_TEST is
	 * set (-f options is not used)
	 */
	if (STD_FUNCTIONAL_TEST == 0) {
		return 0;
	}
	/* Get current real, effective and saved group id's */
	if (getresgid(&cur_rgid, &cur_egid, &cur_sgid) == -1) {
		tst_brkm(TBROK, cleanup, "getresgid() failed");

	}

	if ((cur_rgid == exp_rgid) && (cur_egid == exp_egid)
	    && (cur_sgid == exp_sgid)) {
		return 0;
	}
	return 1;
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	struct passwd *passwd_p;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Check whether we are root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be root for this test!");
	 }

	if ((passwd_p = getpwnam("root")) == NULL) {
		tst_brkm(TBROK, NULL, "getpwnam() failed for root");

	}
	root = *passwd_p;
	root_gid = root.pw_gid;

	if ((passwd_p = getpwnam("nobody")) == NULL) {
		tst_brkm(TBROK, NULL, "nobody user id doesn't exist");

	}
	nobody = *passwd_p;
	nobody_gid = nobody.pw_gid;

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