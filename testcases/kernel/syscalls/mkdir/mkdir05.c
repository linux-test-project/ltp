/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	mkdir05
 *
 * DESCRIPTION
 *	This test will verify the mkdir(2) syscall basic functionality
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling
 *		Pause for SIGUSR1 if option specified
 *		Create temporary directory
 *
 *	Test:
 *		Loop if the proper options are given
 *              call mkdir() using the TEST macro
 *		if the call fails, issue FAIL message and contine
 *		if STD_FUNCTIONAL_TEST
 *		   stat the directory
 *		   check for correct owner and group ID
 *		   if these are correct
 *		      issue a PASS message
 *		   else
 *		      issue a FAIL message
 *              else
 *		   issue a PASS message
 *
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created
 * USAGE
 *	mkdir05 [-c n] [-f] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None.
 */

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

#define PERMS		0777

char *TCID = "mkdir05";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

char tstdir1[100];

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	struct stat buf;

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	/*
	 * perform global setup for test
	 */
	setup();

	/*
	 * check looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * TEST mkdir() base functionality
		 */

		/* Initialize the test directory name */
		sprintf(tstdir1, "tstdir1.%d", getpid());

		/* Call mkdir(2) using the TEST macro */
		TEST(mkdir(tstdir1, PERMS));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "mkdir(%s, %#o) Failed",
				 tstdir1, PERMS);
			continue;
		}

		if (STD_FUNCTIONAL_TEST) {
			if (stat(tstdir1, &buf) == -1) {
				tst_brkm(TBROK, cleanup, "failed to stat the "
					 "new directory");
			 /*NOTREACHED*/}
			/* check the owner */
			if (buf.st_uid != geteuid()) {
				tst_resm(TFAIL, "mkdir() FAILED to set owner ID"
					 " as process's effective ID");
				continue;
			}
			/* check the group ID */
			if (buf.st_gid != getegid()) {
				tst_resm(TFAIL, "mkdir() failed to set group ID"
					 " as the process's group ID");
				continue;
			}
			tst_resm(TPASS, "mkdir() functionality is correct");
		} else {
			tst_resm(TPASS, "call succeeded");
		}

		/* clean up things in case we are looping */
		if (rmdir(tstdir1) == -1) {
			tst_brkm(TBROK, cleanup, "could not remove directory");
		}

	}			/* End for TEST_LOOPING */

	/*
	 * cleanup and exit
	 */
	cleanup();
	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();

	/*
	 * Exit with return code appropriate for results.
	 */
	tst_exit();
}
