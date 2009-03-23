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
 *	vhangup01.c
 *
 * DESCRIPTION
 *	Check the return value, and errno of vhangup(2)
 *	when a non-root user calls vhangup().
 *
 * USAGE: <for command-line>
 *	vhangup01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	None
 */

#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <wait.h>
#include <test.h>
#include <usctest.h>

void setup(void);
void cleanup(void);

char *TCID = "vhangup01";
int TST_TOTAL = 1;

/* 0 terminated list of expected errnos */
int exp_enos[] = { EPERM, 0 };

int fail;
char user1name[] = "nobody";
extern struct passwd *my_getpwnam(char *);
extern int Tst_count;

int main(int argc, char **argv)
{
	int lc;
	char *msg;

	pid_t pid;
	int retval, status;

	struct passwd *nobody;

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

		nobody = my_getpwnam(user1name);

		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TFAIL, cleanup, "fork failed");
		 /*NOTREACHED*/} else if (pid > 0) {	/* parent */
			waitpid(pid, &status, 0);
			_exit(0);	/*
					 * Exit here and let the child clean up.
					 * This allows the errno information set
					 * by the TEST_ERROR_LOG macro and the
					 * PASS/FAIL status to be preserved for
					 * use during cleanup.
					 */

		} else {	/* child */
			retval = setreuid(nobody->pw_uid, nobody->pw_uid);
			if (retval < 0) {
				perror("setreuid");
				tst_brkm(TFAIL, cleanup, "setreuid failed");
			 /*NOTREACHED*/}
			TEST(vhangup());
			if (TEST_RETURN != -1) {
				tst_brkm(TFAIL, cleanup, "vhangup() failed to "
					 "fail");
			 /*NOTREACHED*/} else if (TEST_ERRNO == EPERM) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TPASS, "Got EPERM as expected.");
			} else {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL, "expected EPERM got %d",
					 TEST_ERRNO);
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
	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

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
