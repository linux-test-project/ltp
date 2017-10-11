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

char *TCID = "io_destroy01";

int TST_TOTAL = 1;

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
       io_destroy  removes  the asynchronous I/O context from the list of I/O
       contexts and then destroys it.  io_destroy can also  cancel  any  out-
       standing asynchronous I/O actions on ctx and block on completion.

 RETURN VALUE
       io_destroy returns 0 on success.

 ERRORS
       EINVAL The AIO context specified by ctx is invalid.
*/

#define EXP_RET (-EINVAL)

int main(int argc, char *argv[])
{
	int lc;

	io_context_t ctx;

	memset(&ctx, 0xff, sizeof(ctx));

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(io_destroy(ctx));

		switch (TEST_RETURN) {
		case 0:
			tst_resm(TFAIL, "call succeeded unexpectedly");
			break;
		case EXP_RET:
			tst_resm(TPASS, "expected failure - "
				 "returned value = %ld : %s", TEST_RETURN,
				 strerror(-TEST_RETURN));
			break;
		case -ENOSYS:
			tst_resm(TCONF, "io_cancel returned ENOSYS");
			break;
		default:
			tst_resm(TFAIL, "unexpected returned value - %s (%i) - "
				 "expected %s (%i)", strerror(-TEST_RETURN),
				 (int)TEST_RETURN, strerror(-EXP_RET), EXP_RET);
			break;
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
