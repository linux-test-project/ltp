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
 *    TEST IDENTIFIER   : query_module03
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Checking error conditions for query_module(2)
 *
 *    TEST CASE TOTAL   : 4
 *
 *    AUTHOR            : Madhu T L <madhu.tarikere@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that,
 *	1. query_module(2) returns -1 and sets errno to EFAULT for module name
 *	   argument outside program's accessible address space.
 *	2. query_module(2) returns -1 and sets errno to EFAULT for return size
 *	   argument outside program's accessible address space.
 *	3. query_module(2) returns -1 and sets errno to EFAULT for output buffer
 *	   argument outside program's accessible address space.
 *	4. query_module(2) returns -1 and sets errno to ENOSPC for too small
 *	   buffer size.
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Test caller is superuser
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
 *  query_module03 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
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
 * RESTRICTIONS
 *	-c option has no effect for this testcase, even if used allows only
 *	one instance to run at a time.
 *
 * CHANGES
 *
 * 12/03/02 Added "force" to insmod to ignore kernel version.
 *          -Robbie Williamson <robbiew@us.ibm.com>
 *
 ****************************************************************/

#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <asm/atomic.h>
#include <linux/module.h>
#include <sys/mman.h>
#include "test.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#endif

#define EXP_RET_VAL	-1
#define DUMMY_MOD	"dummy_query_mod"
#define SMALLBUFSIZE	1

struct test_case_t {		/* test case structure */
	char *modname;
	int which;
	void *buf;
	size_t bufsize;
	size_t *ret_size;
	int experrno;		/* expected errno */
	char *desc;
	int (*setup) (void);
	void (*cleanup) (void);
};

char *TCID = "query_module03";

static int testno;
static char out_buf[PAGE_SIZE];
static size_t ret_size;

char *bad_addr = 0;

static void setup(void);
static void cleanup(void);
static int setup1(void);
static void cleanup1(void);

static struct test_case_t tdat[] = {

	{(char *)-1, QM_MODULES, (void *)out_buf, sizeof(out_buf), &ret_size,
	 EFAULT, "results for module name argument outside program's "
	 "accessible address space", NULL, NULL}
	,

	{NULL, QM_MODULES, (void *)out_buf, sizeof(out_buf), (size_t *) - 1,
	 EFAULT, "results for return size argument outside program's "
	 "accessible address space", NULL, NULL}
	,

	{NULL, QM_MODULES, (void *)-1, sizeof(out_buf), &ret_size, EFAULT,
	 "results for output buffer argument outside program's "
	 "accessible address space", setup1, cleanup1}
	,

	{NULL, QM_MODULES, (void *)out_buf, SMALLBUFSIZE, &ret_size, ENOSPC,
	 "results for too small buffer size", setup1, cleanup1},
};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	tst_tmpdir();
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		for (testno = 0; testno < TST_TOTAL; ++testno) {

			if ((tdat[testno].setup) && (tdat[testno].setup())) {
				/* setup() failed, skip this test */
				continue;
			}
			TEST(query_module(tdat[testno].modname,
					  tdat[testno].which, tdat[testno].buf,
					  tdat[testno].bufsize,
					  tdat[testno].ret_size));
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
	char cmd[80];

	if (sprintf(cmd, "cp `which %s.o` ./", DUMMY_MOD) == -1) {
		tst_resm(TBROK, "sprintf failed");
		return 1;
	}
	if (system(cmd) != 0) {
		tst_resm(TBROK, "Failed to copy %s module", DUMMY_MOD);
		return 1;
	}

	/* Should use force to ignore kernel version & insure loading  */
	/* -RW                                                         */
	/* if (sprintf(cmd, "insmod %s.o", DUMMY_MOD) == -1) {         */
	if (sprintf(cmd, "insmod --force -q %s.o >/dev/null 2>&1", DUMMY_MOD) ==
	    -1) {
		tst_resm(TBROK, "sprintf failed");
		return 1;
	}
	if (system(cmd) != 0) {
		tst_resm(TBROK, "Failed to load %s module", DUMMY_MOD);
		return 1;
	}
	return 0;
}

void cleanup1(void)
{
	/* Remove the loadable module - DUMMY_MOD */
	if (system("rmmod " DUMMY_MOD) != 0) {
		tst_brkm(TBROK, cleanup, "Failed to unload module %s",
			 DUMMY_MOD);
	}
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	if (tst_kvercmp(2, 5, 48) >= 0)
		tst_brkm(TCONF, NULL, "This test will not work on "
			 "kernels after 2.5.48");

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;

	bad_addr = mmap(0, 1, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	tdat[0].modname = bad_addr;
	tdat[2].buf = (void *)bad_addr;

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
	tst_rmdir();
}
