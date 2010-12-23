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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Porting from Crackerjack to LTP is done
   by Masatake YAMATO <yamato@redhat.com> */

#include "config.h"
#include "test.h"
#include "usctest.h"

char *TCID = "io_getevents01";

int TST_TOTAL = 1;

#ifdef HAVE_LIBAIO_H
#include <libaio.h>
#include <errno.h>
#include <string.h>

static void cleanup(void)
{
	TEST_CLEANUP;
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
   DESCRIPTION
   io_getevents  attempts  to  read  at  least min_nr events and up to nr
   events from the completion queue  of  the  AIO  context  specified  by
   ctx_id.   timeout  specifies  the  amount  of time to wait for events,
   where a NULL timeout waits until at  least  min_nr  events  have  been
   seen.   Note  that timeout is relative and will be updated if not NULL
   and the operation blocks.

   RETURN VALUE
   io_getevents returns the number of events read: 0  if  no  events  are
   available or < min_nr if the timeout has elapsed.

   ERRORS
   EINVAL ctx_id  is  invalid.  min_nr  is  out  of range or nr is out of
   range.
 */

#define EXP_RET (-EINVAL)

int main(int argc, char *argv[])
{
	int lc;
	char *msg;
	
	io_context_t ctx;

	memset(&ctx, 0, sizeof(ctx));

	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		TEST(io_getevents(ctx, 0, 0, NULL, NULL));

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
int main(int argc, char *argv[])
{
	tst_brkm(TCONF, NULL, "System doesn't support execution of the test");
	tst_exit();
}
#endif
