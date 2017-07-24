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
/**********************************************************
 *
 *    TEST IDENTIFIER	: capset01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for capset(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This is a Phase I test for the capset(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  call capget() to save the current capability data
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  call capset() with the saved data
 *	  if return value == 0
 *		Test passed
 *	  Otherwise
 *		Test failed
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * capset01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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
 * CHANGES:
 *  2005/01/01: add an hint to a possible solution when test fails
 *              - Ricky Ng-Adam <rngadam@yahoo.com>
 ****************************************************************/
#include <unistd.h>
#include <errno.h>
#include "test.h"
#include "lapi/syscalls.h"

/**************************************************************************/
/*                                                                        */
/*   Some archs do not have the manpage documented sys/capability.h file, */
/*   and require the use of the line below                                */

#include <linux/capability.h>

/*   If you are having issues with including this file and have the sys/  */
/*   version, then you may want to try switching to it. -Robbie W.        */
/**************************************************************************/

static void setup();
static void cleanup();

char *TCID = "capset01";
int TST_TOTAL = 1;

static struct __user_cap_header_struct header;	/* cap_user_header_t is a pointer
						   to __user_cap_header_struct */

static struct __user_cap_data_struct data;	/* cap_user_data_t is a pointer to
						   __user_cap_data_struct */

int main(int ac, char **av)
{

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(ltp_syscall(__NR_capset, &header, &data));

		if (TEST_RETURN == 0) {
			tst_resm(TPASS, "capset() returned %ld", TEST_RETURN);
		} else {
			tst_resm(TFAIL | TTERRNO,
				 "Test Failed, capset() returned %ld"
				 " Maybe you need to do `modprobe capability`?",
				 TEST_RETURN);
		}
	}

	cleanup();

	tst_exit();
}

void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	header.version = _LINUX_CAPABILITY_VERSION;
	header.pid = 0;
	if (ltp_syscall(__NR_capget, &header, &data) == -1)
		tst_brkm(TBROK | TERRNO, NULL, "capget() failed");
}

void cleanup(void)
{
}
