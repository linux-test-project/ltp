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
/* $Id: rmdir05.c,v 1.8 2009/03/23 13:36:01 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: rmdir05
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Functionality Tests for rmdir(2)
 *
 *    PARENT DOCUMENT	: rmstds02
 *
 *    TEST CASE TOTAL	: 6
 *
 *    WALL CLOCK TIME	: 2
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: Bill Branum
 *
 *    CO-PILOT		: Steve Shaw
 *
 *    DATE STARTED	: 4/23/92
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *	rmdir(2) test for errno(s) EINVAL, EMLINK, EFAULT
 *
 *    INPUT SPECIFICATIONS
 *	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    ENVIRONMENTAL NEEDS
 *      No run-time environmental needs.
 *
 *    DETAILED DESCRIPTION
 *	Verify that rmdir(2) returns a value of -1 and sets errno
 *	to indicate the error.
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Create a temporary directory and make it current.
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
 *	  Print errno log and/or timing stats if options given.
 *	  Remove the temporary directory.
 *	  Exit.
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

#if !defined(UCLINUX)
extern char *get_high_address();
int TST_TOTAL = 6;
#else
int TST_TOTAL = 4;
#endif

char *TCID = "rmdir05";		/* Test program identifier.    */
extern int Tst_count;		/* Test Case counter for tst_* routines. */
struct stat stat_buf;		/* Stat buffer used for verification. */
char dir_name[256];		/* Array to hold directory name. */

char *bad_addr = 0;

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

    /***************************************************************
     * parse standard options
     ***************************************************************/
	if ((msg =
	     parse_opts(argc, argv, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * TEST CASE: 1
		 * path points to the current directory
		 */

		/* Call rmdir(2) */
		TEST(rmdir("."));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
		}

	/***************************************************************
	 * only perform functional verification if flag set (-f not given)
	 ***************************************************************/
		if (STD_FUNCTIONAL_TEST) {

			if (TEST_RETURN == -1) {
#if defined(sgi)
				if (TEST_ERRNO == EINVAL) {
#elif defined(linux)
				if (TEST_ERRNO & (EBUSY | ENOTEMPTY)) {
#endif

					/* For functionality tests, verify that the
					 * directory wasn't removed.
					 */
					if (stat(".", &stat_buf) == -1) {
						tst_resm(TFAIL,
							 "rmdir(\".\") removed the current working directory when it should have failed.");
					} else {
						tst_resm(TPASS,
							 "rmdir(\".\") failed to remove the current working directory. Returned %d : %s",
							 TEST_ERRNO,
							 strerror(TEST_ERRNO));
					}
				} else {
#if defined(sgi)
					tst_resm(TFAIL,
						 "rmdir(\".\") failed with errno %d : %s but expected %d (EINVAL)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EINVAL);
#elif defined(linux)
					tst_resm(TFAIL,
						 "rmdir(\".\") failed with errno %d : %s but expected %d (EBUSY)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EBUSY);
#endif
				}
			} else {
				tst_resm(TFAIL,
					 "rmdir(\".\") succeeded unexpectedly.");
			}
		}

		/*
		 * TEST CASE: 2
		 * path points to the "." (dot) entry of a directory
		 */
#if defined(linux)
		tst_resm(TCONF, "rmdir on \"dir/.\" supported on Linux");
#elif defined(sgi)
		/* Call rmdir(2) */
		TEST(rmdir("dir1/."));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
		}

	/***************************************************************
	 * only perform functional verification if flag set (-f not given)
	 ***************************************************************/
		if (STD_FUNCTIONAL_TEST) {

			if (TEST_RETURN == -1) {
				if (TEST_ERRNO == EINVAL) {

					/* For functionality tests, verify that the
					 * directory wasn't removed.
					 */
					if (stat("dir1/.", &stat_buf) == -1) {
						tst_resm(TFAIL,
							 "rmdir(\"dir1/.\") removed the \".\" entry of a directory when it should have failed.");
					} else {
						tst_resm(TPASS,
							 "rmdir(\"dir1/.\") failed to remove the \".\" entry of a directory. Returned %d : %s",
							 TEST_ERRNO,
							 strerror(TEST_ERRNO));
					}
				} else {
					tst_resm(TFAIL,
						 "rmdir(\"dir1/.\") failed with errno %d : %s but expected %d (EINVAL)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EINVAL);
				}
			} else {
				tst_resm(TFAIL,
					 "rmdir(\"dir1/.\") - path points to the \".\" entry of a directory succeeded unexpectedly.");
			}
		}
#endif

#if defined(sgi)
		/*
		 * TEST CASE: 3
		 * the directory has been linked
		 */
		tst_resm(TCONF, "linked directories not valid on IRIX");
#elif defined(linux)
		tst_resm(TCONF,
			 "linked directories test not implemented on Linux");
#elif defined(CRAY)

		/* Call rmdir(2) */
		TEST(rmdir("dir2"));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
		}

	/***************************************************************
	 * only perform functional verification if flag set (-f not given)
	 ***************************************************************/
		if (STD_FUNCTIONAL_TEST) {

			if (TEST_RETURN == -1) {
				if (TEST_ERRNO == EMLINK) {
					/* For functionality tests, verify that the directory wasn't
					 * removed.
					 */
					if (stat("dir2", &stat_buf) == -1) {
						tst_resm(TFAIL,
							 "rmdir(\"dir2\") removed a directory with multiple links when it should have failed.");
					} else {
						tst_resm(TPASS,
							 "rmdir(\"dir2\") failed to remove a directory with multiple links. Returned %d : %s",
							 TEST_ERRNO,
							 strerror(TEST_ERRNO));
					}
				} else {
					tst_resm(TFAIL,
						 "rmdir(\"dir2\") failed with errno %d : %s but expected %d (EMLINK)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EMLINK);
				}
			} else {
				tst_resm(TFAIL,
					 "rmdir(\"dir2\") - the directory has been linked succeeded unexpectedly.");
			}
		}
#endif /* linux */

		/*
		 * TEST CASE: 4
		 * path argument points below the minimum allocated address space
		 */

#if !defined(UCLINUX)
		/* Call rmdir(2) */
		TEST(rmdir(bad_addr));

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
						 "rmdir() - path argument points below the minimum allocated address space failed as expected with errno %d : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "rmdir() - path argument points below the minimum allocated address space failed with errno %d : %s but expected %d (EFAULT)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EFAULT);
				}
			} else {
				tst_resm(TFAIL,
					 "rmdir() - path argument points below the minimum allocated address space succeeded unexpectedly.");
			}
		}

		/*
		 * TEST CASE: 5
		 * path argument points above the maximum allocated address space
		 */

		/* Call rmdir(2) */
		TEST(rmdir(get_high_address()));

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
						 "rmdir() - path argument points above the maximum allocated address space failed as expected with errno %d : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "rmdir() - path argument points above the maximum allocated address space failed with errno %d : %s but expected %d (EFAULT)",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO), EFAULT);
				}
			} else {
				tst_resm(TFAIL,
					 "rmdir() - path argument points above the maximum allocated address space succeeded unexpectedly.");
			}
		}
#endif

		/*
		 * TEST CASE: 6
		 * able to remove a directory
		 */

		/* Create a directory. */
		if (mkdir(dir_name, 0777) != 0) {
			tst_brkm(TBROK, cleanup,
				 "mkdir(\"%s\") failed with errno %d : %s",
				 dir_name, errno, strerror(errno));
		}

		/* Call rmdir(2) */
		TEST(rmdir(dir_name));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "rmdir(\"%s\") failed when it should have passed. Returned %d : %s",
				 dir_name, TEST_ERRNO, strerror(TEST_ERRNO));
		} else {

	  /***************************************************************
	   * only perform functional verification if flag set (-f not given)
	   ***************************************************************/
			if (STD_FUNCTIONAL_TEST) {

				/* Verify the directory was removed. */
				if (stat(dir_name, &stat_buf) != 0) {
					tst_resm(TPASS,
						 "rmdir(\"%s\") removed the directory as expected.",
						 dir_name);
				} else {
					tst_resm(TFAIL,
						 "rmdir(\"%s\") returned a zero exit status but failed to remove the directory.",
						 dir_name);
				}
			}
		}

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
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	/* Create a directory. */
	if (mkdir("dir1", 0777) == -1) {
		tst_brkm(TBROK, cleanup, "mkdir() failed to create dir1.");
	}
#if defined(CRAY)
	/* NOTE: linking directories is NOT valid on IRIX  */

	/* Create a directory that has multiple links to it. */
	if (mkdir("dir2", 0777) == -1) {
		tst_brkm(TBROK, cleanup, "mkdir() failed to create dir2.");
	} else {
		if (system
		    ("runcmd `get_attrib -A link` dir2 mlink_dir > link.out 2>&1")
		    != 0) {
			tst_brk(TBROK, "link.out", cleanup,
				"link failed to link dir2 and mlink_dir.");
		}
	}

#endif

	/* Create a unique directory name. */
	sprintf(dir_name, "./dir_%d", getpid());

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
#endif
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

#if defined(CRAY)
	/* NOTE: setup was not done on IRIX */
	/* Unlink the directory. */
	if (system("runcmd `get_attrib -A unlink` dir2 > unlink.out 2>&1") != 0) {
		tst_res(TWARN, "unlink.out", "unlink failed to unlink dir2.");
	}
#endif

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();

	/*
	 * Exit with a return value appropriate for the results.
	 */
	tst_exit();

}				/* End cleanup() */
