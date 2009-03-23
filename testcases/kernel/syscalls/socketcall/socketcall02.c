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
/**************************************************************************
 *
 *    TEST IDENTIFIER	: socketcall02
 *
 *    EXECUTED BY	: All user
 *
 *    TEST TITLE	: Error test for socketcall(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: sowmya adiga<sowmya.adiga@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	verify socketcall(2) returns -1 and sets errno
 *	appropriately if argument passed is invalid.
 *
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	  Loop if the proper option is given.
 *	  Execute system call.
 *	  Check return code, If system call failed (return == -1) &&
 *				(errno set == expected errno)
 *	  Issue sys call pass with expected error
 *	  otherwise
 *	  Issue sys call fails with unexpected error
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  socketcall02 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *		where,		-c n : Run n copies concurrently
 *				-e   : Turn on errno logging.
 *				-h   : Show this help screen
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 * RESTRICTIONS
 * None
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/net.h>
#include <sys/un.h>
#include <netinet/in.h>

#include "test.h"
#include "usctest.h"

char *TCID = "socketcall02";	/* Test program identifier.    */

#ifdef __NR_socketcall

#define socketcall(call, args) syscall(__NR_socketcall, call, args)

void setup();
void cleanup();

int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* TestCase counter for tst_* routine */
int exp_enos[] = { EINVAL, 0 };

struct test_case_t {
	int call;
	unsigned long args[3];
	int retval;
	int experrno;
	char *desc;
} TC = {
	-1, {
PF_INET, SOCK_STREAM, 0}, -1, EINVAL, "invalid call"};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* perform global setup for test */
	setup();

	/* check looping state */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		TEST(socketcall(TC.call, TC.args));

		TEST_ERROR_LOG(TEST_ERRNO);

		/* check return code */
		if ((TEST_RETURN == -1)
		    && (TEST_ERRNO == TC.experrno)) {
			tst_resm(TPASS, "socketcall() failed"
				 " as expected for %s", TC.desc);
		} else {
			tst_brkm(TFAIL, tst_exit, "socketcall()"
				 " Failed with wrong experrno"
				 " =%d got: errno=%d : %s",
				 TC.experrno, TEST_ERRNO, strerror(TEST_ERRNO));
		}
	}

	/* cleanup and exit */
	cleanup();

	return 0;
}				/* End main */

/* setup() - performs all ONE TIME setup for this test. */
void setup()
{

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/*set the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}

#else

int TST_TOTAL = 0;		/* Total number of test cases. */

int main()
{
	tst_resm(TPASS, "socket call test on this architecture disabled.");
	tst_exit();
	return 0;
}

#endif
