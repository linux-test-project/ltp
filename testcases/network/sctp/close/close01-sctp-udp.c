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
 * 	close01-sctp-udp.c
 *
 * DESCRIPTION
 * 	Check that an invalid file descriptor returns EBADF
 *
 * ALGORITHM
 *	loop if that option is specified
 * 	   call close using the TEST macro and passing in an invalid fd
 *	   if the call succeedes
 *	      issue a FAIL message
 *	   else
 *	      log the errno
 *	      if the errno == EBADF
 *	         issue a PASS message
 *	      else
 *	         issue a FAIL message
 *	cleanup
 *
 * USAGE:  <for command-line>
 *  close01-sctp-udp [-i n] [-P x]
 *     where, -i n : Execute test n times.
 *            -P x : Pause for x seconds between iterations.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	05/2002 Ported for SCTP by Mingqin Liu
 *
 * RESTRICTIONS
 * 	None
 */

#include <stdio.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

void cleanup(void);
void setup(void);

char *TCID = "close01-sctp-udp()";
int TST_TOTAL = 1;
extern int Tst_count;

void main(int ac, char **av)
{
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();			/* global setup */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(close(-1));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "Closed a non existent fildes");
		} 
		else {
			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO != EBADF) {
				tst_resm(TFAIL, "close() FAILED to set errno "
					 "to EBADF on an invalid fd, got %d",
					 errno);
			} 
			else {
				tst_resm(TPASS, "call returned EBADF");
			}
		}
	}
	cleanup();

} /* main() */


/*
 * setup() - performs all ONE TIME setup for this test
 */
void
setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * or premature exit.
 */
void
cleanup(void)
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
