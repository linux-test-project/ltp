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
/* $Id: unlink07.c,v 1.8 2009/11/02 13:57:19 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: unlink07
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: unlink(2) negative testcases
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 1
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: Richard Logan
 *
 *    CO-PILOT		: William Roske
 *
 *    DATE STARTED	: 03/30/94
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 * 	1-8) See Testcases structure below.
 *
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    OUTPUT SPECIFICATIONS
 *$
 *    DURATION
 * 	Terminates - with frequency and infinite modes.
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    RESOURCES
 * 	None
 *
 *    ENVIRONMENTAL NEEDS
 *      No run-time environmental needs.
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 * 	None
 *
 *    INTERCASE DEPENDENCIES
 * 	None
 *
 *    DETAILED DESCRIPTION
 *	This is a Phase I test for the unlink(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	unlink(2).
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/param.h>		/* for PATH_MAX */
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

extern char *get_high_address();

char *TCID = "unlink07";	/* Test program identifier.    */
int TST_TOTAL = 6;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int exp_enos[] = { 0, 0 };

char *bad_addr = 0;

int longpath_setup();
int no_setup();
int filepath_setup();
char Longpathname[PATH_MAX + 2];
char High_address[64];

struct test_case_t {
	char *pathname;
	char *desc;
	int exp_errno;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	"nonexistfile", "non-existent file", ENOENT, no_setup}, {
	"", "path is empty string", ENOENT, no_setup}, {
	"nefile/file", "path contains a non-existent file",
		    ENOENT, no_setup},
#if !defined(UCLINUX)
	{
	High_address, "address beyond address space", EFAULT, no_setup},
#endif
	{
	"file/file", "path contains a regular file",
		    ENOTDIR, filepath_setup},
#if !defined(UCLINUX)
	{
	High_address, "address beyond address space", EFAULT, no_setup},
#endif
	{
	Longpathname, "pathname too long", ENAMETOOLONG, longpath_setup}, {
	(char *)-1, "negative address", EFAULT, no_setup}, {
	NULL, NULL, 0, no_setup}
};

/***********************************************************************
 * Main
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *fname;
	char *desc;
	int ind;

    /***************************************************************
     * parse standard options
     ***************************************************************/
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

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

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {

			fname = Test_cases[ind].pathname;
			desc = Test_cases[ind].desc;

#if !defined(UCLINUX)
			if (fname == High_address)
				fname = get_high_address();
#endif
			/*
			 *  Call unlink(2)
			 */
			TEST(unlink(fname));

			/* check return code */
			if (TEST_RETURN == -1) {
				if (STD_FUNCTIONAL_TEST) {
					if (TEST_ERRNO ==
					    Test_cases[ind].exp_errno)
						tst_resm(TPASS,
							 "unlink(<%s>) Failed, errno=%d",
							 desc, TEST_ERRNO);
					else
						tst_resm(TFAIL,
							 "unlink(<%s>) Failed, errno=%d, expected errno:%d",
							 desc, TEST_ERRNO,
							 Test_cases[ind].
							 exp_errno);
				} else
					Tst_count++;
			} else {
				tst_resm(TFAIL,
					 "unlink(<%s>) returned %ld, expected -1, errno:%d",
					 desc, TEST_RETURN,
					 Test_cases[ind].exp_errno);
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
	int ind;

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	bad_addr = mmap(0, 1, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	Test_cases[7].pathname = bad_addr;

	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		Test_cases[ind].setupfunc();
	}

}				/* End setup() */

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup()
{
	chmod("unwrite_dir", 0777);
	chmod("unsearch_dir", 0777);

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();

}				/* End cleanup() */

/******************************************************************
 *
 ******************************************************************/
int no_setup()
{
	return 0;
}

/******************************************************************
 *
 ******************************************************************/
int longpath_setup()
{
	int ind;

	for (ind = 0; ind <= PATH_MAX + 1; ind++) {
		Longpathname[ind] = 'a';
	}
	return 0;

}

/******************************************************************
 *
 ******************************************************************/
int filepath_setup()
{
	int fd;

	if ((fd = creat("file", 0777)) == -1) {
		tst_brkm(TBROK, cleanup, "creat(file) failed, errno:%d %s",
			 errno, strerror(errno));
	}
	close(fd);
	return 0;
}
