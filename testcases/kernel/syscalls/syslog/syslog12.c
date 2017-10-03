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
 *    TEST IDENTIFIER   : syslog12
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Checking error conditions for syslog(2)
 *
 *    TEST CASE TOTAL   : 7
 *
 *    AUTHOR            : Madhu T L <madhu.tarikere@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that,
 *	1. syslog(2) fails with EINVAL for invalid type/command
 *	2. syslog(2) fails with EFAULT for buffer outside program's  accessible
 *	   address space.
 *	3. syslog(2) fails with EINVAL for NULL buffer argument.
 *	4. syslog(2) fails with EINVAL for length arg. set to negative value.
 *	5. syslog(2) fails with EPERM for non-root user.
 *	6. syslog(2) fails with EINVAL for console level less than 0.
 *	7. syslog(2) fails with EINVAL for console level greater than 8.
 *
 *      Setup:
 *	  Setup signal handling.
 *	  Test caller is superuser
 *	  Check existence of user nobody
 *	  Set expected errnos
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return value and errno, if matching,
 *		 Issue PASS message
 *	  Otherwise,
 *		Issue FAIL message
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  syslog12 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
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
#include <signal.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include "test.h"
#include "safe_macros.h"

#define EXP_RET_VAL	-1

struct test_case_t {		/* test case structure */
	int type;		/* 1st arg */
	char *buf;		/* 2nd arg */
	int len;		/* 3rd arg */
	int exp_errno;		/* Expected errno */
	int (*setup) (void);	/* Individual setup routine */
	void (*cleanup) (void);	/* Individual cleanup routine */
	char *desc;		/* Test description */
};

char *TCID = "syslog12";
static int testno;

static char buf;
static struct passwd *ltpuser;

static void setup(void);
static void cleanup(void);
static int setup1(void);
static void cleanup1(void);

#define syslog(arg1, arg2, arg3) syscall(__NR_syslog, arg1, arg2, arg3)

static struct test_case_t tdat[] = {
	{100, &buf, 0, EINVAL, NULL, NULL, "invalid type/command"},
	{2, NULL, 0, EINVAL, NULL, NULL, "NULL buffer argument"},
	{3, &buf, -1, EINVAL, NULL, NULL, "negative length argument"},
	{2, &buf, 0, EPERM, setup1, cleanup1, "non-root user"},
	{8, &buf, -1, EINVAL, NULL, NULL, "console level less than 0"},
	{8, &buf, 9, EINVAL, NULL, NULL, "console level greater than 8"},
};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);

void timeout(int sig)
{
	tst_resm(TWARN, "syslog() timeout after 1s"
		 " for %s", tdat[testno].desc);
}

int main(int argc, char **argv)
{
	int lc;
	struct sigaction sa;
	int ret;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = timeout;
	sa.sa_flags = 0;
	sigaction(SIGALRM, &sa, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		for (testno = 0; testno < TST_TOTAL; ++testno) {

			if (tdat[testno].setup && tdat[testno].setup()) {
				/* Setup failed, skip this testcase */
				continue;
			}

			alarm(1);

			TEST(syslog(tdat[testno].type, tdat[testno].buf,
				    tdat[testno].len));

			alarm(0);
			/* syslog returns an int, so we need to turn the long
			 * TEST_RETURN into an int to test with */
			ret = TEST_RETURN;
			if ((ret == EXP_RET_VAL) &&
			    (TEST_ERRNO == tdat[testno].exp_errno)) {
				tst_resm(TPASS, "syslog() failed as expected"
					 " for %s : errno %d",
					 tdat[testno].desc, TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "syslog() returned "
					 "unexpected results for %s ; returned"
					 " %d (expected %d), errno %d (expected"
					 " %d)", tdat[testno].desc,
					 ret, EXP_RET_VAL, TEST_ERRNO,
					 tdat[testno].exp_errno);
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
