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
 */
/* $Id: ulimit01.c,v 1.6 2009/11/02 13:57:19 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: ulimit01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for ulimit(2)
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 6
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: William Roske
 *
 *    CO-PILOT		: Dave Fenner
 *
 *    DATE STARTED	: 03/30/92
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 * 	1.) ulimit(2) returns...(See Description)
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
 * 	The libcuts.a and libsys.a libraries must be included in
 *	the compilation of this test.
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 * 	None
 *
 *    INTERCASE DEPENDENCIES
 * 	None
 *
 *    DETAILED DESCRIPTION
 *	This is a Phase I test for the ulimit(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	ulimit(2).
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

#include <ulimit.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"

void setup();
void cleanup();

char *TCID = "ulimit01";
int TST_TOTAL = 6;

int cmd;
long limit;			/* saved limit */

struct limits_t {
	int cmd;
	long newlimit;
	int nlim_flag;		/* special flag for UL_SETFSIZE records  */
	int exp_fail;
} Scenarios[] = {

	{
	UL_GETFSIZE, -1, 0, 0}, {
	UL_SETFSIZE, -2, 1, 0},	/* case case: must be after UL_GETFSIZE */
	{
	UL_SETFSIZE, -2, 2, 0},	/* case case: must be after UL_GETFSIZE */
#if UL_GMEMLIM
	{
	UL_GMEMLIM, -1, 0, 0},
#endif
#if UL_GDESLIM
	{
	UL_GDESLIM, -1, 0, 0},
#endif
#if UL_GSHMEMLIM
	{
	UL_GSHMEMLIM, -1, 0, 0},
#endif
};

int main(int ac, char **av)
{
	int lc;
	int i;
	int tmp;

	TST_TOTAL = sizeof(Scenarios) / sizeof(struct limits_t);

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

		for (i = 0; i < TST_TOTAL; i++) {

			cmd = Scenarios[i].cmd;
			limit = Scenarios[i].newlimit;

			/*
			 * Call ulimit(2)
			 */
			TEST(ulimit(cmd, limit));

			/* check return code */
			if (TEST_RETURN == -1) {
				if (Scenarios[i].exp_fail) {
					tst_resm(TPASS | TTERRNO,
						 "ulimit(%d, %ld) Failed expectedly",
						 cmd, limit);
				} else {
					tst_resm(TFAIL | TTERRNO,
						 "ulimit(%d, %ld) Failed",
						 cmd, limit);
				}
			} else {
				if (Scenarios[i].exp_fail) {
					tst_resm(TFAIL,
						 "ulimit(%d, %ld) returned %ld, succeeded unexpectedly",
						 cmd, limit, TEST_RETURN);
				} else {
					tst_resm(TPASS,
						 "ulimit(%d, %ld) returned %ld",
						 cmd, limit, TEST_RETURN);
				}

				/*
				 * Save the UL_GETFSIZE return value in the newlimit field
				 * for UL_SETFSIZE test cases.
				 */
				if (cmd == UL_GETFSIZE) {
					for (tmp = i + 1; tmp < TST_TOTAL;
					     tmp++) {
						if (Scenarios[tmp].nlim_flag ==
						    1) {
							Scenarios[tmp].newlimit
							    = TEST_RETURN;
						}
						if (Scenarios[tmp].nlim_flag ==
						    2) {
							Scenarios[tmp].newlimit
							    = TEST_RETURN - 1;
						}
					}
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
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup(void)
{

}
