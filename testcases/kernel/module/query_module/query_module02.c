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
 *    TEST IDENTIFIER   : query_module02
 *
 *    EXECUTED BY       : anyone
 *
 *    TEST TITLE        : Checking error conditions for query_module(2)
 *
 *    TEST CASE TOTAL   : 5
 *
 *    AUTHOR            : Madhu T L <madhu.tarikere@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that,
 *	1. query_module(2) returns -1 and sets errno to ENOENT for non-existing
 *	   module.
 *	2. query_module(2) returns -1 and sets errno to EINVAL for invalid
 *	   which argument.
 *	3. query_module(2) returns -1 and sets errno to EINVAL for NULL
 *	   module name and valid which argument.
 *      4. query_module(2) returns -1 and sets errno to EINVAL, if module
 *         name parameter is null terminated (zero length) string.
 *	5. query_module(2) returns -1 and sets errno to ENAMETOOLONG for long
 *	   module name.
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Initialize  long module name
 *	  Set expected errnos for logging
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code and error number, if matching,
 *		Issue PASS message
 *	  Otherwise,
 *		Issue FAIL message
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  query_module02 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
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
#include <limits.h>
#include <asm/atomic.h>
#include <linux/module.h>
#include "test.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#endif

#define NULLMODNAME	""
#define LONGMODNAMECHAR	'm'	/* Arbitrarily selected */
#define MODNAMEMAX	(PAGE_SIZE + 1)
#define EXP_RET_VAL	-1
#define QM_INVALID	(QM_INFO + 100)

struct test_case_t {		/* test case structure */
	char *modname;
	int which;
	void *buf;
	size_t bufsize;
	int experrno;		/* expected errno */
	char *desc;
};

char *TCID = "query_module02";

static char longmodname[MODNAMEMAX];
static int testno;
static char out_buf[PAGE_SIZE];
static size_t ret_size;

static void setup(void);
static void cleanup(void);

static struct test_case_t tdat[] = {

	{"dummy_mod", QM_REFS, (void *)out_buf, sizeof(out_buf), ENOENT,
	 "results for non-existing module"}
	,

	{NULL, QM_INVALID, (void *)out_buf, sizeof(out_buf), EINVAL,
	 "results for invalid which argument"}
	,

	{NULL, QM_REFS, (void *)out_buf, sizeof(out_buf), EINVAL,
	 "results for NULL module name and valid which argument"}
	,

	{NULLMODNAME, QM_REFS, (void *)out_buf, sizeof(out_buf), EINVAL,
	 "results for null terminated (zero lenght) module name"}
	,

	{longmodname, QM_REFS, (void *)out_buf, sizeof(out_buf), ENAMETOOLONG,
	 "results for long module name"}
	,
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

			TEST(query_module(tdat[testno].modname,
					  tdat[testno].which, tdat[testno].buf,
					  tdat[testno].bufsize, &ret_size));
			if ((TEST_RETURN == EXP_RET_VAL) &&
			    (TEST_ERRNO == tdat[testno].experrno)) {
				tst_resm(TPASS, "Expected %s, errno: %d",
					 tdat[testno].desc, TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "Unexpected %s ; returned"
					 " %d (expected %d), errno %d (expected"
					 " %d)", tdat[testno].desc,
					 TEST_RETURN, EXP_RET_VAL,
					 TEST_ERRNO, tdat[testno].experrno);
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

	if (tst_kvercmp(2, 5, 48) >= 0)
		tst_brkm(TCONF, NULL, "This test will not work on "
			 "kernels after 2.5.48");

	/* Initialize longmodname to LONGMODNAMECHAR character */
	memset(longmodname, LONGMODNAMECHAR, MODNAMEMAX - 1);

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
