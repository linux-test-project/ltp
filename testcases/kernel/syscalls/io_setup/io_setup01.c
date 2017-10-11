/*
 *
 *   Copyright (c) Crackerjack Project., 2007
 *   Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
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

/* Porting from Crackerjack to LTP is done
   by Masatake YAMATO <yamato@redhat.com> */

#include <errno.h>
#include <string.h>

#include "config.h"
#include "test.h"

char *TCID = "io_setup01";

int TST_TOTAL = 4;

#ifdef HAVE_LIBAIO
#include <libaio.h>

static void cleanup(void)
{
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
   DESCRIPTION
   io_setup  creates  an asynchronous I/O context capable of receiving at
   least nr_events.  ctxp must not point to an AIO context  that  already
   exists, and must be initialized to 0 prior to the call.  On successful
   creation of the AIO context, *ctxp is filled  in  with  the  resulting
   handle.
 */
int main(int argc, char *argv[])
{
	int lc;

	io_context_t ctx;
	int expected_return;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		memset(&ctx, 0, sizeof(ctx));
		expected_return = 0;
		TEST(io_setup(1, &ctx));

		if (TEST_RETURN == expected_return) {
			tst_resm(TPASS, "call succeeded expectedly");
			io_destroy(ctx);
		} else {
			tst_resm(TFAIL, "unexpected returned value - %ld - "
				 "expected %d", TEST_RETURN, expected_return);
		}

		memset(&ctx, 1, sizeof(ctx));
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

		memset(&ctx, 0, sizeof(ctx));
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

	}
	cleanup();

	tst_exit();
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL, "test requires libaio and it's development packages");
}
#endif
