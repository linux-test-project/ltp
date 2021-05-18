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
#include "lapi/syscalls.h"

#define SET_MODE (ADJ_OFFSET | ADJ_FREQUENCY | ADJ_MAXERROR | ADJ_ESTERROR | \
				ADJ_STATUS | ADJ_TIMECONST | ADJ_TICK)

static int hz;		/* HZ from sysconf */

static struct timex *tim_save, *buf;
static struct passwd *ltpuser;

struct test_case {
	unsigned int modes;
	long lowlimit;
	long highlimit;
	long delta;
	int exp_err;
};

static int libc_adjtimex(void *timex)
{
	return adjtimex(timex);
}

static int sys_adjtimex(void *timex)
{
	return tst_syscall(__NR_adjtimex, timex);
}

static struct test_case tc[] = {
	{
	.modes = SET_MODE,
	.exp_err = EFAULT,
	},
	{
	.modes = ADJ_TICK,
	.lowlimit = 900000,
	.delta = 1,
	.exp_err = EINVAL,
	},
	{
	.modes = ADJ_TICK,
	.highlimit = 1100000,
	.delta = 1,
	.exp_err = EINVAL,
	},
	{
	.modes = ADJ_OFFSET,
	.highlimit = 512000L,
	.delta = 1,
	.exp_err = EINVAL,
	},
	{
	.modes = ADJ_OFFSET,
	.lowlimit = -512000L,
	.delta = -1,
	.exp_err = EINVAL,
	},
};

static struct test_variants
{
	int (*adjtimex)(void *timex);
	char *desc;
} variants[] = {
	{ .adjtimex = libc_adjtimex, .desc = "libc adjtimex()"},

#if (__NR_adjtimex != __LTP__NR_INVALID_SYSCALL)
	{ .adjtimex = sys_adjtimex,  .desc = "__NR_adjtimex syscall"},
#endif
};

static void verify_adjtimex(unsigned int i)
{
	struct timex *bufp;
	struct test_variants *tv = &variants[tst_variant];

	*buf = *tim_save;
	buf->modes = SET_MODE;
	bufp = buf;

	if (tc[i].exp_err == EINVAL) {
		if (tc[i].modes == ADJ_TICK) {
			if (tc[i].lowlimit)
				buf->tick = tc[i].lowlimit - tc[i].delta;

			if (tc[i].highlimit)
				buf->tick = tc[i].highlimit + tc[i].delta;
		}
		if (tc[i].modes == ADJ_OFFSET) {
			if (tc[i].lowlimit) {
				tst_res(TCONF, "this kernel normalizes buf. offset value if it is outside the acceptable range.");
				return;
			}
			if (tc[i].highlimit) {
				tst_res(TCONF, "this kernel normalizes buf. offset value if it is outside the acceptable range.");
				return;
			}
		}
	}

	if (tc[i].exp_err == EFAULT) {
		if (tv->adjtimex != libc_adjtimex) {
			bufp = (struct timex *) -1;
		} else {
			tst_res(TCONF, "EFAULT is skipped for libc variant");
			return;
		}
	}

	TEST(tv->adjtimex(bufp));

	if (tc[i].exp_err != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "adjtimex(): expected %s mode %x",
					tst_strerrno(tc[i].exp_err), tc[i].modes);
		return;
	}

	tst_res(TPASS, "adjtimex() error %s", tst_strerrno(tc[i].exp_err));

}

static void setup(void)
{
	struct test_variants *tv = &variants[tst_variant];
	size_t i;
	int expected_errno = 0;

	tst_res(TINFO, "Testing variant: %s", tv->desc);

	tim_save->modes = 0;

	buf->modes = SET_MODE;

	/* Switch to nobody user for correct error code collection */
	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(ltpuser->pw_uid);
	expected_errno = EPERM;

	TEST(tv->adjtimex(buf));

	if ((TST_RET == -1) && (TST_ERR == expected_errno)) {
		tst_res(TPASS, "adjtimex() error %s ",
				tst_strerrno(expected_errno));
	} else {
		tst_res(TFAIL | TTERRNO,
				"adjtimex(): returned %ld", TST_RET);
	}

	SAFE_SETEUID(0);

	/* set the HZ from sysconf */
	hz = SAFE_SYSCONF(_SC_CLK_TCK);

	for (i = 0; i < ARRAY_SIZE(tc); i++) {
		if (tc[i].modes == ADJ_TICK) {
			tc[i].highlimit /= hz;
			tc[i].lowlimit /= hz;
		}
	}

	if ((adjtimex(tim_save)) == -1) {
		tst_brk(TBROK | TERRNO,
		"adjtimex(): failed to save current params");
	}
}

static void cleanup(void)
{

	tim_save->modes = SET_MODE;

	if ((adjtimex(tim_save)) == -1) {
		tst_brk(TBROK | TERRNO,
		"adjtimex(): failed to save current params");
	}
}

static struct tst_test test = {
	.test = verify_adjtimex,
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tc),
	.test_variants = ARRAY_SIZE(variants),
	.needs_root = 1,
	.bufs = (struct tst_buffers []) {
		 {&buf, .size = sizeof(*buf)},
		 {&tim_save, .size = sizeof(*tim_save)},
		 {},
		}
};
