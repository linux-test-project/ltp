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
 *    TEST IDENTIFIER   : syslog11
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Basic tests for syslog(2)
 *
 *    TEST CASE TOTAL   : 11
 *
 *    AUTHOR            : Madhu T L <madhu.tarikere@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that, syslog(2) is successful for type ranging from 1 to 8
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Test caller is superuser
 *	  Check existence of user nobody
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return value, if not successful,
 *		 Issue FAIL message
 *	  Otherwise,
 *		Issue PASS message
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  syslog11 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
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

#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include "test.h"
#include "safe_macros.h"

#define UNEXP_RET_VAL	-1

struct test_case_t {		/* test case structure */
	int type;		/* 1st arg. */
	char *buf;		/* 2nd arg. */
	int len;		/* 3rd arg. */
	int (*setup) (void);	/* Individual setup routine */
	void (*cleanup) (void);	/* Individual cleanup routine */
	char *desc;		/* Test description */
};

char *TCID = "syslog11";
static int testno;
static char buf;
static struct passwd *ltpuser;

static void setup(void);
static void cleanup(void);
static int setup1(void);
static void cleanup1(void);

#define syslog(arg1, arg2, arg3) syscall(__NR_syslog, arg1, arg2, arg3)

static struct test_case_t tdat[] = {
	/* Type 0 and 1 are currently not implemented, always returns success */
	{0, &buf, 0, NULL, NULL, "type 0/Close the log"},
	{1, &buf, 0, NULL, NULL, "type 1/Open the log"},
	{2, &buf, 0, NULL, NULL, "type 2/Read from the log"},
	{3, &buf, 0, NULL, NULL, "type 3/Read ring buffer"},
	{3, &buf, 0, setup1, cleanup1, "type 3/Read ring buffer for non-root "
	 "user"},
	/* Next two lines will clear dmesg. Uncomment if that is okay. -Robbie Williamson */
	/*    { 4, &buf, 0, NULL, NULL, "type 4/Read and clear ring buffer" },            */
	/*    { 5, &buf, 0, NULL, NULL, "type 5/Clear ring buffer" },                     */

	{8, NULL, 1, NULL, NULL, "type 8/Set log level to 1"},
	{8, NULL, 7, NULL, NULL, "type 8/Set log level to 7(default)"},
	{6, NULL, 0, NULL, NULL, "type 6/Disable printk's to console"},
	{7, NULL, 0, NULL, NULL, "type 7/Enable printk's to console"},
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

			if (tdat[testno].setup && tdat[testno].setup()) {
				/* Setup failed, skip this testcase */
				continue;
			}

			TEST(syslog(tdat[testno].type, tdat[testno].buf,
				    tdat[testno].len));

			if (TEST_RETURN == UNEXP_RET_VAL) {
				if (TEST_ERRNO == EPERM && geteuid() != 0) {
					tst_resm(TPASS,
						 "syslog() passed for %s (non-root EPERM is OK)",
						 tdat[testno].desc);
				} else {
					tst_resm(TFAIL,
						 "syslog() failed for %s: errno "
						 "%d (%s)", tdat[testno].desc,
						 TEST_ERRNO, strerror(errno));
				}
			} else {
				tst_resm(TPASS, "syslog() successful for %s",
					 tdat[testno].desc);
			}

			if (tdat[testno].cleanup) {
				tdat[testno].cleanup();
			}
		}
	}
	cleanup();

	tst_exit();
}

int setup1(void)
{
	/* Change effective user id to nodody */
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TBROK, "seteuid failed to set the effective"
			 " uid to %d", ltpuser->pw_uid);
		return 1;
	}
	return 0;
}

void cleanup1(void)
{
	/* Change effective user id to root */
	SAFE_SETEUID(NULL, 0);
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Check for nobody_uid user id */
	if ((ltpuser = getpwnam("nobody")) == NULL) {
		tst_brkm(TBROK, NULL, "nobody user id doesn't exist");
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
