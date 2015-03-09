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
 *    TEST IDENTIFIER	: munlock02
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Test for checking basic error conditions for
 * 	   		  munlock(2)
 *
 *    TEST CASE TOTAL	: 2
 *
 *    AUTHOR		: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 * 	Check for basic errors returned by munlock(2) system call.
 *
 * 	Verify that munlock(2) returns -1 and sets errno to
 *
 * 	1) ENOMEM - Some of the specified address range does not correspond to
 *			mapped pages in the address space of the process.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	  Do necessary setup for each test.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  munlock02 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
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
 *	Test must run as root.
 *****************************************************************************/
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pwd.h>
#include "test.h"

void setup();
void cleanup();

char *TCID = "munlock02";
int TST_TOTAL = 1;

#define LEN	1024

void *addr1;

struct test_case_t {
	void *addr;
	int len;
	int error;
	char *edesc;
} TC[] = {
	{
NULL, 0, ENOMEM, "address range out of address space"},};

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* check looping state */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++) {
#ifdef __ia64__
			TC[0].len = 8 * getpagesize();
#endif
			TEST(munlock(TC[i].addr, TC[i].len));

			/* check return code */
			if (TEST_RETURN == -1) {
				if (TEST_ERRNO != TC[i].error)
					tst_brkm(TFAIL, cleanup,
						 "munlock() Failed with wrong "
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
					 "munlock() Failed, expected "
					 "return value=-1, got %ld",
					 TEST_RETURN);
			}
		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();
}

/* setup() - performs all ONE TIME setup for this test. */

void setup(void)
{

	char *address;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TC[0].len = 8 * getpagesize();
	address = mmap(0, TC[0].len, PROT_READ | PROT_WRITE,
		       MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (address == MAP_FAILED)
		tst_brkm(TFAIL, cleanup, "mmap_failed");
	memset(address, 0x20, TC[0].len);
	TEST(mlock(address, TC[0].len));

	/* check return code */
	if (TEST_RETURN == -1) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "mlock(%p, %d) Failed with return=%ld", address,
			 TC[0].len, TEST_RETURN);
	}
	TC[0].addr = address;
	/*
	 * unmap part of the area, to create the condition for ENOMEM
	 */
	address += 2 * getpagesize();
	munmap(address, 4 * getpagesize());

	TEST_PAUSE;

	return;
}

#else

int main(void)
{
	tst_resm(TINFO, "test is not available on uClinux");
	tst_exit();
}

#endif /* if !defined(UCLINUX) */

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{
	return;
}
