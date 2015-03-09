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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

char *TCID = "socketcall02";

#ifdef __NR_socketcall

#define socketcall(call, args) syscall(__NR_socketcall, call, args)

void setup();
void cleanup();

int TST_TOTAL = 1;

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
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* check looping state */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(socketcall(TC.call, TC.args));

		/* check return code */
		if ((TEST_RETURN == -1)
		    && (TEST_ERRNO == TC.experrno)) {
			tst_resm(TPASS, "socketcall() failed"
				 " as expected for %s", TC.desc);
		} else {
			tst_brkm(TFAIL, NULL, "socketcall()"
				 " Failed with wrong experrno"
				 " =%d got: errno=%d : %s",
				 TC.experrno, TEST_ERRNO, strerror(TEST_ERRNO));
		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();
}

/* setup() - performs all ONE TIME setup for this test. */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{
}

#else

int TST_TOTAL = 0;

int main(void)
{
	tst_resm(TPASS, "socket call test on this architecture disabled.");
	tst_exit();
}

#endif
