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

char *TCID = "io_submit01";	/* Test program identifier.    */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int TST_TOTAL = 3;

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
	long expected_return;

	int rval;
	char buf[256];
	struct iocb iocb;
	struct iocb *iocbs[1];

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
		   io_submit() queues nr I/O request blocks for processing in the AIO con-
		   text ctx_id.  iocbpp should be an array of nr AIO request blocks, which
		   will be submitted to context ctx_id.

		   RETURN VALUE
		   On  success,  io_submit()  returns the number of iocbs submitted (which
		   may be 0 if nr is zero); on failure,  it  returns  one  of  the  errors
		   listed under ERRORS.

		   ERRORS
		   EINVAL The aio_context specified by ctx_id is invalid.  nr is less than
		   0.  The iocb at *iocbpp[0] is not properly initialized,  or  the
		   operation  specified  is  invalid for the file descriptor in the
		   iocb.
		 */
		expected_return = -EINVAL;
		TEST(io_submit(ctx, 0, NULL));
		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
		} else if (TEST_RETURN == expected_return) {
			tst_resm(TPASS, "expected failure - "
				 "returned value = %ld : %s", (-1 * TEST_RETURN),
				 strerror(-1 * TEST_RETURN));
		} else {
			tst_resm(TFAIL, "unexpected returned value - %ld - "
				 "expected %ld", TEST_RETURN, expected_return);
		}

		/*
		   EFAULT One of the data structures points to invalid data.
		 */
		expected_return = -EFAULT;
		TEST(io_submit(ctx, 1, (void *)-1));
		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
		} else if (TEST_RETURN == expected_return) {
			tst_resm(TPASS, "expected failure - "
				 "returned value = %ld : %s", (-1 * TEST_RETURN),
				 strerror(-1 * TEST_RETURN));
		} else {
			tst_resm(TFAIL, "unexpected returned value - %ld - "
				 "expected %ld", TEST_RETURN, expected_return);
		}

		/* Special case EFAULT or EINVAL (indetermination)

		   The errno depends on the per architecture implementation
		   of io_submit. On the architecture using compat_sys_io_submit
		   as its implementation, errno is set to -EINVAL. */
		{
			long expected_fault = -EFAULT;
			long expected_inval = -EINVAL;

			TEST(io_submit(ctx, 0, (void *)-1));
			if (TEST_RETURN == 0) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
			} else if (TEST_RETURN == expected_fault
				   || TEST_RETURN == expected_inval) {
				tst_resm(TPASS, "expected failure - "
					 "returned value = %ld : %s",
					 (-1 * TEST_RETURN),
					 strerror(-1 * TEST_RETURN));
			} else {
				tst_resm(TFAIL,
					 "unexpected returned value - %ld - "
					 "expected %ld(%s) or %ld(%s)",
					 TEST_RETURN, expected_fault,
					 strerror(-1 * expected_fault),
					 expected_inval,
					 strerror(-1 * expected_inval));
			}

		}

		/*
		   EBADF  The file descriptor specified in the first iocb is invalid.
		 */
		expected_return = -EBADF;
		io_prep_pread(&iocb, -1, (void *)buf, sizeof(buf), 0);
		iocbs[0] = &iocb;
		memset(&ctx, 0, sizeof(io_context_t));
		rval = io_setup(1, &ctx);
		if (rval != 0) {
			tst_brkm(TBROK, cleanup, "io_setup failed: %d", rval);
		 /*NOTREACHED*/}

		TEST(io_submit(ctx, 1, iocbs));
		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
		} else if (TEST_RETURN == expected_return) {
			tst_resm(TPASS, "expected failure - "
				 "returned value = %ld : %s", (-1 * TEST_RETURN),
				 strerror(-1 * TEST_RETURN));
		} else {
			tst_resm(TFAIL, "unexpected returned value - %ld - "
				 "expected %ld", TEST_RETURN, expected_return);
		}

		/*
		   EAGAIN Insufficient resources are available to queue any iocbs.

		   ENOSYS io_submit() is not implemented on this architecture.
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
