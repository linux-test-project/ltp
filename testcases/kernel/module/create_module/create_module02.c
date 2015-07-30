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
 *    TEST IDENTIFIER   : create_module02
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Checking error conditions for create_module(2)
 *
 *    TEST CASE TOTAL   : 8
 *
 *    AUTHOR            : Madhu T L <madhu.tarikere@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *      Verify that,
 *      1. create_module(2) returns -1 and sets errno to EPERM, if effective
 *         user id of the caller is not superuser.
 *      2. create_module(2) returns -1 and sets errno to EFAULT, if module
 *         name is outside the  program's  accessible  address space.
 *      3. create_module(2) returns -1 and sets errno to EFAULT, if module
 *         name parameter is NULL.
 *      4. create_module(2) returns -1 and sets errno to EINVAL, if module
 *         name parameter is null terminated (zero length) string.
 *      5. create_module(2) returns -1 and sets errno to EEXIST, if module
 *         entry with the same name already exists.
 *      6. create_module(2) returns -1 and sets errno to EINVAL, if module
 *         size parameter is too small.
 *      7. create_module(2) returns -1 and sets errno to ENAMETOOLONG, if
 *         module name parameter is too long.
 *      8. create_module(2) returns -1 and sets errno to ENOMEM, if module
 *         size parameter is too large.
 *
 *      Setup:
 *        Setup signal handling.
 *        Test caller is super user
 *        Check existances of "nobody" user id.
 *        Initialize  long module name
 *        Set expected errnos for logging
 *        Pause for SIGUSR1 if option specified.
 *	  Initialize modname for each child process
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
 *  create_module02 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
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

#define MODSIZE 10000		/* Arbitrarily selected MODSIZE */
#define NULLMODNAME ""
#define MAXMODSIZE  0xffffffffffffffff	/* Max size of size_t */
#define SMALLMODSIZE  1		/* Arbitrarily selected SMALLMODSIZE */
#define BASEMODNAME "dummy"
#define LONGMODNAMECHAR 'm'	/* Arbitrarily selected the alphabet */
#define MODNAMEMAX (PAGE_SIZE + 1)

struct test_case_t {		/* test case structure */
	char *modname;
	size_t size;
	caddr_t retval;		/* syscall return value */
	int experrno;		/* expected errno */
	char *desc;
	int (*setup) (void);	/* Individual setup routine */
	void (*cleanup) (void);	/* Individual cleanup routine */
};

char *TCID = "create_module02";
static char nobody_uid[] = "nobody";
struct passwd *ltpuser;
static char longmodname[MODNAMEMAX];
static int testno;
static char modname[20];	/* Name of the module */

static void setup(void);
static void cleanup(void);
static int setup1(void);
static void cleanup1(void);
static int setup2(void);
static void cleanup2(void);

static struct test_case_t tdat[] = {
	{modname, MODSIZE, (caddr_t) - 1, EPERM,
	 "non-superuser", setup1, cleanup1},
	{(char *)-1, MODSIZE, (caddr_t) - 1, EFAULT,
	 "module name outside the  program's  accessible  address space", NULL,
	 NULL},
	{NULL, MODSIZE, (caddr_t) - 1, EFAULT,
	 "NULL module name", NULL, NULL},
	{NULLMODNAME, MODSIZE, (caddr_t) - 1, EINVAL,
	 "null terminated module name", NULL, NULL},
	{modname, MODSIZE, (caddr_t) - 1, EEXIST,
	 "already existing module", setup2, cleanup2},
	{modname, SMALLMODSIZE, (caddr_t) - 1, EINVAL,
	 "insufficient module size", NULL, NULL},
	{longmodname, MODSIZE, (caddr_t) - 1, ENAMETOOLONG,
	 "long module name", NULL, NULL},

	/*
	 *This test case is giving segmentation fault on
	 * 2.4.* series, but works as expected with 2.5.* series of kernel.
	 */
	{modname, MAXMODSIZE, (caddr_t) - 1, ENOMEM,
	 "large module size", NULL, NULL},
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

			TEST(create_module(tdat[testno].modname,
					   tdat[testno].size));
			if ((TEST_RETURN == (int)tdat[testno].retval) &&
			    (TEST_ERRNO == tdat[testno].experrno)) {
				tst_resm(TPASS, "Expected results for %s, "
					 "errno: %d",
					 tdat[testno].desc, TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "Unexpected results for %s ; "
					 "returned %d (expected %d), errno %d "
					 "(expected %d)", tdat[testno].desc,
					 TEST_RETURN, tdat[testno].retval,
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
	if (seteuid(0) == -1) {
		tst_brkm(TBROK, NULL, "seteuid failed to set the effective"
			 " uid to root");
	}
}

int setup2(void)
{
	/* Create a loadable module entry */
	if (create_module(modname, MODSIZE) == -1) {
		tst_resm(TBROK, "Failed to create module entry"
			 " for %s", modname);
		return 1;
	}
	return 0;
}

void cleanup2(void)
{
	/* Remove loadable module entry */
	if (delete_module(modname) == -1) {
		tst_brkm(TBROK, NULL, "Failed to delete module entry"
			 " for %s", modname);
	}
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	if (tst_kvercmp(2, 5, 48) >= 0)
		tst_brkm(TCONF, NULL, "This test will not work on "
			 "kernels after 2.5.48");

	/* Check for nobody_uid user id */
	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		tst_brkm(TBROK, NULL, "Required user %s doesn't exists",
			 nobody_uid);
	}

	/* Initialize longmodname to LONGMODNAMECHAR character */
	memset(longmodname, LONGMODNAMECHAR, MODNAMEMAX - 1);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;

	/* Get unique module name for each child process */
	if (sprintf(modname, "%s_%d", BASEMODNAME, getpid()) == -1) {
		tst_brkm(TBROK, NULL, "Failed to initialize module name");
	}
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
}
