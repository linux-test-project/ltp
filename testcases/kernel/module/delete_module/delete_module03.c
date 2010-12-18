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
 *    TEST IDENTIFIER   : delete_module03
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Checking error conditions for delete_module(2)
 *
 *    TEST CASE TOTAL   : 1
 *
 *    AUTHOR            : Madhu T L <madhu.tarikere@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *      Verify that, delete_module(2) returns -1 and sets errno to EBUSY, if
 *		 tried to remove a module in-use.
 *
 *      Setup:
 *        Setup signal handling.
 *        Test caller is super user
 *        Set expected errnos for logging
 *        Pause for SIGUSR1 if option specified.
 *		   Insert loadable modules
 *
 *      Test:
 *       Loop if the proper options are given.
 *        Execute system call
 *        Check return code and error number, if matching,
 *                   Issue PASS message
 *        Otherwise,
 *                   Issue FAIL message
 *
 *      Cleanup:
 *        Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  delete_module03 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
 *		 		 where,  -c n : Run n copies concurrently. (no
 *		 		 		effect)
 *	 		 		 -e   : Turn on errno logging.
 *	 		 		 -f   : Turn off functional testing
 *	 		 		 -h   : Show help screen
 *	 		 		 -i n : Execute test n times.
 *	 		 		 -I x : Execute test for x seconds.
 *	 		 		 -p   : Pause for SIGUSR1 before
 *	 		 		 	starting
 *	 		 		 -P x : Pause for x seconds between
 *	 		 		 	iterations.
 *	 		 		 -t   : Turn on syscall timing.
 *
 * RESTRICTIONS
 *		 -c option has no effect for this testcase, even if used allows
 *		 only one instance to run at a time.
 *
 * CHANGELOG
 *
 *  11/22/02 -	Added "--force" to insmod options and redirected output to
 *  		/dev/null. This was done to allow kernel mismatches, b/c it
 *  		doesn't matter in this case.
 *		Robbie Williamson <robbiew@us.ibm.com>
 *
 ****************************************************************/

#include <libgen.h>
#include <errno.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"


#define DUMMY_MOD		 "dummy_del_mod"
#define DUMMY_MOD_DEP		 "dummy_del_mod_dep"
#define EXP_RET_VAL		 -1
#define EXP_ERRNO		 EWOULDBLOCK
/*#define EXP_ERRNO		 EBUSY */

char *TCID = "delete_module03";
/*static int exp_enos[] = {EBUSY, 0}; */
static int exp_enos[] = {EWOULDBLOCK, 0};
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

int
main(int argc, char **argv)
{
	int lc;		 		 /* loop counter */
	char *msg;	 		 /* message returned from parse_opts */
	char cmd[50];

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (STD_COPIES != 1) {
		tst_resm(TINFO, "-c option has no effect for this testcase - "
				"doesn't allow running more than one instance "
		 		"at a time");
		STD_COPIES = 1;
	}

	/* Load first kernel module */
	if (sprintf(cmd, "/sbin/insmod %s/%s.ko", dirname(argv[0]),
		DUMMY_MOD) <= 0) {
		tst_resm(TBROK, "sprintf failed");
		return 1;
	}
	if ((system(cmd)) != 0) {
		tst_resm(TBROK, "Failed to load %s module", DUMMY_MOD);
		return 1;
	}

	/* Load dependant kernel module */
        if (sprintf(cmd, "/sbin/insmod %s/%s.ko", dirname(argv[0]),
		DUMMY_MOD_DEP) <= 0) {
		tst_resm(TBROK, "sprintf failed");
		goto END;
	}
        if ((system(cmd)) != 0) {
		tst_resm(TBROK, "Failed to load %s module", DUMMY_MOD_DEP);
		goto END;
        }

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* Test the system call */
		TEST(delete_module(DUMMY_MOD));

		TEST_ERROR_LOG(TEST_ERRNO);
		if ((TEST_RETURN == (int) EXP_RET_VAL) &&
		     (TEST_ERRNO == EXP_ERRNO) ) {
			tst_resm(TPASS, "Expected failure for module in-use, "
		 			"errno: %d", TEST_ERRNO);
		} else {
			tst_resm(TFAIL, "Unexpected results for module in-use; "
		 			"returned %d (expected %d), errno %d "
					"(expected %d)", TEST_RETURN,
					EXP_RET_VAL, TEST_ERRNO, EXP_ERRNO);
		}
	}
	cleanup();
END:
	if (system("rmmod "DUMMY_MOD) != 0) {
		tst_resm(TBROK, "Failed to unload %s module", DUMMY_MOD);
		return 1;
	}

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void
setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);

	tst_tmpdir();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

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
void
cleanup(void)
{
	/* Unload dependent kernel module */
	if (system("rmmod "DUMMY_MOD_DEP) != 0) {
		tst_resm(TBROK, "Failed to unload %s module", DUMMY_MOD_DEP);
	}
	/* Unload first kernel module */
	if (system("rmmod "DUMMY_MOD) != 0) {
		tst_resm(TBROK, "Failed to unload %s module",
		DUMMY_MOD);
	}
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;
	tst_rmdir();
}