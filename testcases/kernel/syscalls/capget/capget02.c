/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER	: capget02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Tests for error conditions.
 *
 *    TEST CASE TOTAL	: 5
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that
 *	1) capget() fails with errno set to EFAULT if an invalid address
 *	   is given for header
 *	2) capget() fails with errno set to EFAULT if an invalid address
 *	   is given for data
 *	3) capget() fails with errno set to EINVAL if an invalid value
 *	   is given for header->version
 *	4) capget() fails with errno set to EINVAL if header->pid < 0
 *	5) capget() fails with errno set to ESRCH if the process with
 *	   pid, header->pid does not exit
 *
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  call capget with proper arguments
 *	  if capget() fails with expected errno
 *		Test passed
 *	  Otherwise
 *		Test failed
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * capget02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *			where,  -c n : Run n copies concurrently.
 *				-e   : Turn on errno logging.
 *				-h   : Show help screen
 *				-f   : Turn off functional testing
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 ****************************************************************/

#include <errno.h>
#include "test.h"
#include "usctest.h"

/**************************************************************************/
/*                                                                        */
/*   Some archs do not have the manpage documented sys/capability.h file, */
/*   and require the use of the line below                                */

#include <linux/capability.h>

/*   If you are having issues with including this file and have the sys/  */
/*   version, then you may want to try switching to it. -Robbie W.        */
/**************************************************************************/

#define INVALID_PID 999999

extern int capget(cap_user_header_t, cap_user_data_t);
static void setup();
static void cleanup();
static void test_setup(int);

char *TCID = "capget02";	/* Test program identifier.    */
extern int Tst_count;		/* Test Case counter for tst_* routines */
static int exp_enos[] = { EFAULT, EINVAL, ESRCH, 0 };

static struct __user_cap_header_struct header;
static struct __user_cap_data_struct data;

struct test_case_t {
	cap_user_header_t headerp;
	cap_user_data_t datap;
	int exp_errno;
	char *errdesc;
} test_cases[] = {
#ifndef UCLINUX
	/* Skip since uClinux does not implement memory protection */
	{
	(cap_user_header_t) - 1, &data, EFAULT, "EFAULT"}, {
	&header, (cap_user_data_t) - 1, EFAULT, "EFAULT"},
#endif
	{
	&header, &data, EINVAL, "EINVAL"}, {
	&header, &data, EINVAL, "EINVAL"}, {
	&header, &data, ESRCH, "ESRCH"}
};

int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av)
{

	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL))
	    != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* perform global setup for test */
	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			test_setup(i);
			TEST(capget(test_cases[i].headerp,
				    test_cases[i].datap));

			if ((TEST_RETURN == -1) && (TEST_ERRNO ==
						    test_cases[i].exp_errno)) {
				tst_resm(TPASS, "capget() returned -1,"
					 " errno: %s", test_cases[i].errdesc);
			} else {
				tst_resm(TFAIL|TTERRNO, "Test Failed, capget() returned %ld",
					 TEST_RETURN);
			}
			TEST_ERROR_LOG(TEST_ERRNO);
		}
	}			/* End for TEST_LOOPING */

	/* cleanup and exit */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified */
	TEST_PAUSE;

}				/* End setup() */

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
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
}				/* End cleanup() */

void test_setup(int i)
{
#ifdef UCLINUX
	i = i + 2;
#endif
	switch (i) {

	case 0:
		break;
	case 1:
		header.version = _LINUX_CAPABILITY_VERSION;
		header.pid = getpid();
		break;
	case 2:
		header.version = 0;
		header.pid = getpid();
		break;
	case 3:
		header.version = _LINUX_CAPABILITY_VERSION;
		header.pid = -1;
		break;
	case 4:
		header.version = _LINUX_CAPABILITY_VERSION;
		header.pid = INVALID_PID;
		break;
	}
}
