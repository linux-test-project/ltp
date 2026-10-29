// SPDX-License-Identifier: GPL-2.0-only

/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * AUTHOR: Saji Kumar.V.R <saji.kumar@wipro.com>
 */

#include <errno.h>
#include <sys/timex.h>
#include "tst_test.h"

#define SET_MODE (ADJ_OFFSET | ADJ_FREQUENCY | ADJ_MAXERROR | ADJ_ESTERROR | \
	ADJ_STATUS | ADJ_TIMECONST | ADJ_TICK)

static struct timex *tim_save;
static struct timex *buf;

void verify_adjtimex(void)
{
	*buf = *tim_save;
	buf->modes = SET_MODE;
	TEST(adjtimex(buf));
	if ((TST_RET >= TIME_OK) && (TST_RET <= TIME_ERROR)) {
		tst_res(TPASS, "adjtimex() with mode 0x%x ", SET_MODE);
	} else {
		tst_res(TFAIL | TTERRNO, "adjtimex() with mode 0x%x ",
				SET_MODE);
	}

	buf->modes = ADJ_OFFSET_SINGLESHOT;
	TEST(adjtimex(buf));
	if ((TST_RET >= TIME_OK) && (TST_RET <= TIME_ERROR)) {
		tst_res(TPASS, "adjtimex() with mode 0x%x ",
				ADJ_OFFSET_SINGLESHOT);
	} else {
		tst_res(TFAIL | TTERRNO,
				"adjtimex() with mode 0x%x ",
				ADJ_OFFSET_SINGLESHOT);
	}
}

static void setup(void)
{
	tim_save->modes = 0;

	/* Save current parameters */
	if ((adjtimex(tim_save)) == -1) {
		tst_brk(TBROK | TERRNO,
			"adjtimex(): failed to save current params");
	}
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.test_all = verify_adjtimex,
	.bufs = (struct tst_buffers []) {
		{&buf, .size = sizeof(*buf)},
		{&tim_save, .size = sizeof(*tim_save)},
		{},
	}
};
