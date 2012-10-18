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
/* $Id: link03.c,v 1.5 2009/10/26 14:55:47 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: link03
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: multi links tests
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 2
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: Richard Logan
 *
 *    CO-PILOT		: William Roske
 *
 *    DATE STARTED	: 03/31/94
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 * 	1.) link(2) returns...(See Description)
 *
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *	-N #links : Use #links links every iteration
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
#include "test.h"
#include "usctest.h"

void setup();
void help();
void cleanup();

char *TCID = "link03";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */

int exp_enos[] = { 0, 0 };

#define BASENAME	"lkfile"

char Basename[255];
char Fname[255];
int Nlinks = 0;
char *Nlinkarg;

int Nflag = 0;

/* for test specific parse_opts options */
option_t options[] = {
	{"N:", &Nflag, &Nlinkarg},	/* -N #links */
	{NULL, NULL, NULL}
};

/***********************************************************************
 * Main
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	struct stat fbuf, lbuf;
	int cnt;
	int nlinks;
	char lname[255];


    /***************************************************************
     * parse standard options
     ***************************************************************/
	if ((msg = parse_opts(ac, av, options, &help)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	if (Nflag) {
		if (sscanf(Nlinkarg, "%i", &Nlinks) != 1) {
			tst_brkm(TBROK, NULL, "--N option arg is not a number");
			tst_exit();
		}
		if (Nlinks > 1000) {
			tst_resm(TWARN,
				 "--N option arg > 1000 - may get errno:%d (EMLINK)",
				 EMLINK);
		}
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

		Tst_count = 0;

		if (Nlinks)
			nlinks = Nlinks;
		else
			/* min of 10 links and max of a 100 links */
			nlinks = (lc % 90) + 10;

		for (cnt = 1; cnt < nlinks; cnt++) {

			sprintf(lname, "%s%d", Basename, cnt);
			/*
			 *  Call link(2)
			 */
			TEST(link(Fname, lname));

			/* check return code */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_brkm(TFAIL, cleanup,
					 "link(%s, %s) Failed, errno=%d : %s",
					 Fname, lname, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			}
		}

	/***************************************************************
	 * only perform functional verification if flag set (-f not given)
	 ***************************************************************/
		if (STD_FUNCTIONAL_TEST) {
			stat(Fname, &fbuf);

			for (cnt = 1; cnt < nlinks; cnt++) {
				sprintf(lname, "%s%d", Basename, cnt);

				stat(lname, &lbuf);
				if (fbuf.st_nlink <= 1 || lbuf.st_nlink <= 1 ||
				    (fbuf.st_nlink != lbuf.st_nlink)) {

					tst_resm(TFAIL,
						 "link(%s, %s[1-%d]) ret %ld for %d files, stat values do not match %d %d",
						 Fname, Basename, nlinks,
						 TEST_RETURN, nlinks,
						 fbuf.st_nlink, lbuf.st_nlink);
					break;
				}
			}
			if (cnt >= nlinks) {
				tst_resm(TPASS,
					 "link(%s, %s[1-%d]) ret %ld for %d files, stat linkcounts match %d",
					 Fname, Basename, nlinks, TEST_RETURN,
					 nlinks, fbuf.st_nlink);
			}
		} else
			Tst_count++;

		for (cnt = 1; cnt < nlinks; cnt++) {

			sprintf(lname, "%s%d", Basename, cnt);

			if (unlink(lname) == -1) {
				tst_res(TWARN,
					"unlink(%s) Failed, errno=%d : %s",
					Fname, errno, strerror(errno));
			}
		}

	}

    /***************************************************************
     * cleanup and exit
     ***************************************************************/
	cleanup();

	tst_exit();
}

/***************************************************************
 * help
 ***************************************************************/
void help()
{
	printf("  -N #links : create #links hard links every iteration\n");
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	int fd;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(Fname, "%s_%d", BASENAME, getpid());
	if ((fd = open(Fname, O_RDWR | O_CREAT, 0700)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT,0700) Failed, errno=%d : %s",
			 Fname, errno, strerror(errno));
	} else if (close(fd) == -1) {
		tst_res(TWARN, "close(%s) Failed, errno=%d : %s",
			Fname, errno, strerror(errno));
	}
	sprintf(Basename, "%s_%d.", BASENAME, getpid());
}

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

	tst_rmdir();

}
