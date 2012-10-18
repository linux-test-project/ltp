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
 * 	getppid02.c
 *
 * DESCRIPTION
 * 	Testcase to check the basic functionality of the getppid() syscall.
 *
 * USAGE:  <for command-line>
 *  getppid02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 * 	None
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

char *TCID = "getppid02";
int TST_TOTAL = 1;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{

	int lc;			/* loop counter */
	int status;
	pid_t pid, ppid;
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		ppid = getpid();
		pid = FORK_OR_VFORK();
		if (pid == -1)
			tst_brkm(TBROK, cleanup, "fork failed");

		if (pid == 0) {
			TEST(getppid());

			if (STD_FUNCTIONAL_TEST) {
				if (TEST_RETURN != ppid)
					errx(1, "getppid failed (%ld != %d)",
					    TEST_RETURN, ppid);
				else
					printf("return value and parent's pid "
					    "value match\n");
			} else
				tst_resm(TPASS, "call succeeded");
			exit(0);
		} else {
			if (wait(&status) == -1)
				tst_brkm(TBROK|TERRNO, cleanup, "wait failed");
			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
				tst_resm(TFAIL,
				    "getppid functionality incorrect");
		}
	}
	cleanup();

	tst_exit();
}

void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

void cleanup()
{
	TEST_CLEANUP;

}
