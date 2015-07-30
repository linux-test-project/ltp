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
 *    TEST IDENTIFIER	: ioperm01
 *
 *    EXECUTED BY	: superuser
 *
 *    TEST TITLE	: Basic test for ioperm(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Subhab Biswas <subhabrata.biswas@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This is a Phase I test for the ioperm(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Test caller is superuser
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *        Check return code, if system call failed (return=-1)
 *              Issue FAIL message with errno.
 *        Otherwise, Issue PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * ioperm01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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

char *TCID = "ioperm01";

#if defined __i386__ || defined(__x86_64__)

#include <errno.h>
#include <unistd.h>
#include <sys/io.h>

#include "test.h"

unsigned long io_addr;		/*kernel version dependant io start address */
#define NUM_BYTES 3		/* number of bytes from start address */
#define TURN_ON 1
#define TURN_OFF 0
#ifndef IO_BITMAP_BITS
#define IO_BITMAP_BITS 1024
#endif

static void setup();
static void cleanup();

int TST_TOTAL = 1;

int main(int ac, char **av)
{

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Test the system call.
		 */
		TEST(ioperm(io_addr, NUM_BYTES, TURN_ON));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "ioperm() failed for port address "
				 "%lu,  errno=%d : %s", io_addr,
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS, "ioperm() passed for port "
				 "address %lu, returned %lu",
				 io_addr, TEST_RETURN);
		}
	}

	/* cleanup and exit */
	cleanup();
	tst_exit();

}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/*
	 * The value of IO_BITMAP_BITS (include/asm-i386/processor.h) changed
	 * from kernel 2.6.8 to permit 16-bits ioperm
	 *
	 * Ricky Ng-Adam, rngadam@yahoo.com
	 * */
	if (tst_kvercmp(2, 6, 8) < 0) {
		/*get ioperm on 1021, 1022, 1023 */
		io_addr = IO_BITMAP_BITS - NUM_BYTES;
	} else {
		/*get ioperm on 65533, 65534, 65535 */
		io_addr = IO_BITMAP_BITS - NUM_BYTES;
	}

	TEST_PAUSE;

}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{

	/*
	 * Reset I/O privileges for the specified port.
	 */
	if ((ioperm(io_addr, NUM_BYTES, TURN_OFF)) == -1) {
		tst_brkm(TBROK, NULL, "ioperm() cleanup failed");
	}

}

#else /* __i386__ */

#include "test.h"

int TST_TOTAL = 0;

int main(void)
{
	tst_resm(TPASS,
		 "LSB v1.3 does not specify ioperm() for this architecture.");
	tst_exit();
}

#endif /* __i386__ */
