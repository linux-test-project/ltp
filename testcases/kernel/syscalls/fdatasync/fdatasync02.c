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
 *    TEST IDENTIFIER   : fdatasync02
 *
 *    EXECUTED BY       : Any user
 *
 *    TEST TITLE        : Checking error conditions for fdatasync(2)
 *
 *    TEST CASE TOTAL   : 2
 *
 *    AUTHOR            : Madhu T L <madhu.tarikere@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *      Verify that,
 *      1. fdatasync(2) returns -1 and sets errno to EBADF for invalid
 *	   file descriptor.
 *      2. fdatasync(2) returns -1 and sets errno to EINVAL for file
 *         descriptor to a special file.
 *
 *      Setup:
 *        Setup signal handling.
 *        Set expected errnos for logging
 *        Pause for SIGUSR1 if option specified.
 *
 *      Test:
 *       Loop if the proper options are given.
 *	  Perform testcase specific setup (if needed)
 *        Execute system call
 *        Check return code and error number, if matching,
 *                   Issue PASS message
 *        Otherwise,
 *                   Issue FAIL message
 *	  Perform testcase specific cleanup (if needed)
 *
 *      Cleanup:
 *        Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  fdatasync02 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"
#include "tso_safe_macros.h"

#define EXP_RET_VAL	-1
#define SPL_FILE	"/dev/null"

struct test_case_t {		/* test case structure */
	int experrno;		/* expected errno */
	char *desc;
	int (*setup) (void);	/* Individual setup routine */
	void (*cleanup) (void);	/* Individual cleanup routine */
};

char *TCID = "fdatasync02";

static int testno;
static int fd;

static void setup(void);
static void cleanup(void);
static int setup1(void);
static int setup2(void);
static void cleanup2(void);

static struct test_case_t tdat[] = {
	{EBADF, "invalid file descriptor", setup1, NULL},
	{EINVAL, "file descriptor to a special file", setup2, cleanup2},
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
			if ((tdat[testno].setup) && (tdat[testno].setup())) {
				/* setup() failed, skip this test */
				continue;
			}

			/* Test the system call */
			TEST(fdatasync(fd));
			if ((TEST_RETURN == EXP_RET_VAL) &&
			    (TEST_ERRNO == tdat[testno].experrno)) {
				tst_resm(TPASS, "Expected failure for %s, "
					 "errno: %d", tdat[testno].desc,
					 TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "Unexpected results for %s ; "
					 "returned %ld (expected %d), errno %d "
					 "(expected %d)", tdat[testno].desc,
					 TEST_RETURN, EXP_RET_VAL,
					 TEST_ERRNO, tdat[testno].experrno);
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
	fd = -1;
	return 0;
}

int setup2(void)
{
	/* Open special file */
	if ((fd = open(SPL_FILE, O_RDONLY)) == -1) {
		tst_resm(TBROK, "Failed to open %s", SPL_FILE);
		return 1;
	}
	return 0;
}

void cleanup2(void)
{
	/* close special file */
	SAFE_CLOSE(NULL, fd);
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

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
