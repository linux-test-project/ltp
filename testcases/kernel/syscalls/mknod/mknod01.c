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
/* $Id: mknod01.c,v 1.5 2009/11/02 13:57:17 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: mknod01
 *
 *    EXECUTED BY	: root
 *
 *    TEST TITLE	: Basic test for mknod(2)
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 4
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: William Roske
 *
 *    CO-PILOT		: Dave Fenner
 *
 *    DATE STARTED	: 05/13/92
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 * 	1.) mknod(2) returns...(See Description)
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
 *	This is a Phase I test for the mknod(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	mknod(2).
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

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "mknod01";		/* Test program identifier.    */
int TST_TOTAL;			/* Total number of test cases. */

char Path[1024];		/* path to create */
int i;
int tcases[] = {		/* modes to give nodes created (1 per text case) */
	S_IFREG | 0777,		/* ordinary file with mode 0777 */
	S_IFIFO | 0777,		/* fifo special with mode 0777 */
	S_IFCHR | 0777,		/* character special with mode 0777 */
	S_IFBLK | 0777,		/* block special with mode 0777 */

	S_IFREG | 04700,	/* ordinary file with mode 04700 (suid) */
	S_IFREG | 02700,	/* ordinary file with mode 02700 (sgid) */
	S_IFREG | 06700,	/* ordinary file with mode 06700 (sgid & suid) */

#ifdef CRAY
	S_IFDIR | 0777,		/* Direcory */
	S_IRESTART | 0400,	/* restartbit  */
#ifdef S_IFOFD
	S_IFOFD | 0777,		/* off line, with data  */
#endif
#ifdef S_IFOFL
	S_IFOFL | 0777,		/* off line, with no data   */
#endif
#endif /* CRAY */

};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	TST_TOTAL = (sizeof(tcases) / sizeof(tcases[0]));
    /***************************************************************
     * parse standard options
     ***************************************************************/
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
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

		Tst_count = 0;

		/*
		 * TEST CASES:
		 *  Make nodes in tcases array
		 */
		for (i = 0; i < TST_TOTAL; i++) {
			/* Call mknod(2) */
			TEST(mknod(Path, tcases[i], 0));

			/* check return code */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL,
					 "mknod(%s, %#o, 0) failed, errno=%d : %s",
					 Path, tcases[i], TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
		/***************************************************************
		 * only perform functional verification if flag set (-f not given)
		 ***************************************************************/
				if (STD_FUNCTIONAL_TEST) {
					/* No Verification test, yet... */
					tst_resm(TPASS,
						 "mknod(%s, %#o, 0) returned %ld",
						 Path, tcases[i], TEST_RETURN);
				}
			}

			/* remove the node for the next go `round */
			if (unlink(Path) == -1) {
				if (rmdir(Path) == -1) {
					tst_resm(TWARN,
						 "unlink(%s) & rmdir(%s) failed, errno:%d %s",
						 Path, Path, errno,
						 strerror(errno));
				}
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
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temp dir and cd to it */
	tst_tmpdir();

	/* Check that user is root */
	if (geteuid() != 0)
		tst_brkm(TBROK, cleanup, "Must be root for this test!");

	/* build a temp node name to bre created my mknod */
	sprintf(Path, "./tnode_%d", getpid());
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
