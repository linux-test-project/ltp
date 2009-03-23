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
 *	sched_setscheduler01.c
 *
 * DESCRIPTION
 *	Testcase to test whether sched_setscheduler(2) sets the errnos
 *	correctly.
 *
 * ALGORITHM
 *	1.	Call sched_setscheduler as a non-root uid, and expect EPERM
 *	to be returned.
 *
 * USAGE:  <for command-line>
 *  sched_setscheduler02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	Must run test as root.
 */
#include <stdio.h>
#include <errno.h>
#include <sched.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

#define SCHED_INVALID	99
#define INVALID_PID	999999

char *TCID = "sched_setscheduler02";
int TST_TOTAL = 1;
extern int Tst_count;

int exp_enos[] = { EPERM, 0 };
extern struct passwd *my_getpwnam(char *);

void setup(void);
void cleanup(void);

char user1name[] = "nobody";

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	struct passwd *nobody;
	pid_t pid;
	struct sched_param param;
	int status;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();

	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork failed");
		}

		if (pid == 0) {	/* child */
			param.sched_priority = 1;

			nobody = my_getpwnam(user1name);

			if (seteuid(nobody->pw_uid) == -1) {
				tst_brkm(TBROK, cleanup, "seteuid() failed");
			}

			TEST(sched_setscheduler(pid, SCHED_FIFO, &param));

			if (TEST_ERRNO) {
				TEST_ERROR_LOG(TEST_ERRNO);
			}

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "sched_setscheduler(2) passed "
					 "with non root priveledges");
			} else if (TEST_ERRNO != EPERM) {
				tst_resm(TFAIL, "Expected EPERM, got %d",
					 TEST_ERRNO);
			} else {
				tst_resm(TPASS, "got EPERM");
			}
		} else {	/* parent */
			/* let the child carry on */
			wait(&status);
			if (WIFEXITED(status) != 0) {	/* Exit with errors */
				exit(WEXITSTATUS(status));
			} else {
				exit(0);
			}
		}

		if (seteuid(0) == -1) {
			tst_brkm(TBROK, cleanup, "seteuid(0) failed");
		}
	}
	cleanup();
	 /*NOTREACHED*/ return 0;

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* must run test as root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Must run test as root");
	}

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();

}
