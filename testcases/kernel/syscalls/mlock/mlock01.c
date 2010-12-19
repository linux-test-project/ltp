/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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
 * 	mlock01.c
 *
 * DESCRIPTION
 * 	Test to see that mlock works
 *$
 * ALGORITHM
 * 	test 1:
 *		Call mlock with various valid addresses and lengths.  No
 *		error should be returned
 *
 * USAGE:  <for command-line>
 *         -c n    Run n copies concurrently
 *         -e      Turn on errno logging
 *         -f      Turn off functional testing
 *         -h      Show this help screen
 *         -i n    Execute test n times
 *         -I x    Execute test for x seconds
 *         -p      Pause for SIGUSR1 before starting
 *         -P x    Pause for x seconds between iterations
 *         -t      Turn on syscall timing
 *
 * HISTORY
 *	06/2002 Written by Paul Larson
 *
 * RESTRICTIONS
 * 	None
 */
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include "test.h"
#include "usctest.h"

void setup();
void setup1(int);
void cleanup();

char *TCID = "mlock01";		/* Test program identifier.    */
int TST_TOTAL = 4;		/* Total number of test cases. */

int exp_enos[] = { 0 };

void *addr1;

struct test_case_t {
	void **addr;
	int len;
	void (*setupfunc)();
} TC[] = {
	/* mlock should return ENOMEM when some or all of the address
	 * range pointed to by addr and len are not valid mapped pages
	 * in the address space of the process
	 */
	{ &addr1, 1, setup1},
	{ &addr1, 1024, setup1},
	{ &addr1, 1024 * 1024, setup1},
	{ &addr1, 1024 * 1024 * 10, setup1}
};

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	/*
	 * FIXME (garrcoop): this should really test out whether or not the
	 * process's mappable address space is indeed accessible by the
	 * current user, instead of needing to be run by root all the time.
	 */
	tst_require_root(NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (TC[i].setupfunc != NULL)
				TC[i].setupfunc(TC[i].len);

			TEST(mlock(*(TC[i].addr), TC[i].len));

			/* I'm confused -- given the description above this
			 * should fail as designed, but this application
			 * */
			if (TEST_RETURN == -1)
				tst_resm(TFAIL|TTERRNO, "mlock failed");
			else
				tst_resm(TPASS, "mlock passed");
		}
	}

	cleanup();

	tst_exit();
}

#else

int main()
{
	tst_brkm(TCONF, NULL, "test is not available on uClinux");
}

#endif /* if !defined(UCLINUX) */

void setup()
{
	TEST_PAUSE;
}

void setup1(int len)
{
	addr1 = malloc(len);
	if (addr1 == NULL)
		tst_brkm(TFAIL, cleanup, "malloc failed");
}

void cleanup()
{
	TEST_CLEANUP;

}