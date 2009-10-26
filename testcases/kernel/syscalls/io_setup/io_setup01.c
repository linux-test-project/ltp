/*
 *
 *   Copyright (c) Crackerjack Project., 2007
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

/* Porting from Crackerjack to LTP is done
   by Masatake YAMATO <yamato@redhat.com> */

#include "config.h"
#include "test.h"
#include "usctest.h"

char *TCID = "io_setup01";	/* Test program identifier.    */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int TST_TOTAL = 4;

#ifdef HAVE_LIBAIO_H
#include <libaio.h>
#include <errno.h>
#include <string.h>

/*
 * cleanup()
 * 	performs all the ONE TIME cleanup for this test at completion or
 * 	premature exit
 */
void cleanup(void)
{
	/*
	 * print timing status if that option was specified
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

}				/* End setup() */

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* parse_opts() return message */

	io_context_t ctx;
	int expected_return;

	if ((msg =
	     parse_opts(argc, argv, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/*

		   DESCRIPTION
		   io_setup  creates  an asynchronous I/O context capable of receiving at
		   least nr_events.  ctxp must not point to an AIO context  that  already
		   exists, and must be initialized to 0 prior to the call.  On successful
		   creation of the AIO context, *ctxp is filled  in  with  the  resulting
		   handle.
		 */
		memset(&ctx, 0, sizeof(io_context_t));
		expected_return = 0;
		TEST(io_setup(1, &ctx));

		if (TEST_RETURN == expected_return) {
			tst_resm(TPASS, "call succeeded expectedly");
			io_destroy(ctx);
		} else {
			tst_resm(TFAIL, "unexpected returned value - %ld - "
				 "expected %d", TEST_RETURN, expected_return);
		}

		/*
		   RETURN VALUE
		   io_setup  returns 0 on success; otherwise, one of the errors listed in
		   the "Errors" section is returned.

		   ERRORS
		   EINVAL ctxp is not initialized, or the specified nr_events exceeds in-
		   ternal limits. nr_events should be greater than 0.
		 */
		memset(&ctx, 1, sizeof(io_context_t));
		expected_return = -EINVAL;
		TEST(io_setup(1, &ctx));

		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			io_destroy(ctx);
		} else if (TEST_RETURN == expected_return) {
			tst_resm(TPASS, "expected failure - "
				 "returned value = %ld : %s", TEST_RETURN,
				 strerror(-1 * TEST_RETURN));
		} else {
			tst_resm(TFAIL, "unexpected returned value - %ld - "
				 "expected %d", TEST_RETURN, expected_return);
		}

		memset(&ctx, 0, sizeof(io_context_t));
		expected_return = -EINVAL;
		TEST(io_setup(-1, &ctx));
		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			io_destroy(ctx);
		} else if (TEST_RETURN == expected_return) {
			tst_resm(TPASS, "expected failure - "
				 "returned value = %ld : %s", TEST_RETURN,
				 strerror(-1 * TEST_RETURN));
		} else {
			tst_resm(TFAIL, "unexpected returned value - %ld - "
				 "expected %d", TEST_RETURN, expected_return);
		}

		/*

		   EFAULT An invalid pointer is passed for ctxp.
		 */
		expected_return = -EFAULT;
		TEST(io_setup(1, NULL));
		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			io_destroy(ctx);
		} else if (TEST_RETURN == expected_return) {
			tst_resm(TPASS, "expected failure - "
				 "returned value = %ld : %s", TEST_RETURN,
				 strerror(-1 * TEST_RETURN));
		} else {
			tst_resm(TFAIL, "unexpected returned value - %ld - "
				 "expected %d", TEST_RETURN, expected_return);
		}

		/*
		   ENOMEM Insufficient kernel resources are available.

		   EAGAIN The  specified  nr_events exceeds the user's limit of available
		   events.
		 */
		/*
		   ENOSYS io_setup is not implemented on this architecture.
		 */
		/* Crackerjack has a test case for ENOSYS. But Testing for ENOSYS
		   is not meaningful for LTP, I think.
		   -- Masatake */
	}
	cleanup();

	return 0;
}
#else
int main(int argc, char **argv)
{
	tst_resm(TCONF, "System doesn't support execution of the test");
	return 0;
}
#endif
