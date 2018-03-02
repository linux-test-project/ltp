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
 *	pipe05.c
 *
 * DESCRIPTION
 *	Check what happens when pipe is passed a bad file descriptor.
 *
 * ALGORITHM
 *	Issue the pipe call with a bad file descriptor.
 *	Check that we get EFAULT.
 *
 * USAGE:  <for command-line>
 *  pipe05 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */
#include <fcntl.h>
#include <errno.h>
#include <setjmp.h>
#include "test.h"

char *TCID = "pipe05";
int TST_TOTAL = 1;

intptr_t pipes;
void setup(void);
void cleanup(void);
jmp_buf sig11_recover;
void sig11_handler(int sig);

int main(int ac, char **av)
{
	volatile int lc;
	struct sigaction sa, osa;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;
		/* special sig11 case */
		sa.sa_handler = &sig11_handler;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;

		sigaction(SIGSEGV, NULL, &osa);
		sigaction(SIGSEGV, &sa, NULL);

		if (setjmp(sig11_recover)) {
			TEST_RETURN = -1;
			TEST_ERRNO = EFAULT;
		} else {
			TEST(pipe((int *)pipes));
		}
		sigaction(SIGSEGV, &osa, NULL);

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
		}

		if (TEST_ERRNO != EFAULT) {
			tst_resm(TFAIL, "unexpected error - %d : %s - "
				 "expected EMFILE", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS, "expected failure - "
				 "errno = %d : %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}

	}
	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/******************************************************************
 * sig11_handler() - our segfault recover hack
 ******************************************************************/
void sig11_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
	longjmp(sig11_recover, 1);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{
}
