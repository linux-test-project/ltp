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
 */

/*
  AUTHOR: Saji Kumar.V.R <saji.kumar@wipro.com>
  EXECUTED BY: root / superuser

  TEST ITEMS:
   1. Check to see if adjtimex succeed with mode combination :
      ADJ_OFFSET | ADJ_FREQUENCY | ADJ_MAXERROR | ADJ_ESTERROR |
      ADJ_STATUS | ADJ_TIMECONST | ADJ_TICK
   2. Check to see if adjtimex succeed with mode ADJ_OFFSET_SINGLESHOT
*/

#if defined UCLINUX && !__THROW
/* workaround for libc bug causing failure in sys/timex.h */
#define __THROW
#endif

#include <errno.h>
#include <sys/timex.h>
#include "test.h"

#define SET_MODE (ADJ_OFFSET | ADJ_FREQUENCY | ADJ_MAXERROR | ADJ_ESTERROR | \
	ADJ_STATUS | ADJ_TIMECONST | ADJ_TICK)

static void setup(void);
static void cleanup(void);

char *TCID = "adjtimex01";
int TST_TOTAL = 2;

static struct timex tim_save;

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* Call adjtimex(2) */
		tim_save.modes = SET_MODE;

		TEST(adjtimex(&tim_save));

		if ((TEST_RETURN >= 0) && (TEST_RETURN <= 5)) {
			tst_resm(TPASS, "adjtimex() with mode %u returned %ld",
				 SET_MODE, TEST_RETURN);
		} else {
			tst_resm(TFAIL | TTERRNO,
				"Test Failed, adjtimex() with mode %u "
				"returned %ld", SET_MODE, TEST_RETURN);
		}

		/* Call adjtimex(2) */
		tim_save.modes = ADJ_OFFSET_SINGLESHOT;

		TEST(adjtimex(&tim_save));

		if ((TEST_RETURN >= 0) && (TEST_RETURN <= 5)) {
			tst_resm(TPASS, "adjtimex() with mode %u returned %ld",
				 ADJ_OFFSET_SINGLESHOT, TEST_RETURN);
		} else {
			tst_resm(TFAIL | TTERRNO,
				"Test Failed, adjtimex() with mode %u returned "
				"%ld", ADJ_OFFSET_SINGLESHOT, TEST_RETURN);
		}
	}

	cleanup();

	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	tim_save.modes = 0;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Save current parameters in tim_save */
	if ((adjtimex(&tim_save)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to save current parameters");
}

static void cleanup(void)
{
}
