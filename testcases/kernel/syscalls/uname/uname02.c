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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	uname02.c
 *
 * DESCRIPTION
 *	uname02 - call uname() with an invalid address to produce a failure
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call
 *	check the errno value
 *	  issue a PASS message if we get EFAULT - errno 14
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  uname02 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *		where,  -c n : Run n copies concurrently.
 *			-e   : Turn on errno logging.
 *			-i n : Execute test n times.
 *			-I x : Execute test for x seconds.
 *			-P x : Pause for x seconds between iterations.
 *			-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	none
 */

#include "test.h"

#include <errno.h>
#include <sys/utsname.h>

void cleanup(void);
void setup(void);

char *TCID = "uname02";
int TST_TOTAL = 1;

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		/*
		 * call the system call with the TEST() macro
		 * send -1 for an illegal address
		 */

		TEST(uname((struct utsname *)-1));

		if (TEST_RETURN == 0)
			tst_resm(TFAIL, "call succeed when failure expected");

		switch (TEST_ERRNO) {
		case EFAULT:
			tst_resm(TPASS | TTERRNO, "uname failed as expected");
			break;
		default:
			tst_resm(TFAIL | TTERRNO, "uname failed unexpectedly");
		}
	}

	cleanup();

	tst_exit();

}

void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

void cleanup(void)
{
}
#else
int main(void)
{
	tst_resm(TCONF, NULL, "test is not available on uClinux");
}
#endif /* if !defined(UCLINUX) */
