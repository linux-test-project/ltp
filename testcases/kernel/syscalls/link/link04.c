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
/* $Id: link04.c,v 1.9 2009/10/26 14:55:47 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: link04
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Negative test cases for link(2).
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 14
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
 * 	1-14.) link(2) returns...(See Test_cases structure below)
 *
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    OUTPUT SPECIFICATIONS
 *	Standard tst_res output formt.
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
 *	This is a Phase I test for the link(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	link(2).
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
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/param.h>		/* for PATH_MAX */
#include <sys/mman.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

extern char *get_high_address();

char *TCID = "link04";		/* Test program identifier.    */
int TST_TOTAL = 14;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int exp_enos[] = { 0, 0 };

char *bad_addr = 0;

int longpath_setup();
int no_setup();
int filepath_setup();
int filepath2_setup();
char Longpathname[PATH_MAX + 2];
#if !defined(UCLINUX)
char High_address[64];
#endif
int dir_setup();

struct test_case_t {
	char *file1;
	char *desc1;
	char *file2;
	char *desc2;
	int exp_errno;
	int (*setupfunc1) ();
	int (*setupfunc2) ();
} Test_cases[] = {
	/* This test program should contain test cases where link */
	/* will fail regardless of who executed it (i.e. joe-user or root) */

	/* first path is invalid */

	{
	"nonexistfile", "non-existent file", "nefile", "nefile",
		    ENOENT, no_setup, no_setup}, {
	"", "path is empty string", "nefile", "nefile",
		    ENOENT, no_setup, no_setup}, {
	"neefile/file", "path contains a non-existent file", "nefile",
		    "nefile", ENOENT, no_setup, no_setup}, {
	"regfile/file", "path contains a regular file", "nefile",
		    "nefile", ENOTDIR, filepath_setup, no_setup}, {
	Longpathname, "pathname too long", "nefile", "nefile",
		    ENAMETOOLONG, longpath_setup, no_setup},
#if !defined(UCLINUX)
	{
	High_address, "address beyond address space", "nefile",
		    "nefile", EFAULT, no_setup, no_setup},
#endif
	{
	(char *)-1, "negative address", "nefile", "nefile",
		    EFAULT, no_setup, no_setup},
	    /* second path is invalid */
	{
	"regfile", "regfile", "", "empty string",
		    ENOENT, no_setup, no_setup}, {
	"regfile", "regfile", "neefile/file",
		    "path contains a non-existent file", ENOENT,
		    filepath_setup, no_setup}, {
	"regfile", "regfile", "file/file",
		    "path contains a regular file", ENOENT,
		    filepath_setup, no_setup}, {
	"regfile", "regfile", Longpathname, "pathname too long",
		    ENAMETOOLONG, no_setup, longpath_setup},
#if !defined(UCLINUX)
	{
	"regfile", "regfile", High_address,
		    "address beyond address space", EFAULT, no_setup, no_setup},
#endif
	{
	"regfile", "regfile", (char *)-1, "negative address",
		    EFAULT, no_setup, no_setup},
	    /* two existing files */
	{
	"regfile", "regfile", "regfile2", "regfile2",
		    EEXIST, filepath_setup, filepath2_setup}, {
	NULL, NULL, NULL, NULL, 0, no_setup, no_setup}
};

/***********************************************************************
 * Main
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *fname1, *fname2;
	char *desc1, *desc2;
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

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; Test_cases[ind].desc1 != NULL; ind++) {

			fname1 = Test_cases[ind].file1;
			desc1 = Test_cases[ind].desc1;
			fname2 = Test_cases[ind].file2;
			desc2 = Test_cases[ind].desc2;

#if !defined(UCLINUX)
			if (fname1 == High_address)
				fname1 = get_high_address();

			if (fname2 == High_address)
				fname2 = get_high_address();
#endif

			/*
			 *  Call link(2)
			 */
			TEST(link(fname1, fname2));

			/* check return code */
			if (TEST_RETURN == -1) {
				if (STD_FUNCTIONAL_TEST) {
					if (TEST_ERRNO ==
					    Test_cases[ind].exp_errno)
						tst_resm(TPASS,
							 "link(<%s>, <%s>) Failed, errno=%d",
							 desc1, desc2,
							 TEST_ERRNO);
					else
						tst_resm(TFAIL,
							 "link(<%s>, <%s>) Failed, errno=%d, expected errno:%d",
							 desc1, desc2,
							 TEST_ERRNO,
							 Test_cases[ind].
							 exp_errno);
				} else
					Tst_count++;
			} else {
				tst_resm(TFAIL,
					 "link(<%s>, <%s>) returned %ld, expected -1, errno:%d",
					 desc1, desc2, TEST_RETURN,
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

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	Test_cases[6].file1 = bad_addr;
	Test_cases[12].file2 = bad_addr;
#endif

	for (ind = 0; Test_cases[ind].desc1 != NULL; ind++) {
		Test_cases[ind].setupfunc1();
		Test_cases[ind].setupfunc2();
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

	static int alreadycalled = 0;

	if (alreadycalled)
		return 0;
	alreadycalled++;

	for (ind = 0; ind <= PATH_MAX + 1; ind++) {
		Longpathname[ind] = 'a';
	}
	return 0;

}

/******************************************************************
 *
 ******************************************************************/
int filepath2_setup()
{
	int fd;
	static int alreadycalled = 0;

	if (alreadycalled)
		return 0;
	alreadycalled++;

	if ((fd = creat("regfile2", 0777)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "creat(regfile2, 0777) failed, errno:%d %s", errno,
			 strerror(errno));
	}
	close(fd);
	return 0;
}

/******************************************************************
 *
 ******************************************************************/
int filepath_setup()
{
	int fd;
	static int alreadycalled = 0;

	if (alreadycalled)
		return 0;
	alreadycalled++;

	if ((fd = creat("regfile", 0777)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "creat(regfile, 0777) failed, errno:%d %s", errno,
			 strerror(errno));
	}
	close(fd);
	return 0;
}

/******************************************************************
 *
 ******************************************************************/
int dir_setup()
{
	static int alreadycalled = 0;

	if (alreadycalled)
		return 0;
	alreadycalled++;

	if (mkdir("dir", 0777) == -1) {
		tst_brkm(TBROK, cleanup,
			 "mkdir(dir, 0700) Failed, errno=%d : %s",
			 errno, strerror(errno));
	}

	return 0;
}
