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
/* $Id: access03.c,v 1.8 2009/03/23 13:35:39 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: access03
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: EFAULT error testing for access(2)
 *
 *    PARENT DOCUMENT	: acstds01
 *
 *    TEST CASE TOTAL	: 8
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: Kathy Olmsted
 *
 *    CO-PILOT		: Tom Hampson
 *
 *    DATE STARTED	: 05/13/92
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 *	access(2) test for errno(s) EFAULT.
 *
 *    INPUT SPECIFICATIONS
 *	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    DURATION
 *	Terminates - with frequency and infinite modes.
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    ENVIRONMENTAL NEEDS
 *      No run-time environmental needs.
 *
 *    DETAILED DESCRIPTION
 *
 *	Setup:
 *	  Setup signal handling.
 *        Make and change to a temporary directory.
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno.
 *        If doing functional test
 *            check the errno returned and print result message
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *        Remove the temporary directory and exit.
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <errno.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <sys/mman.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *get_high_address();

char *TCID = "access03";	/* Test program identifier.    */
int TST_TOTAL = 8;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int exp_enos[] = { EFAULT, 0 };	/* List must end with 0 */

char *bad_addr = 0;

#if !defined(UCLINUX)

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

	/* set the expected errnos. */
	TEST_EXP_ENOS(exp_enos);

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * TEST CASE:
		 *  R_OK on low pointer (-1) for path
		 */

		/* Call access(2) */
		TEST(access(bad_addr, R_OK));

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
						 "access((char *)-1,R_OK) failed as expected with errno %d (EFAULT) : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "access((char *)-1,R_OK) failed with errno %d : %s but expected %d (EFAULT)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EFAULT);
				}
			} else {
				tst_resm(TFAIL,
					 "access((char *)-1,R_OK) succeeded unexpectedly.");

			}
		}

		/*
		 * TEST CASE:
		 *  W_OK on low pointer (-1) for path
		 */

		/* Call access(2) */
		TEST(access(bad_addr, W_OK));

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
						 "access((char *)-1,W_OK) failed as expected with errno %d (EFAULT) : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "access((char *)-1,W_OK) failed with errno %d : %s but expected %d (EFAULT)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EFAULT);
				}
			} else {
				tst_resm(TFAIL,
					 "access((char *)-1,W_OK) succeeded unexpectedly.");

			}
		}

		/*
		 * TEST CASE:
		 *  X_OK on low pointer (-1) for path
		 */

		/* Call access(2) */
		TEST(access(bad_addr, X_OK));

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
						 "access((char*)-1,X_OK) failed as expected with errno %d (EFAULT) : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "access((char*)-1,X_OK) failed with errno %d : %s but expected %d (EFAULT)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EFAULT);
				}
			} else {
				tst_resm(TFAIL,
					 "access((char*)-1,X_OK) succeeded unexpectedly.");

			}
		}

		/*
		 * TEST CASE:
		 *  F_OK on low pointer (-1) for path
		 */

		/* Call access(2) */
		TEST(access(bad_addr, F_OK));

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
						 "access((char*)-1,F_OK) failed as expected with errno %d (EFAULT) : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "access((char*)-1,F_OK) failed with errno %d : %s but expected %d (EFAULT)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EFAULT);
				}
			} else {
				tst_resm(TFAIL,
					 "access((char*)-1,F_OK) succeeded unexpectedly.");

			}
		}

		/*
		 * TEST CASE:
		 *  R_OK on high pointer (sbrk(0)+1) for path
		 */

		/* Call access(2) */
		TEST(access(get_high_address(), R_OK));

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
						 "access((char*)sbrk(0)+1,R_OK) failed as expected with errno %d (EFAULT) : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "access((char*)sbrk(0)+1,R_OK) failed with errno %d : %s but expected %d (EFAULT)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EFAULT);
				}
			} else {
				tst_resm(TFAIL,
					 "access((char*)sbrk(0)+1,R_OK) succeeded unexpectedly.");

			}
		}

		/*
		 * TEST CASE:
		 *  W_OK on high pointer (sbrk(0)+1) for path
		 */

		/* Call access(2) */
		TEST(access(get_high_address(), W_OK));

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
						 "access((char*)sbrk(0)+1,W_OK) failed as expected with errno %d (EFAULT) : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "access((char*)sbrk(0)+1,W_OK) failed with errno %d : %s but expected %d (EFAULT)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EFAULT);
				}
			} else {
				tst_resm(TFAIL,
					 "access((char*)sbrk(0)+1,W_OK) succeeded unexpectedly.");

			}
		}

		/*
		 * TEST CASE:
		 *  X_OK on high pointer (sbrk(0)+1) for path
		 */

		/* Call access(2) */
		TEST(access(get_high_address(), X_OK));

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
						 "access(high_address,X_OK) failed as expected with errno %d (EFAULT) : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "access(high_address,X_OK) failed with errno %d : %s but expected %d (EFAULT)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EFAULT);
				}
			} else {
				tst_resm(TFAIL,
					 "access(high_address,X_OK) succeeded unexpectedly.");

			}
		}

		/*
		 * TEST CASE:
		 *  F_OK on high pointer (sbrk(0)+1) for path
		 */

		/* Call access(2) */
		TEST(access(get_high_address(), F_OK));

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
						 "access((char*)sbrk(0)+1,F_OK) failed as expected with errno %d (EFAULT) : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "access((char*)sbrk(0)+1,F_OK) failed with errno %d : %s but expected %d (EFAULT)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EFAULT);
				}
			} else {
				tst_resm(TFAIL,
					 "access((char*)sbrk(0)+1,F_OK) succeeded unexpectedly.");

			}
		}

	}			/* End for TEST_LOOPING */

    /***************************************************************
     * cleanup and exit
     ***************************************************************/
	cleanup();

	return 0;
}				/* End main */

#else

int main()
{
	tst_resm(TINFO, "test is not available on uClinux");
	return 0;
}

#endif /* if !defined(UCLINUX) */

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make and change to a temporary directory */
	tst_tmpdir();

	bad_addr =
	    mmap(0, 1, PROT_NONE, MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0,
		 0);
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

	/* remove the temporary directory and exit with
	   return code appropriate for results */
	tst_rmdir();
	tst_exit();
}				/* End cleanup() */
