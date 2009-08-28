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
 *    TEST IDENTIFIER	: capset02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Tests for error conditions.
 *
 *    TEST CASE TOTAL	: 4
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that
 *	1) capset() fails with errno set to EFAULT if an invalid address
 *	   is given for header
 *	2) capset() fails with errno set to EFAULT if an invalid address
 *	   is given for data
 *	3) capset() fails with errno set to EINVAL if an invalid value
 *	   is given for header->version
 *	4) capset() fails with errno set to EPERM the process does not
 *	   have enough privilege to set capabilities
 *
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  Call capget() to save current capability data
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	  do test specific setup.
 * 	  call capset with proper arguments
 *	  if capset() fails with expected errno
 *		Test passed
 *	  Otherwise
 *		Test failed
 *	  do test specific cleanup
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * capset02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
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

#define INVALID_VERSION 0

extern int capget(cap_user_header_t, cap_user_data_t);
extern int capset(cap_user_header_t, const cap_user_data_t);

static void setup();
static void cleanup();
static void test_setup(int, char *);
static void child_func();

static pid_t child_pid = -1;

char *TCID = "capset02";	/* Test program identifier.    */
extern int Tst_count;		/* Test Case counter for tst_* routines */
static int exp_enos[] = { EFAULT, EINVAL, EPERM, 0 };

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
&header, &data, EPERM, "EPERM"},};

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
#ifdef UCLINUX
	maybe_run_child(&child_func, "");
#endif

	/* perform global setup for test */
	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			test_setup(i, av[0]);
			TEST(capset(test_cases[i].headerp,
				    test_cases[i].datap));

			if ((TEST_RETURN == -1) && (TEST_ERRNO ==
						    test_cases[i].exp_errno)) {
				tst_resm(TPASS, "capset() returned -1,"
					 " errno: %s", test_cases[i].errdesc);
			} else {
				tst_resm(TFAIL|TTERRNO, "Test Failed, capset() returned %ld",
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

	/*
	 * Save current capability data.
	 * header.version must be _LINUX_CAPABILITY_VERSION
	 */
	header.version = _LINUX_CAPABILITY_VERSION;
	if ((capget(&header, &data)) == -1) {
		tst_brkm(TBROK|TERRNO, tst_exit, "capget() failed");
	}

}				/* End setup() */

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{
	if (child_pid > 0)
		kill(child_pid, SIGTERM);
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */

void child_func()
{
	signal(SIGTERM, SIG_DFL);
	for (;;) {
		sleep(10);
	}
}

void test_setup(int i, char *argv0)
{
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;

#ifdef UCLINUX
	i = i + 2;
#endif
	switch (i) {

	case 0:
		break;

	case 1:
		header.version = _LINUX_CAPABILITY_VERSION;
		header.pid = 0;
		break;

	case 2:
		header.version = INVALID_VERSION;
		header.pid = 0;
		break;

	case 3:
		header.version = _LINUX_CAPABILITY_VERSION;
		/*
		 * when a non-zero pid is specified, process should have
		 * CAP_SETPCAP capability to change capabilities.
		 * by default, CAP_SETPCAP is not enabled. So giving
		 * a non-zero pid results in capset() failing with
		 * errno EPERM
		 *
		 * Note: this seems to have changed with recent kernels
		 * => create a child and try to set its capabilities
		 */
		child_pid = FORK_OR_VFORK();
		switch (child_pid) {
		case -1:
			tst_resm(TBROK|TERRNO, "fork() failed");
			cleanup();
			break;
		case 0:
#ifdef UCLINUX
			if (self_exec(argv0, "") < 0) {
				tst_resm(TBROK, "self_exec() failed");
				cleanup();
				break;
			}
#else
			child_func();
#endif
			break;
		default:
			signal(SIGCHLD, SIG_IGN);
			header.pid = child_pid;
			ltpuser = getpwnam(nobody_uid);
			if (seteuid(ltpuser->pw_uid) == -1) {
				tst_resm(TBROK|TERRNO, "seteuid() failed");
				cleanup();
			}

			break;
		}
		break;

	}
}
