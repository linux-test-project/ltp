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
/* $Id: fpathconf01.c,v 1.6 2009/10/26 14:55:47 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: fpathconf01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for fpathconf(2)
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 7
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
 * 	1.) fpathconf(2) returns...(See Description)
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
 *	This is a Phase I test for the fpathconf(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	fpathconf(2).
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

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "fpathconf01";	/* Test program identifier.    */

#define FILENAME	"fpafile01"

int exp_enos[] = { 0, 0 };

int i;

struct pathconf_args {
	char *define_tag;
	int value;
	int defined;		/* Some of these are undefined on regular files.
				 * Cancer does a slightly better job with these already,
				 * so this is all I'll do to this test.  11/19/98 roehrich
				 */
} args[] = {
	{
	"_PC_MAX_CANON", _PC_MAX_CANON, 0}, {
	"_PC_MAX_INPUT", _PC_MAX_INPUT, 0}, {
	"_PC_VDISABLE", _PC_VDISABLE, 0}, {
	"_PC_LINK_MAX", _PC_LINK_MAX, 1}, {
	"_PC_NAME_MAX", _PC_NAME_MAX, 1}, {
	"_PC_PATH_MAX", _PC_PATH_MAX, 1}, {
	"_PC_PIPE_BUF", _PC_PIPE_BUF, 0}
};

int TST_TOTAL = (sizeof(args) / sizeof(args[0]));
int fd = -1;			/* temp file for fpathconf */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			TEST(fpathconf(fd, args[i].value));

			if (TEST_RETURN == -1 && args[i].defined) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL|TTERRNO,
				    "fpathconf(fd, %s) failed",
				    args[i].define_tag);
			} else {
				if (STD_FUNCTIONAL_TEST) {
					tst_resm(TPASS,
						 "fpathconf(fd, %s) returned %ld",
						 args[i].define_tag,
						 TEST_RETURN);
				}
			}
		}
	}

	cleanup();

	tst_exit();
}

void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	if ((fd = open(FILENAME, O_RDWR | O_CREAT, 0700)) == -1)
		tst_brkm(TBROK, cleanup, "Unable to open temp file %s!",
			 FILENAME);

}

void cleanup()
{
	TEST_CLEANUP;

	if (fd != -1) {
		if (close(fd) == -1)
			tst_resm(TWARN|TERRNO, "close failed");
		fd = -1;
	}

	tst_rmdir();

}
