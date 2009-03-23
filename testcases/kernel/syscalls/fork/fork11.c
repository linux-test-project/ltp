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
 * 	fork11.c
 *
 * DESCRIPTION
 *	Test that parent gets a pid from each child when doing wait
 *
 * ALGORITHM
 *	Fork NUMFORKS children that do nothing.
 *
 * USAGE
 * 	fork11
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
#include <errno.h>
#include "test.h"
#include "usctest.h"

char *TCID = "fork11";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);

#define NUMFORKS 100

int main(int ac, char **av)
{
	int i, pid, cpid, status;
	int fail = 0;
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

		for (i = 0; i < NUMFORKS; i++) {
			if ((pid = fork()) == 0) {	/* child */
				exit(0);
			}

			if (pid > 0) {	/* parent */
				cpid = wait(&status);
				if (cpid != pid) {
					fail++;
				}
			} else {
				fail++;
				break;
			}
		}
		if (fail) {
			tst_resm(TFAIL, "fork failed %d times", fail);
		} else {
			tst_resm(TPASS, "fork test passed, %d processes", i);
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
 *	       completion or premature exit
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
