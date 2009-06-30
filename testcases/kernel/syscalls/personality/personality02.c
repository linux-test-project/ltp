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
 *	personality02.c
 *
 * DESCRIPTION
 *	personality02 - Check that we don't get EINVAL for a bad personality.
 *
 * CALLS
 *	personality()
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call
 *	check the errno value
 *	  issue a FAIL message if we get EINVAL
 *	otherwise, the tests passes
 *	  issue a PASS message
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  personality02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	02/2003 - inverted by Paul Larson
 *		  It appears that personality() should NEVER return
 *		  EINVAL unless something went horribly wrong. Changed
 *		  the test to reflect this behaviour.
 *
 * RESTRICTIONS
 *	none
 *
 * NOTES
 *	It appears that the personality() call will always be
 *	successful with the way it is implemented in the kernel.
 *	This behavior differs from that described in the man page.
 */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <linux/personality.h>
#undef personality

extern int personality(unsigned long);

void cleanup(void);
void setup(void);

char *TCID = "personality02";
int TST_TOTAL = 1;
extern int Tst_count;

#define	PER_BAD	0x00dd		/* A non-existent personality type */

#ifdef __NR_personality
int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int start_pers;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	start_pers = personality(PER_LINUX);
	if (start_pers == -1) {
		printf("personality01:  Test Failed\n");
		exit(-1);
	}

	/* The following checks the looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(personality(PER_BAD));

		if (TEST_RETURN != 0) {
			tst_brkm(TFAIL, cleanup, "call failed - errno = %d "
				 "- %s", TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS, "call to personality() with a "
				 "bad personality passed");
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		/*
		 * set our personality back to PER_LINUX
		 */
		if (personality(start_pers) == -1) {
			tst_brkm(TBROK, cleanup, "personality reset failed");
		}
	}

	cleanup();

	 /*NOTREACHED*/ return 0;
}
#else
int main(int ac, char **av)
{
	tst_resm(TCONF, "personality() not defined in your system");
	tst_exit();
}
#endif


/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
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
}
