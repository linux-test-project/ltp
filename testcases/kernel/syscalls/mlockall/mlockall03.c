/*
 * Copyright (C) Bull S.A. 2005. $
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
/**************************************************************************
 *
 *    TEST IDENTIFIER	: mlockall03
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Test for checking basic error conditions for
 *    			   mlockall(2)
 *
 *    TEST CASE TOTAL	: 3
 *
 *    AUTHOR		: Jacky Malcles
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *$
 * 	Verify that mlockall(2) returns -1 and sets errno to
 *
 *	1) ENOMEM - If the caller had a non-zero RLIMIT_MEMLOCK
 *		    and tried to lock more memory than the limit permitted.
 *	2) EPERM  - If the  caller  was  not  privileged
 *		    and its RLIMIT_MEMLOCK soft resource limit was 0.
 *	3) EINVAL - Unknown flags were specified.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	  Do necessary setup for each test.
 *	  Execute system call
 *	  Check return code, if system call failed and errno == expected errno
 *		Issue sys call passed with expected return value and errno.
 *	  Otherwise,
 *		Issue sys call failed to produce expected error.
 *	  Do cleanup for each test.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  mlockall03 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *		where,
 *			-c n : Run n copies concurrently
 *			-e   : Turn on errno logging.
 *			-h   : Show this help screen
 *			-i n : Execute test n times.
 *			-I x : Execute test for x seconds.
 *			-p   : Pause for SIGUSR1 before starting
 *			-P x : Pause for x seconds between iterations.
 *			-t   : Turn on syscall timing.
 *
 * RESTRICTIONS
 *	Test must run as root.
 *****************************************************************************/
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/mman.h>
#include "test.h"
#include "usctest.h"
#include <sys/resource.h>
#include <sys/utsname.h>

void setup();
int setup_test(int);
int compare(char s1[], char s2[]);
void cleanup_test(int);
void cleanup();

char *TCID = "mlockall03";	/* Test program identifier.    */
int TST_TOTAL = 3;		/* Total number of test cases. */
extern int Tst_count;		/* TestCase counter for tst_* routine */

#if !defined(UCLINUX)

char *ref_release = "2.6.8\0";
int exp_enos[] = { ENOMEM, EPERM, EINVAL, 0 };

struct test_case_t {
	int flag;		/* flag value                   */
	int error;		/* error description            */
	char *edesc;		/* Expected error no            */
} TC[] = {
	{
	MCL_CURRENT, ENOMEM,
		    "tried to lock more memory than the limit permitted"}, {
	MCL_CURRENT, EPERM, "Not a superuser and RLIMIT_MEMLOCK was 0"}, {
	~(MCL_CURRENT | MCL_FUTURE), EINVAL, "Unknown flag"}
};

int main(int ac, char **av)
{
	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */
	struct utsname *buf;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* allocate some space for buf */
	if ((buf = (struct utsname *)malloc((size_t)
					    sizeof(struct utsname))) == NULL) {
		tst_resm(TFAIL, "malloc failed for buf");
		return 0;
	}

	if (uname(buf) < 0) {
		tst_resm(TFAIL, "uname failed getting release number");
	}

	if ((compare(ref_release, buf->release)) <= 0) {
		tst_resm(TCONF,
			 "In Linux 2.6.8 and earlier this test will not run.");
		tst_exit();
	}

	/* perform global setup for test */
	setup();

	/* check looping state */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (setup_test(i)) {
				tst_resm(TFAIL, "mlockall() Failed while setup "
					 "for checking error %s", TC[i].edesc);
				continue;
			}

			TEST(mlockall(TC[i].flag));

			/* check return code */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				if (TEST_ERRNO != TC[i].error)
					tst_brkm(TFAIL, cleanup,
						 "mlockall() Failed with wrong "
						 "errno, expected errno=%s, "
						 "got errno=%d : %s",
						 TC[i].edesc, TEST_ERRNO,
						 strerror(TEST_ERRNO));
				else
					tst_resm(TPASS,
						 "expected failure - errno "
						 "= %d : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
			} else {
				tst_brkm(TFAIL, cleanup,
					 "mlockall() Failed, expected "
					 "return value=-1, got %ld",
					 TEST_RETURN);
			}
			cleanup_test(i);
		}
	}			/* End for TEST_LOOPING */

	/* cleanup and exit */
	cleanup();

	return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	return;
}

int compare(char s1[], char s2[])
{
	int i = 0;
	while (s1[i] == s2[i] && s1[i])
		i++;

	if (i < 4)
		return s2[i] - s1[i];
	if ((i == 4) && (isalnum(s2[i + 1]))) {
		return 1;
	} else {
		/* it is not an alphanumeric character */
		return s2[i] - s1[i];
	}
	return 0;
}

int setup_test(int i)
{
	struct rlimit rl;
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;

	switch (i) {
	case 0:
		ltpuser = getpwnam(nobody_uid);
		if (seteuid(ltpuser->pw_uid) == -1) {
			tst_brkm(TBROK, cleanup, "seteuid() failed to "
				 "change euid to %d errno = %d : %s",
				 ltpuser->pw_uid, TEST_ERRNO,
				 strerror(TEST_ERRNO));
			return 1;
		}

		rl.rlim_max = 10;
		rl.rlim_cur = 7;

		if (setrlimit(RLIMIT_MEMLOCK, &rl) != 0) {
			tst_resm(TWARN, "setrlimit failed to set the "
				 "resource for RLIMIT_MEMLOCK to check "
				 "for mlockall() error %s\n", TC[i].edesc);
			return 1;
		}
		return 0;
	case 1:
		rl.rlim_max = 0;
		rl.rlim_cur = 0;

		if (setrlimit(RLIMIT_MEMLOCK, &rl) != 0) {
			tst_resm(TWARN, "setrlimit failed to set the "
				 "resource for RLIMIT_MEMLOCK to check "
				 "for mlockall() error %s\n", TC[i].edesc);
			return 1;
		}

		ltpuser = getpwnam(nobody_uid);
		if (seteuid(ltpuser->pw_uid) == -1) {
			tst_brkm(TBROK, cleanup, "seteuid() failed to "
				 "change euid to %d errno = %d : %s",
				 ltpuser->pw_uid, TEST_ERRNO,
				 strerror(TEST_ERRNO));
			return 1;
		}

		return 0;
	}
	return 0;
}

void cleanup_test(int i)
{
	struct rlimit rl;

	switch (i) {
	case 0:
		if (seteuid(0) == -1) {
			tst_brkm(TBROK, cleanup, "seteuid() failed to "
				 "change euid to %d errno = %d : %s",
				 0, TEST_ERRNO, strerror(TEST_ERRNO));
		}

		rl.rlim_max = -1;
		rl.rlim_cur = -1;

		if (setrlimit(RLIMIT_MEMLOCK, &rl) != 0) {
			tst_brkm(TFAIL, cleanup,
				 "setrlimit failed to reset the "
				 "resource for RLIMIT_MEMLOCK while "
				 "checking for mlockall() error %s\n",
				 TC[i].edesc);
		}

		return;

	case 1:
		if (seteuid(0) == -1) {
			tst_brkm(TBROK, cleanup, "seteuid() failed to "
				 "change euid to %d errno = %d : %s",
				 0, TEST_ERRNO, strerror(TEST_ERRNO));
		}
		return;

	}
}

#else

int main()
{
	tst_resm(TINFO, "test is not available on uClinux");
	return 0;
}

#endif /* if !defined(UCLINUX) */

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();

	return;
}
