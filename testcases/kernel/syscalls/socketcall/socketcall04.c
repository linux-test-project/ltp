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
 *    TEST IDENTIFIER	: socketcall04
 *
 *    EXECUTED BY	: All user
 *
 *    TEST TITLE	: Basic test for socketcall(2) for listen(2)
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
 *	This is a phase I test for the socketcall(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *        Execute system call
 *	  Check return code, if system call failed (return=-1)
 *	  Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  socketcall04 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *		where,		-c n : Run n copies concurrently
 *				-e   : Turn on errno logging.
 *				-h   : Show this help screen
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				 t   : Turn on syscall timing.
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

char *TCID = "socketcall04";	/* Test program identifier.    */

#ifdef __NR_socketcall

#define socketcall(call, args) syscall(__NR_socketcall, call, args)

void setup();
void cleanup();
void setup1(void);

int TST_TOTAL = 1;		/* Total number of test cases. */
int i, s;
unsigned long args[3];
struct sockaddr_in si;

struct test_case_t {
	int domain;
	int type;
	int pro;
	int call;
	void (*setupfunc) (void);
	char *desc;
} TC = {
AF_INET, SOCK_STREAM, 6, SYS_LISTEN, setup1, "listen call"};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	setup();

	/* check looping state */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		TC.setupfunc();

		TEST(socketcall(TC.call, args));

		/* check return code */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL|TTERRNO, "socketcall() Failed "
				 " with return=%ld",
				 TEST_RETURN);
		} else {
			tst_resm(TPASS, "socketcall() passed "
				 "for %s with return=%ld ",
				 TC.desc, TEST_RETURN);
			close(s);
		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();
}

/*setup1()*/
void setup1(void)
{
	if ((s = socket(TC.domain, TC.type, TC.pro)) == -1) {
		tst_brkm(TBROK, NULL, "socket creation failed");
	}
	args[0] = s;
	args[1] = 1;
}

/* setup() - performs all ONE TIME setup for this test. */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{
	TEST_CLEANUP;

}

#else

int TST_TOTAL = 0;		/* Total number of test cases. */

int main()
{
	tst_resm(TPASS, "socket call test on this architecture disabled.");
	tst_exit();
	tst_exit();
}

#endif
