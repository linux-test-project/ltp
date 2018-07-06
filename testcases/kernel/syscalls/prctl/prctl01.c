// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This is a Phase I test for the prctl(2) system call.
 * It is intended to provide a limited exposure of the system call.
 */

#include <errno.h>
#include <signal.h>
#include <sys/prctl.h>

#include "tst_test.h"

static void verify_prctl(void)
{
	int get_sig = 0;

	TEST(prctl(PR_SET_PDEATHSIG, SIGUSR2));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "prctl(PR_SET_PDEATHSIG) failed");
		return;
	}

	tst_res(TPASS, "prctl(PR_SET_PDEATHSIG) succeeded");

	TEST(prctl(PR_GET_PDEATHSIG, &get_sig));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "prctl(PR_GET_PDEATHSIG) failed");
		return;
	}

	if (get_sig == SIGUSR2) {
		tst_res(TPASS,
			"prctl(PR_GET_PDEATHSIG) got expected death signal");
	} else {
		tst_res(TFAIL, "prctl(PR_GET_PDEATHSIG) got death signal %d, expected %d",
			get_sig, SIGUSR2);
	}
}

static struct tst_test test = {
	.test_all = verify_prctl,
};
