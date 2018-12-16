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
/* $Id: pathconf01.c,v 1.5 2009/11/02 13:57:17 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: pathconf01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for pathconf(2)
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
 *	1.) pathconf(2) returns...(See Description)
 *
 *    INPUT SPECIFICATIONS
 *	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    OUTPUT SPECIFICATIONS
 *
 *    DURATION
 *	Terminates - with frequency and infinite modes.
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    RESOURCES
 *	None
 *
 *    ENVIRONMENTAL NEEDS
 *      No run-time environmental needs.
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 *	None
 *
 *    INTERCASE DEPENDENCIES
 *	None
 *
 *    DETAILED DESCRIPTION
 *	This is a Phase I test for the pathconf(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	pathconf(2).
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"

void setup();
void cleanup();
void help();

struct pathconf_args {
	char *define_tag;
	int value;
} args[] = {
	{
	"_PC_LINK_MAX", _PC_LINK_MAX}, {
	"_PC_NAME_MAX", _PC_NAME_MAX}, {
	"_PC_PATH_MAX", _PC_PATH_MAX}, {
	"_PC_PIPE_BUF", _PC_PIPE_BUF}, {
	"_PC_CHOWN_RESTRICTED", _PC_CHOWN_RESTRICTED}, {
	"_PC_NO_TRUNC", _PC_NO_TRUNC}, {
	NULL, 0}
};

char *TCID = "pathconf01";
int TST_TOTAL = ARRAY_SIZE(args);

int i;


int lflag;
char *path;

option_t options[] = {
	{"l:", &lflag, &path},	/* -l <path to test> */
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, options, &help);

	if (!lflag) {
		tst_tmpdir();
		path = tst_get_tmpdir();
	}
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

			errno = -4;

			/*
			 * Call pathconf(2)
			 */
			TEST(pathconf(path, args[i].value));

			/*
			 * A test case can only fail if -1 is returned and the errno
			 * was set.  If the errno remains unchanged, the
			 * system call did not fail.
			 */
			if (TEST_RETURN == -1 && errno != -4) {
				tst_resm(TFAIL,
					 "pathconf(%s, %s) Failed, errno=%d : %s",
					 path, args[i].define_tag, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TPASS,
					 "pathconf(%s, %s) returned %ld",
					 path, args[i].define_tag,
					 TEST_RETURN);
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
	if (!lflag) {
		tst_rmdir();
		free(path);
	}
}

/***************************************************************
 * help
 ***************************************************************/
void help(void)
{
	printf("  -l path a path to test with pathconf(2) (def: /tmp)\n");
}
