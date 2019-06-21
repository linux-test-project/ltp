// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

/*
 * clock_adjtime() syscall might have as execution path:
 *
 *   1) a regular POSIX clock (only REALTIME clock implements adjtime())
 *      - will behave exactly like adjtimex() system call.
 *      - only one being tested here.
 *
 *   2) a dynamic POSIX clock (which ops are implemented by PTP clocks)
 *      - will trigger the PTP clock driver function "adjtime()"
 *      - different implementations from one PTP clock to another
 *      - might return EOPNOTSUPP (like ptp_kvm_caps, for example)
 *      - no entry point for clock_adjtime(), missing "CLOCK_PTP" model
 *
 * so it is sane to check for the following errors:
 *
 *   EINVAL -  clock id being used does not exist
 *
 *   EFAULT - (struct timex *) does not point to valid memory
 *
 *   EINVAL - ADJ_OFFSET + .offset outside range -512000 < x < 512000
 *            (after 2.6.26, kernels normalize to the limit if outside range)
 *
 *   EINVAL - ADJ_FREQUENCY + .freq outside range -32768000 < x < 3276800
 *            (after 2.6.26, kernels normalize to the limit if outside range)
 *
 *   EINVAL - .tick outside permitted range (900000/HZ < .tick < 1100000/HZ)
 *
 *   EPERM  - .modes is neither 0 nor ADJ_OFFSET_SS_READ (CAP_SYS_TIME required)
 *
 *   EINVAL - .status other than those listed bellow
 *
 * For ADJ_STATUS, consider the following flags:
 *
 *      rw  STA_PLL - enable phase-locked loop updates (ADJ_OFFSET)
 *      rw  STA_PPSFREQ - enable PPS (pulse-per-second) freq discipline
 *      rw  STA_PPSTIME - enable PPS time discipline
 *      rw  STA_FLL - select freq-locked loop mode.
 *      rw  STA_INS - ins leap sec after the last sec of UTC day (all days)
 *      rw  STA_DEL - del leap sec at last sec of UTC day (all days)
 *      rw  STA_UNSYNC - clock unsynced
 *      rw  STA_FREQHOLD - hold freq. ADJ_OFFSET made w/out auto small adjs
 *      ro  STA_PPSSIGNAL - valid PPS (pulse-per-second) signal is present
 *      ro  STA_PPSJITTER - PPS signal jitter exceeded.
 *      ro  STA_PPSWANDER - PPS signal wander exceeded.
 *      ro  STA_PPSERROR - PPS signal calibration error.
 *      ro  STA_CLOKERR - clock HW fault.
 *      ro  STA_NANO - 0 = us, 1 = ns (set = ADJ_NANO, cl = ADJ_MICRO)
 *      rw  STA_MODE - mode: 0 = phased locked loop. 1 = freq locked loop
 *      ro  STA_CLK - clock source. unused.
 */

#include "clock_adjtime.h"

static long hz;
static struct timex saved, ttxc;
static int supported;

static void cleanup(void);

struct test_case {
	clockid_t clktype;
	unsigned int modes;
	long lowlimit;
	long highlimit;
	long *ptr;
	long delta;
	int exp_err;
	int droproot;
};

struct test_case tc[] = {
	{
	 .clktype = MAX_CLOCKS,
	 .exp_err = EINVAL,
	},
	{
	 .clktype = MAX_CLOCKS + 1,
	 .exp_err = EINVAL,
	},
	{
	 .clktype = CLOCK_REALTIME,
	 .modes = ADJ_ALL,
	 .exp_err = EFAULT,
	},
	{
	 .clktype = CLOCK_REALTIME,
	 .modes = ADJ_TICK,
	 .lowlimit = 900000,
	 .ptr = &ttxc.tick,
	 .delta = 1,
	 .exp_err = EINVAL,
	},
	{
	 .clktype = CLOCK_REALTIME,
	 .modes = ADJ_TICK,
	 .highlimit = 1100000,
	 .ptr = &ttxc.tick,
	 .delta = 1,
	 .exp_err = EINVAL,
	},
	{
	 .clktype = CLOCK_REALTIME,
	 .modes = ADJ_ALL,
	 .exp_err = EPERM,
	 .droproot = 1,
	},
};

static void verify_clock_adjtime(unsigned int i)
{
	uid_t whoami = 0;
	struct timex *txcptr;
	struct passwd *nobody;
	static const char name[] = "nobody";

	txcptr = &ttxc;

	memset(txcptr, 0, sizeof(struct timex));

	SAFE_CLOCK_ADJTIME(CLOCK_REALTIME, txcptr);
	timex_show("GET", *txcptr);

	if (tc[i].droproot) {
		nobody = SAFE_GETPWNAM(name);
		whoami = nobody->pw_uid;
		SAFE_SETEUID(whoami);
	}

	txcptr->modes = tc[i].modes;

	if (tc[i].ptr) {

		if (tc[i].lowlimit)
			*tc[i].ptr = tc[i].lowlimit - tc[i].delta;

		if (tc[i].highlimit)
			*tc[i].ptr = tc[i].highlimit + tc[i].delta;
	}

	/* special case: EFAULT for bad addresses */
	if (tc[i].exp_err == EFAULT)
		txcptr = tst_get_bad_addr(cleanup);

	TEST(sys_clock_adjtime(tc[i].clktype, txcptr));
	if (txcptr && tc[i].exp_err != EFAULT)
		timex_show("TEST", *txcptr);

	if (TST_RET >= 0) {
		tst_res(TFAIL, "clock_adjtime(): passed unexpectedly (mode=%x, "
				"uid=%d)", tc[i].modes, whoami);
		return;
	}

	if (tc[i].exp_err != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "clock_adjtime(): expected %d but "
				"failed with %d (mode=%x, uid=%d)",
				tc[i].exp_err, TST_ERR, tc[i].modes, whoami);
		return;
	}

	tst_res(TPASS, "clock_adjtime(): failed as expected (mode=0x%x, "
			"uid=%d)", tc[i].modes, whoami);

	if (tc[i].droproot)
		SAFE_SETEUID(0);
}

static void setup(void)
{
	size_t i;
	int rval;

	rval = SAFE_CLOCK_ADJTIME(CLOCK_REALTIME, &saved);
	supported = 1;

	if (rval != TIME_OK && rval != TIME_ERROR) {
		timex_show("SAVE_STATUS", saved);
		tst_brk(TBROK | TTERRNO, "clock has on-going leap changes, "
				"returned: %i", rval);
	}

	hz = SAFE_SYSCONF(_SC_CLK_TCK);

	/* fix high and low limits by dividing it per HZ value */

	for (i = 0; i < ARRAY_SIZE(tc); i++) {
		if (tc[i].modes == ADJ_TICK) {
			tc[i].highlimit /= hz;
			tc[i].lowlimit /= hz;
		}
	}
}

static void cleanup(void)
{
	if (supported == 0)
		return;

	saved.modes = ADJ_ALL;

	/* restore clock resolution based on original status flag */

	if (saved.status & STA_NANO)
		saved.modes |= ADJ_NANO;
	else
		saved.modes |= ADJ_MICRO;

	/* restore original clock flags */

	SAFE_CLOCK_ADJTIME(CLOCK_REALTIME, &saved);
}

static struct tst_test test = {
	.test = verify_clock_adjtime,
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tc),
	.needs_root = 1,
	.restore_wallclock = 1,
};
