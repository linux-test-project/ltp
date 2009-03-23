/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: mkdir01.c,v 1.7 2009/03/23 13:35:54 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: mkdir01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic errno test for mkdir(2)
 *
 *    PARENT DOCUMENT	: mkstds02
 *
 *    TEST CASE TOTAL	: 2
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: Bill Branum
 *
 *    CO-PILOT		: Kathy Olmsted
 *
 *    DATE STARTED	: 4/15/92
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 * 	mkdir(2) test for errno(s) EFAULT.
 *
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    DURATION
 * 	Terminates - with frequency and infinite modes.
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    ENVIRONMENTAL NEEDS
 *      No run-time environmental needs.
 *
 *    DETAILED DESCRIPTION
 *	This test will verify that mkdir(2) returns a value of
 *	-1 and sets errno to EFAULT when the path argument points
 *	outside (above/below) the allocated address space of the
 *	process.
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Create and make current a temporary directory.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno.
 *        If doing functional test
 *            check the errno returned and print result message
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *	  Remove the temporary directory.
 *	  Exit.
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

#if !defined(UCLINUX)
char *get_high_address();
#endif

char *TCID = "mkdir01";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int exp_enos[] = { EFAULT, 0 };	/* List must end with 0 */

char *bad_addr = 0;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

    /***************************************************************
     * parse standard options
     ***************************************************************/
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * TEST CASE: 1
		 * mkdir() call with pointer below allocated address space.
		 */

		/* Call mkdir(2) */
		TEST(mkdir(bad_addr, 0777));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
		}

	/***************************************************************
	 * only perform functional verification if flag set (-f not given)
	 ***************************************************************/
		if (STD_FUNCTIONAL_TEST) {
			if (TEST_RETURN == -1) {
				if (TEST_ERRNO == EFAULT) {
					tst_resm(TPASS,
						 "mkdir - path argument pointing below allocated address space failed as expected with errno %d : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "mkdir - path argument pointing below allocated address space failed with errno %d : %s but expected %d (EFAULT)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EFAULT);
				}
			} else {
				tst_resm(TFAIL,
					 "mkdir - path argument pointing below allocated address space succeeded unexpectedly.");

			}
		}
#if !defined(UCLINUX)
		/*
		 * TEST CASE: 2
		 * mkdir() call with pointer above allocated address space.
		 */

		/* Call mkdir(2) */
		TEST(mkdir(get_high_address(), 0777));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
		}

	/***************************************************************
	 * only perform functional verification if flag set (-f not given)
	 ***************************************************************/
		if (STD_FUNCTIONAL_TEST) {
			if (TEST_RETURN == -1) {
				if (TEST_ERRNO == EFAULT) {
					tst_resm(TPASS,
						 "mkdir - path argument pointing above allocated address space failed as expected with errno %d : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "mkdir - path argument pointing above allocated address space failed with errno %d : %s but expected %d (EFAULT)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EFAULT);
				}
			} else {
				tst_resm(TFAIL,
					 "mkdir - path argument pointing above allocated address space succeeded unexpectedly.");

			}
		}
#endif /* if !defined(UCLINUX) */

	}			/* End for TEST_LOOPING */

    /***************************************************************
     * cleanup and exit
     ***************************************************************/
	cleanup();

	return 0;
}				/* End main */

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
}				/* End setup() */

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();

	/*
	 * Exit with return code appropriate for results.
	 */
	tst_exit();

}				/* End cleanup() */
