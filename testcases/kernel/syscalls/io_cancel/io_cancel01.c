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

char *TCID = "io_cancel01";	/* Test program identifier.    */

int TST_TOTAL = 1;

#ifdef HAVE_LIBAIO_H
#include <libaio.h>
#include <errno.h>
#include <string.h>

void cleanup(void)
{
	TEST_CLEANUP;

}

void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* parse_opts() return message */

	io_context_t ctx = 0;
	long expected_return;

	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		/*
		   DESCRIPTION
		   io_cancel attempts to cancel an asynchronous I/O operation  previously
		   submitted  with  the io_submit system call.  ctx_id is the AIO context
		   ID of the operation to be cancelled.  If the AIO context is found, the
		   event  will be cancelled and then copied into the memory pointed to by
		   result without being placed into the completion queue.

		   RETURN VALUE
		   io_cancel returns 0 on success; otherwise, it returns one of  the  er-
		   rors listed in the "Errors" section.

		   ERRORS
		   EINVAL The AIO context specified by ctx_id is invalid.

		   EFAULT One of the data structures points to invalid data.
		 */
		expected_return = -EFAULT;
		TEST(io_cancel(ctx, NULL, NULL));

		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			continue;
		}

		if (TEST_RETURN == expected_return) {
			tst_resm(TPASS, "expected failure - "
				 "returned value = %ld : %s", TEST_RETURN,
				 strerror(-1 * TEST_RETURN));
		} else {
			tst_resm(TFAIL, "unexpected returned value - %ld - "
				 "expected %ld", TEST_RETURN, expected_return);
		}

		/*
		   EAGAIN The iocb specified was not cancelled.

		   ENOSYS io_cancel is not implemented on this architecture.
		 */
		/* Crackerjack has a test case for ENOSYS. But Testing for ENOSYS
		   is not meaningful for LTP, I think.
		   -- Masatake */
	}
	cleanup();

	tst_exit();
}
#else
int main(int argc, char **argv)
{
	tst_brkm(TCONF, NULL, "System doesn't have libaio support");
}
#endif
