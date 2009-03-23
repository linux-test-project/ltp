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
 * 	fork06.c
 *
 * DESCRIPTION
 *	Test that a process can fork children a large number of
 *	times in succession
 *
 * ALGORITHM
 *	Attempt to fork a child that exits immediately
 *	Repeat it many times(1000), counting the number of successes and
 *	failures
 *
 * USAGE
 * 	fork06
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 * 	None
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include "test.h"
#include "usctest.h"

char *TCID = "fork06";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);

#define NUMFORKS 1000

int main(int ac, char **av)
{
	int i, pid, status, childpid, succeed, fail;

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	/*
	 * perform global setup for the test
	 */
	setup();

	/*
	 * check looping state if -i option is given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/*
		 * reset Tst_count in case we are looping.
		 */
		Tst_count = 0;

		succeed = 0;
		fail = 0;

		for (i = 0; i < NUMFORKS; i++) {
			if ((pid = fork()) == -1) {
				fail++;
				continue;
			}

			if (pid == 0) {	/* child */
				_exit(0);
			}

			/* parent */
			childpid = wait(&status);
			if (pid != childpid) {
				tst_resm(TFAIL, "pid from wait %d", childpid);
			}
			succeed++;
		}

		tst_resm(TINFO, "tries %d", i);
		tst_resm(TINFO, "successes %d", succeed);
		tst_resm(TINFO, "failures %d", fail);

		if ((wait(&status)) == -1) {
			tst_resm(TINFO, "There were no children to wait for");
		} else {
			tst_resm(TINFO, "There were children left");
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup()
{
	/*
	 * capture signals
	 */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/*
	 * Pause if that option was specified
	 */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 * 	       completion or premature exit
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	tst_exit();
}
