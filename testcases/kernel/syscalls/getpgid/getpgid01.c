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
 * 	getpgid01.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of getpgid().
 *
 * ALGORITHM
 * 	block1: Does getpgid(0), and checks for error.
 * 	block2: Does getpgid(getpid()) and checks for error.
 * 	block3: Does getpgid(getppid()) and checks for error.
 * 	block4: Verifies that getpgid(getpgid(0)) == getpgid(0).
 * 	block5: Does getpgid(1) and checks for error.
 *
 * USAGE
 * 	getpgid01
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	Expects that there are no EPERM limitations on getting the
 *	process group ID from proc 1 (init).
 */
#define _GNU_SOURCE 1

#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <wait.h>
#include <sys/types.h>
#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);

char *TCID = "getpgid01";
int TST_TOTAL = 1;
extern int Tst_count;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned by parse_opts */

	register int pgid_0, pgid_1;
	register int my_pid, my_ppid;
	int ex_stat, fail = 0;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if ((pgid_0 = FORK_OR_VFORK()) < 0) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}
		if (pgid_0 > 0) {	/* parent */
			/*
			 * parent process waits for child to exit, and then
			 * exits itself.
			 */
			while ((pgid_0 = wait(&ex_stat)) > 0) ;

			if (WEXITSTATUS(ex_stat) == 0) {
				tst_resm(TINFO, "%s PASSED", TCID);
			} else {
				tst_resm(TINFO, "%s FAILED", TCID);
			}

			/* let the child carry on */
			exit(0);
		}

		/* child */
//block1:
		tst_resm(TINFO, "Enter block 1");
		fail = 0;
		if ((pgid_0 = getpgid(0)) < 0) {
			perror("getpgid");
			tst_resm(TFAIL, "getpgid(0) failed");
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "Test block 1: getpgid(0) FAILED");
		} else {
			tst_resm(TPASS, "Test block 1: getpgid(0) PASSED");
		}
		tst_resm(TINFO, "Exit block 1");

//block2:
		tst_resm(TINFO, "Enter block 2");
		fail = 0;

		my_pid = getpid();
		if ((pgid_1 = getpgid(my_pid)) < 0) {
			perror("getpgid");
			tst_resm(TFAIL, "getpgid(my_pid=%d) failed", my_pid);
			tst_exit();
		}
		if (pgid_0 != pgid_1) {
			tst_resm(TFAIL, "getpgid(my_pid=%d) != getpgid(0) "
				 "[%d != %d]", my_pid, pgid_1, pgid_0);
			fail = 1;
		}
		if (fail) {
			tst_resm(TINFO, "Test block 2: getpgid(getpid()) "
				 "FAILED");
		} else {
			tst_resm(TPASS, "Test blcok 2: getpgid(getpid()) "
				 "PASSED");
		}
		tst_resm(TINFO, "Exit block 2");

//block3:
		tst_resm(TINFO, "Enter block 3");
		fail = 0;

		my_ppid = getppid();
		if ((pgid_1 = getpgid(my_ppid)) < 0) {
			perror("getpgid");
			tst_resm(TFAIL, "getpgid(my_ppid=%d) failed", my_ppid);
			tst_exit();
		}
		if (pgid_0 != pgid_1) {
			tst_resm(TFAIL, "getpgid(my_ppid=%d) != getpgid(0) "
				 "[%d != %d]", my_ppid, pgid_1, pgid_0);
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "Test block 3: getpgid(getppid()) "
				 "FAILED");
		} else {
			tst_resm(TPASS, "Test block 3: getpgid(getppid()) "
				 "PASSED");
		}
		tst_resm(TINFO, "Exit block 3");

//block4:
		tst_resm(TINFO, "Enter block 4");
		fail = 0;

		if ((pgid_1 = getpgid(pgid_0)) < 0) {
			perror("getpgid");
			tst_resm(TFAIL, "getpgid(my_pgid=%d) failed", pgid_0);
			tst_exit();
		}
		if (pgid_0 != pgid_1) {
			tst_resm(TFAIL, "getpgid(my_pgid=%d) != getpgid(0) "
				 "[%d != %d]", pgid_0, pgid_1, pgid_0);
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "Test block 4: getpgid(1) FAILED");
		} else {
			tst_resm(TPASS, "Test block 4: getpgid(1) PASSED");
		}
		tst_resm(TINFO, "Exit block 4");

//block5:
		tst_resm(TINFO, "Enter block 5");
		fail = 0;

		if (getpgid(1) < 0) {
			perror("getpgid");
			tst_resm(TFAIL, "getpgid(1) failed");
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "Test block 5: getpgid(1) FAILED");
		} else {
			tst_resm(TPASS, "Test block 5: getpgid(1) PASSED");
		}
		tst_resm(TINFO, "Exit block 5");
	}
	cleanup();
	return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
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
