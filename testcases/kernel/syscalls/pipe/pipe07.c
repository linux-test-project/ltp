/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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
 *	pipe07.c
 *
 * DESCRIPTION
 * 	Test the ability of pipe to open the maximum even number of file
 * 	descriptors permitted (or (maxfds - 3)/2 pipes)
 *
 * ALGORITHM
 * 	1. open pipes until EMFILE is returned
 * 	2. check to see that the number of pipes opened is
 * 	   (maxfds - 3) / 2
 *
 * USAGE:  <for command-line>
 *  pipe07 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	12/2002 Ported by Paul Larson
 *
 * RESTRICTIONS
 *	None
 */
#include <fcntl.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

char *TCID = "pipe07";
int TST_TOTAL = 1;
extern int Tst_count;

int exp_enos[] = {EMFILE, 0};

int pipe_ret, pipes[2];
void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */
	int min;			/* number of file descriptors */
	int npipes;			/* number of pipes opened */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
		/*NOTREACHED*/
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		min = getdtablesize();
		/* subtract 3 for std files */
		min -= 3;

		for (npipes = 0; ; npipes++) {
			pipe_ret = pipe(pipes);
			if (pipe_ret < 0) {
				if (errno != EMFILE) {
					tst_brkm(TFAIL, cleanup, 
						"got unexpected error - %d", 
						errno);
				}
				break;
			}
		}
		if (npipes == (min / 2))
			tst_resm(TPASS, "Opened maxfds/2 pipes");
		else
			tst_resm(TFAIL, "Unable to open maxfds/2 pipes");

	}
	cleanup();

	/* NOT REACHED */
	return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void
setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void
cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
