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
 *    TEST IDENTIFIER   : query_module01
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Checking functionality of query_module(2)
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
 *      Verify that,
 *	1. query_module(2) is successful for NULL module name, which argument
 *	   set to 0.
 *	2. query_module(2) is successful for NULL module name, which argument
 *	   set to QM_MODULES.
 *	3. query_module(2) is successful for valid module name, which argument
 *	   set to QM_DEPS.
 *	4. query_module(2) is successful for valid module name, which argument
 *	   set to QM_REFS.
 *	5. query_module(2) is successful for valid module name, which argument
 *	   set to QM_INFO.
 *	6. query_module(2) is successful for valid module name, which argument
 *	   set to QM_SYMBOLS.
 *
 *      Setup:
 *	  Setup signal handling.
 *	  Test caller is superuser
 *	  Initialize  long module name
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
 *  query_module01 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
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

#define LONGMODNAMECHAR	'm'	/* Arbitrarily selected */
#define MODNAMEMAX	(PAGE_SIZE + 1)
#define EXP_RET_VAL	0
#define DUMMY_MOD	"dummy_query_mod"
#define DUMMY_MOD_DEP	"dummy_query_mod_dep"
#define QM_INVALID	(QM_INFO + 100)

/* Name of exported function in DUMMY_MOD */
#define EXP_FUNC_NAME	"dummy_func_test"

struct test_case_t {		/* test case structure */
	char *modname;
	int which;
	char *desc;
	int (*setup) (void);	/* Individual setup routine */
	void (*cleanup) (void);	/* Individual cleanup routine */
};

char *TCID = "query_module01";
static char longmodname[MODNAMEMAX];
static int testno;
static char out_buf[PAGE_SIZE];
static size_t ret;

static int test_functionality(int, char *, size_t, size_t);
static void setup(void);
static void cleanup(void);
static int setup1(void);
static void cleanup1(void);
static int setup2(void);
static void cleanup2(void);

static struct test_case_t tdat[] = {
	{NULL, 0, "module name: NULL, which: 0", NULL, NULL},

	{NULL, QM_MODULES, "NULL module name, which: QM_MODULES",
	 setup1, cleanup1},

	{DUMMY_MOD_DEP, QM_DEPS, "valid module name, which: QM_DEPS",
	 setup2, cleanup2},

	{DUMMY_MOD, QM_REFS, "valid module name, which: QM_REFS",
	 setup2, cleanup2},

	{DUMMY_MOD, QM_INFO, "valid module name, which: QM_INFO",
	 setup1, cleanup1},

	{DUMMY_MOD, QM_SYMBOLS, "valid module name, which: QM_SYMBOLS",
	 setup1, cleanup1},
};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);

int main(int argc, char **argv)
{
	int lc;
	size_t buflen = sizeof(out_buf);

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
					  tdat[testno].which, (void *)out_buf,
					  buflen, &ret));

			if ((TEST_RETURN == EXP_RET_VAL) &&
			    !test_functionality(tdat[testno].which,
						out_buf, buflen, ret)) {
				tst_resm(TPASS, "query_module() successful "
					 "for %s", tdat[testno].desc);
			} else {
				tst_resm(TFAIL, "query_module() failed for "
					 "%s ; returned"
					 " %d (expected %d), errno %d (expected"
					 " 0)", tdat[testno].desc,
					 TEST_RETURN, EXP_RET_VAL, TEST_ERRNO);
			}
			if (tdat[testno].cleanup) {
				tdat[testno].cleanup();
			}
		}
	}
	cleanup();
	tst_exit();
}

int test_functionality(int which, char *buf, size_t bufsize, size_t ret)
{
	int i = 0;
	char *modname;
	unsigned long *vals;

	switch (which) {
	case 0:
		/* Always return SUCCESS */
		return 0;

	case QM_MODULES:
	case QM_DEPS:
		/* Return SUCCESS if found DUMMY_MOD entry */
		modname = DUMMY_MOD;
		break;

	case QM_REFS:
		/* Return SUCCESS if found DUMMY_MOD_DEP entry */
		modname = DUMMY_MOD_DEP;
		break;

	case QM_INFO:
		/*
		 * Since module is already loaded, flags should show
		 * MOD_RUNNING
		 */
		if (((struct module_info *)buf)->flags & MOD_RUNNING) {
			return 0;
		}
		return 1;

	case QM_SYMBOLS:
		vals = (unsigned long *)buf;

		/*
		 * Find entry for atleast one symbol, checking for
		 * EXP_FUNC_NAME symbol, if found return SUCCESS.
		 */
		for (i = 0; i < ret; i++, vals += 2) {

			/* buf + vals[1] - address of symbol name */
			if (!strcmp(buf + vals[1], EXP_FUNC_NAME)) {
				return 0;
			}
		}
		return 1;

	default:
		/* Unknown which type */
		return 1;
	}

	/* Return SUCCESS if found entry */
	for (i = 0; i != ret; i++) {
		if (strcmp(buf, modname)) {
			buf += strlen(buf) + 1;
		} else {
			return 0;
		}
	}
	return 1;

}

/* Insert a module of name mod */
int insert_mod(char *mod)
{
	char cmd[80];

	if (sprintf(cmd, "cp `which %s.o` ./", mod) == -1) {
		tst_resm(TBROK, "sprintf failed");
		return 1;
	}
	if (system(cmd) != 0) {
		tst_resm(TBROK, "Failed to copy %s module", mod);
		return 1;
	}

	/* Should use force to ignore kernel version & insure loading  */
	/* -RW                                                         */
	/* if (sprintf(cmd, "insmod %s.o", mod) == -1) {               */
	if (sprintf(cmd, "insmod --force -q %s.o >/dev/null 2>&1", mod) == -1) {
		tst_resm(TBROK, "sprintf failed");
		return 1;
	}
	if (system(cmd) != 0) {
		tst_resm(TBROK, "Failed to load %s module", mod);
		return 1;
	}
	return 0;
}

int setup1(void)
{
	if (insert_mod(DUMMY_MOD)) {
		/* Failed */
		return 1;
	} else {
		return 0;
	}
}

int setup2(void)
{
	if (insert_mod(DUMMY_MOD)) {
		/* Failed */
		return 1;
	}
	if (insert_mod(DUMMY_MOD_DEP)) {
		/* Falied to load DUMMY_MOD_DEP, unload DUMMY_MOD */
		cleanup1();
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

void cleanup2(void)
{
	/* Remove the loadable module - DUMMY_MOD_DEP */
	if (system("rmmod " DUMMY_MOD_DEP) != 0) {
		tst_brkm(TBROK, cleanup, "Failed to unload module %s",
			 DUMMY_MOD_DEP);
	}
	/* Remove the loadable module - DUMMY_MOD */
	cleanup1();
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
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */

	tst_rmdir();
}
