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
 *    TEST IDENTIFIER   : setresgid03
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Checking error conditions for setresgid(2)
 *
 *    TEST CASE TOTAL   : 4
 *
 *    AUTHOR            : Madhu T L <madhu.tarikere@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *      Verify that,
 *	1. setresgid(2) fails with EPERM for unprivileged user in setting
 *	   saved group id.
 *	2. setresgid(2) fails with EPERM for unprivileged user in setting
 *	   effective group id.
 *	3. setresgid(2) fails with EPERM for unprivileged user in setting
 *	   real group id.
 *	4. setresgid(2) fails with EPERM for unprivileged user in setting
 *	   real/effective/saved group id.
 *
 *      Setup:
 *	  Setup signal handling.
 *	  Test caller is superuser
 *	  Check existence of user id's root/bin/nobody
 *	  Set real/effective/saved gid to nobody
 *	  Set effective uid to nobody
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return value, errno and functionality, if success,
 *		 Issue PASS message
 *	Otherwise,
 *		Issue FAIL message
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  setresgid03 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
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

#define EXP_RET_VAL	-1
#define EXP_ERRNO	EPERM
#define TEST_DESC	"unprivileged user"

extern int Tst_count;

struct test_case_t {		/* test case structure */
	uid_t *rgid;		/* real GID */
	uid_t *egid;		/* effective GID */
	uid_t *sgid;		/* saved GID */
	struct passwd *exp_rgid;	/* Expected real GID */
	struct passwd *exp_egid;	/* Expected effective GID */
	struct passwd *exp_sgid;	/* Expected saved GID */
};

char *TCID = "setresgid03";
static int testno;
static struct passwd nobody, bin, root;
static uid_t nobody_gid, bin_gid, neg = -1;
static int exp_enos[] = { EPERM, 0 };

static int test_functionality(uid_t, uid_t, uid_t);
static void setup(void);
static void cleanup(void);

static struct test_case_t tdat[] = {
	{&neg, &neg, &bin.pw_gid, &nobody, &nobody, &nobody},
	{&neg, &bin.pw_gid, &neg, &nobody, &nobody, &nobody},
	{&bin.pw_gid, &neg, &neg, &nobody, &nobody, &nobody},
	{&bin.pw_gid, &bin.pw_gid, &bin.pw_gid, &nobody, &nobody, &nobody},
};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		for (testno = 0; testno < TST_TOTAL; ++testno) {

			TEST(setresgid(*tdat[testno].rgid, *tdat[testno].egid,
				       *tdat[testno].sgid));

			TEST_ERROR_LOG(TEST_ERRNO);
			if ((TEST_RETURN == EXP_RET_VAL) &&
			    (TEST_ERRNO == EXP_ERRNO)) {

				if (!test_functionality
				    (tdat[testno].exp_rgid->pw_gid,
				     tdat[testno].exp_egid->pw_gid,
				     tdat[testno].exp_sgid->pw_gid)) {

					tst_resm(TPASS, "setresgid() failed as "
						 "expected for %s : errno %d",
						 TEST_DESC, TEST_ERRNO);
				} else {
					tst_resm(TFAIL, "Functionality test "
						 "for setresgid() for %s failed",
						 TEST_DESC);
				}

			} else {
				tst_resm(TFAIL, "setresgid() returned "
					 "unexpected results for %s ; returned"
					 " %ld (expected %d), errno %d (expected"
					 " %d)", TEST_DESC,
					 TEST_RETURN, EXP_RET_VAL, TEST_ERRNO,
					 EXP_ERRNO);
			}
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
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
	/* Get current real, effective and saved group id */
	if (getresgid(&cur_rgid, &cur_egid, &cur_sgid) == -1) {
		tst_brkm(TBROK, cleanup, "getresgid() failed");
		/* NOT REACHED */
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

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Check whether we are root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Must be root for this test!");
	 /*NOTREACHED*/}

	if ((passwd_p = getpwnam("root")) == NULL) {
		tst_brkm(TBROK, tst_exit, "getpwnam() failed for root");
		/* NOTREACHED */
	}
	root = *passwd_p;

	if ((passwd_p = getpwnam("bin")) == NULL) {
		tst_brkm(TBROK, tst_exit, "bin user id doesn't exist");
		/* NOTREACHED */
	}
	bin = *passwd_p;
	bin_gid = bin.pw_gid;

	if ((passwd_p = getpwnam("nobody")) == NULL) {
		tst_brkm(TBROK, tst_exit, "nobody user id doesn't exist");
		/* NOTREACHED */
	}
	nobody = *passwd_p;
	nobody_gid = nobody.pw_gid;

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Set real/effective/saved gid to nobody */
	if (setresgid(nobody_gid, nobody_gid, nobody_gid) == -1) {
		tst_brkm(TBROK, tst_exit, "setup() failed for setting while"
			 " setting real/effective/saved gid");
		/* NOTREACHED */
	}
	/* Set euid to nobody */
	if (setuid(nobody.pw_uid) == -1) {
		tst_brkm(TBROK, tst_exit, "setuid failed to "
			 "to set the effective uid to nodody");
		/* NOTREACHED */
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

	/* exit with return code appropriate for results */
	tst_exit();
 /*NOTREACHED*/}
