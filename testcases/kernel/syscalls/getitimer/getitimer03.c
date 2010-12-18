/*
 * $Copyright: $
 * Copyright (c) 1984-2000
 * Sequent Computer Systems, Inc.   All rights reserved.
 *$
 * This software is furnished under a license and may be used
 * only in accordance with the terms of that license and with the
 * inclusion of the above copyright notice.   This software may not
 * be provided or otherwise made available to, or used by, any
 * other person.  No title to or ownership of the software is
 * hereby transferred.
 */

/* $Header: /cvsroot/ltp/ltp/testcases/kernel/syscalls/getitimer/getitimer03.c,v 1.7 2009/08/28 10:18:24 vapier Exp $ */

/*
 * NAME
 *	getitimer03.c
 *
 * DESCRIPTION
 *	getitimer03 - check that a getitimer() call fails as expected
 *		      with an incorrect first argument.
 *
 * CALLS
 *	getitimer()
 *
 * ALGORITHM
 *	loop if that option was specified
 *	allocate space and set up needed values
 *	issue the system call
 *	check the errno value
 *	  issue a PASS message if we get EINVAL
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  getitmer03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * USAGE
 *	./getitimer03
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <sys/time.h>

void cleanup(void);
void setup(void);

char *TCID = "getitimer03";
int TST_TOTAL = 1;

int exp_enos[] = { EINVAL, 0 };

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	struct itimerval *value;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* allocate some space for the timer structure */

		if ((value = (struct itimerval *)malloc((size_t)
							sizeof(struct
							       itimerval))) ==
		    NULL) {
			tst_brkm(TBROK, cleanup, "value malloc failed");
		}

		/*
		 * issue the system call with the TEST() macro
		 * ITIMER_REAL = 0, ITIMER_VIRTUAL = 1 and ITIMER_PROF = 2
		 */

		/* make the first value negative to get a failure */
		TEST(getitimer(-ITIMER_PROF, value));

		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call failed to produce expected error "
				 "- errno = %d - %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		switch (TEST_ERRNO) {
		case EINVAL:
			tst_resm(TPASS, "expected failure - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_resm(TFAIL, "call failed to produce expected error "
				 "- errno = %d - %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}

		/*
		 * clean up things in case we are looping
		 */

		free(value);
		value = NULL;
	}

	cleanup();

	tst_exit();
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

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

}