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
 *	vhangup02.c
 *
 * DESCRIPTION
 * 	To test the basic functionality of vhangup(2)
 *
 *
 * USAGE:  <for command-line>
 *      vhangup02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	None
 */

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <wait.h>
#include <test.h>
#include <usctest.h>

void setup(void);
void cleanup(void);

char *TCID = "vhangup02";
int TST_TOTAL = 1;
extern int Tst_count;

int fail;

int main(int argc, char **argv)
{
	int lc;
	char *msg;

	pid_t pid, pid1;
	int status;

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		fail = 0;

		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TFAIL, cleanup, "fork failed");
		 /*NOTREACHED*/} else if (pid > 0) {	/* parent */
			waitpid(pid, &status, 0);
			_exit(0);
		} else {	/* child */
			pid1 = setsid();
			if (pid1 < 0) {
				tst_brkm(TFAIL, cleanup, "setsid failed");
			 /*NOTREACHED*/}
			TEST(vhangup());
			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "vhangup() failed, errno:%d",
					 errno);
			} else {
				tst_resm(TPASS, "vhangup() succeeded");
			}
		}
	}
	cleanup();
	 /*NOTREACHED*/ return 0;

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
 /*NOTREACHED*/}
