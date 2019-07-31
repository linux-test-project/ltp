// SPDX-License-Identifier: GPL-2.0

/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *  AUTHOR : Saji Kumar.V.R <saji.kumar@wipro.com>
 */

#include <errno.h>
#include <sys/timex.h>
#include <unistd.h>
#include <pwd.h>
#include "tst_test.h"

#define SET_MODE ( ADJ_OFFSET | ADJ_FREQUENCY | ADJ_MAXERROR | ADJ_ESTERROR | \
	ADJ_STATUS | ADJ_TIMECONST | ADJ_TICK )

static int hz;			/* HZ from sysconf */

static struct timex *tim_save;
static struct timex *buf;

static struct passwd *ltpuser;

static void verify_adjtimex(unsigned int nr)
{
	struct timex *bufp;
	int expected_errno = 0;

	/*
	 * since Linux 2.6.26, if buf.offset value is outside
	 * the acceptable range, it is simply normalized instead
	 * of letting the syscall fail. so just skip this test
	 * case.
	 */
	if (nr > 3 && (tst_kvercmp(2, 6, 25) > 0)) {
		tst_res(TCONF, "this kernel normalizes buf."
				"offset value if it is outside"
				" the acceptable range.");
		return;
	}

	*buf = *tim_save;
	buf->modes = SET_MODE;
	bufp = buf;
	switch (nr) {
	case 0:
		bufp = (struct timex *)-1;
		expected_errno = EFAULT;
		break;
	case 1:
		buf->tick = 900000 / hz - 1;
		expected_errno = EINVAL;
		break;
	case 2:
		buf->tick = 1100000 / hz + 1;
		expected_errno = EINVAL;
		break;
	case 3:
		/* Switch to nobody user for correct error code collection */
		ltpuser = SAFE_GETPWNAM("nobody");
		SAFE_SETEUID(ltpuser->pw_uid);
		expected_errno = EPERM;
		break;
	case 4:
		buf->offset = 512000L + 1;
		expected_errno = EINVAL;
		break;
	case 5:
		buf->offset = (-1) * (512000L) - 1;
		expected_errno = EINVAL;
		break;
	default:
		tst_brk(TFAIL, "Invalid test case %u ", nr);
	}

	TEST(adjtimex(bufp));
	if ((TST_RET == -1) && (TST_ERR == expected_errno)) {
		tst_res(TPASS | TTERRNO,
				"adjtimex() error %u ", expected_errno);
	} else {
		tst_res(TFAIL | TTERRNO,
				"Test Failed, adjtimex() returned %ld",
				TST_RET);
	}

	/* clean up after ourselves */
	if (nr == 3)
		SAFE_SETEUID(0);
}

static void setup(void)
{
	tim_save->modes = 0;

	/* set the HZ from sysconf */
	hz = SAFE_SYSCONF(_SC_CLK_TCK);

	/* Save current parameters */
	if ((adjtimex(tim_save)) == -1)
		tst_brk(TBROK | TERRNO,
			"adjtimex(): failed to save current params");
}

static void cleanup(void)
{
	tim_save->modes = SET_MODE;

	/* Restore saved parameters */
	if ((adjtimex(tim_save)) == -1)
		tst_res(TWARN, "Failed to restore saved parameters");
}

static struct tst_test test = {
	.needs_root = 1,
	.tcnt = 6,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_adjtimex,
	.bufs = (struct tst_buffers []) {
		{&buf, .size = sizeof(*buf)},
		{&tim_save, .size = sizeof(*tim_save)},
		{},
	}
};
