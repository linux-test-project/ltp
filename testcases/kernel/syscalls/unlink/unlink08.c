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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
/* $Id: unlink08.c,v 1.5 2009/11/02 13:57:19 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: unlink08
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: unlink(2) negative testcases
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 3
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
 * 	1-3) See Testcases structure below.
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
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "test.h"
#include "safe_macros.h"

void setup(void);
void cleanup(void);

char *TCID = "unlink08";
int TST_TOTAL = 3;

int unwrite_dir_setup(int flag);
int unsearch_dir_setup(int flag);
int dir_setup(int flag);
int no_setup(int flag);

struct test_case_t {
	char *pathname;
	char *desc;
	int (*setupfunc) (int flag);
	int exp_ret;		/* -1 means error, 0 means != -1 */
	int exp_errno;
} Test_cases[] = {
	{
	"unwrite_dir/file", "unwritable directory", unwrite_dir_setup,
		    -1, EACCES}, {
	"unsearch_dir/file", "unsearchable directory",
		    unsearch_dir_setup, -1, EACCES},
#ifdef linux
	{
	"regdir", "directory", dir_setup, -1, EISDIR},
#else
	{
	"regdir", "directory", dir_setup, -1, EPERM},
#endif
	{
	NULL, NULL, no_setup, -1, 0}
};

/***********************************************************************
 * Main
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;
	char *fname;
	char *desc;
	int ind;

    /***************************************************************
     * parse standard options
     ***************************************************************/
	tst_parse_opts(ac, av, NULL, NULL);

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {

			fname = Test_cases[ind].pathname;
			desc = Test_cases[ind].desc;

			/*
			 *  Call unlink(2)
			 */
			TEST(unlink(fname));

			/* check return code */
			if (TEST_RETURN == -1) {
				if (Test_cases[ind].exp_ret == -1) {	/* neg test */
					if (TEST_ERRNO ==
					    Test_cases[ind].exp_errno)
						tst_resm(TPASS,
							 "unlink(<%s>) Failed, errno=%d",
							 desc,
							 TEST_ERRNO);
					else
						tst_resm(TFAIL,
							 "unlink(<%s>) Failed, errno=%d, expected errno:%d",
							 desc,
							 TEST_ERRNO,
							 Test_cases
							 [ind].exp_errno);
				} else {
					tst_resm(TFAIL,
						 "unlink(<%s>) Failed, errno=%d",
						 desc, TEST_ERRNO);
				}
			} else {
				if (Test_cases[ind].exp_ret == -1) {
					tst_resm(TFAIL,
						 "unlink(<%s>) returned %ld, expected -1, errno:%d",
						 desc, TEST_RETURN,
						 Test_cases[ind].exp_errno);
				} else {
					tst_resm(TPASS,
						 "unlink(<%s>) returned %ld",
						 desc, TEST_RETURN);
				}
				Test_cases[ind].setupfunc(1);
			}
		}

	}

	cleanup();
	tst_exit();
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup(void)
{
	int ind;
	int postest = 0;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	if (geteuid() == 0) {
		postest++;
	}

	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		if (Test_cases[ind].exp_errno == EACCES && postest)
			Test_cases[ind].exp_ret = 0;	/* set as a pos test */
		Test_cases[ind].setupfunc(0);
	}

}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup(void)
{
	chmod("unwrite_dir", 0777);
	chmod("unsearch_dir", 0777);

	tst_rmdir();

}

/******************************************************************
 *
 ******************************************************************/
int unwrite_dir_setup(int flag)
{
	int fd;

	if (!flag) {		/* initial setup */
		SAFE_MKDIR(cleanup, "unwrite_dir", 0777);

		if ((fd = creat("unwrite_dir/file", 0777)) == -1) {
			tst_brkm(TBROK, cleanup,
				 "creat(unwrite_dir/file, 0777) failed, errno:%d %s",
				 errno, strerror(errno));
		}
		close(fd);

		SAFE_CHMOD(cleanup, "unwrite_dir", 0555);
	} else {		/* resetup */
		SAFE_CHMOD(cleanup, "unwrite_dir", 0777);

		if ((fd = creat("unwrite_dir/file", 0777)) == -1) {
			tst_brkm(TBROK, cleanup,
				 "creat(unwrite_dir/file, 0777) failed, errno:%d %s",
				 errno, strerror(errno));
		}
		close(fd);

		SAFE_CHMOD(cleanup, "unwrite_dir", 0555);
	}
	return 0;
}

/******************************************************************
 *
 ******************************************************************/
int unsearch_dir_setup(int flag)
{
	int fd;

	if (!flag) {		/* initial setup */
		SAFE_MKDIR(cleanup, "unsearch_dir", 0777);

		if ((fd = creat("unsearch_dir/file", 0777)) == -1) {
			tst_brkm(TBROK, cleanup,
				 "creat(unsearch_dir/file, 0777) failed, errno:%d %s",
				 errno, strerror(errno));
		}
		close(fd);

		SAFE_CHMOD(cleanup, "unsearch_dir", 0666);
	} else {		/* resetup */
		SAFE_CHMOD(cleanup, "unsearch_dir", 0777);

		if ((fd = creat("unsearch_dir/file", 0777)) == -1) {
			tst_brkm(TBROK, cleanup,
				 "creat(unsearch_dir/file, 0777) failed, errno:%d %s",
				 errno, strerror(errno));
		}
		close(fd);

		SAFE_CHMOD(cleanup, "unsearch_dir", 0666);
	}
	return 0;
}

/******************************************************************
 *
 ******************************************************************/
int dir_setup(int flag)
{
	SAFE_MKDIR(cleanup, "regdir", 0777);
	return 0;
}

/******************************************************************
 *
 ******************************************************************/
int no_setup(int flag)
{
	return 0;
}
