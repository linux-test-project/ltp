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
 *    TEST IDENTIFIER   : setresgid02
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Checking functionality of setresgid(2) for
 *			  non-root group id.
 *
 *    TEST CASE TOTAL   : 6
 *
 *    AUTHOR            : Madhu T L <madhu.tarikere@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *      Verify that for non-root effective group id,
 *	1. setresgid(2) is successful for setresgid(-1, -1, -1)
 *	2. setresgid(2) is successful for setresgid(-1, -1, bin)
 *	3. setresgid(2) is successful for setresgid(-1, bin, -1)
 *	4. setresgid(2) is successful for setresgid(bin, -1, -1)
 *	5. setresgid(2) is successful for setresgid(root, root, root)
 *	6. setresgid(2) is successful for setresgid(root, nobody, nobody)
 *
 *      Setup:
 *	  Setup signal handling.
 *	  Test caller is superuser
 *	  Check existence of root, bin and nobody user id's
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
 *  setresgid02 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
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
 * CHANGE:  Madhu T L <madhu.tarikere@wipro.com>
 * Date: April 9 2003
 * Replaced setegid() by setresgid() in setup()
 ****************************************************************/

#define _GNU_SOURCE 1
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include "test.h"
#include "safe_macros.h"
#include "compat_16.h"

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

TCID_DEFINE(setresgid02);
static int testno;
static struct passwd nobody, bin, root;
static uid_t nobody_gid, root_gid, bin_gid, neg = -1;

static int test_functionality(uid_t, uid_t, uid_t);
static void setup(void);
static void cleanup(void);

/* Don't change order of these test cases */
static struct test_case_t tdat[] = {
	{&neg, &neg, &neg, &root, &nobody, &nobody,
	 "setresgid(-1, -1, -1)"},
	{&neg, &neg, &bin.pw_gid, &root, &nobody, &bin,
	 "setresgid(-1, -1, bin)"},
	{&neg, &bin.pw_gid, &neg, &root, &bin, &bin,
	 "setresgid(-1, bin, -1)"},
	{&bin.pw_gid, &neg, &neg, &bin, &bin, &bin,
	 "setresgid(bin, -1, -1)"},
	{&root.pw_gid, &root.pw_gid, &root.pw_gid, &root, &root, &root,
	 "setresgid(root, root, root)"},
	{&root.pw_gid, &nobody.pw_gid, &nobody.pw_gid, &root, &nobody, &nobody,
	 "setresgid(root, nobody, nobody)"},
};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		for (testno = 0; testno < TST_TOTAL; ++testno) {

			TEST(SETRESGID(cleanup, *tdat[testno].rgid, *tdat[testno].egid,
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

	/* Get current real, effective and saved group id */
	SAFE_GETRESGID(cleanup, &cur_rgid, &cur_egid, &cur_sgid);

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

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	if ((passwd_p = getpwnam("root")) == NULL) {
		tst_brkm(TBROK, NULL, "getpwnam() failed for root");

	}
	root = *passwd_p;
	GID16_CHECK((root_gid = root.pw_gid), "setresgid", cleanup)

	if ((passwd_p = getpwnam("bin")) == NULL) {
		tst_brkm(TBROK, NULL, "bin user id doesn't exist");

	}
	bin = *passwd_p;
	GID16_CHECK((bin_gid = bin.pw_gid), "setresgid", cleanup)

	if ((passwd_p = getpwnam("nobody")) == NULL) {
		tst_brkm(TBROK, NULL, "nobody user id doesn't exist");

	}
	nobody = *passwd_p;
	GID16_CHECK((nobody_gid = nobody.pw_gid), "setresgid", cleanup)

	/* Set effective/saved gid to nobody */
	if (setresgid(-1, nobody_gid, nobody_gid) == -1) {
		tst_brkm(TBROK, NULL, "setup() failed for setting while"
			 " setting real/effective/saved gid");

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

}
