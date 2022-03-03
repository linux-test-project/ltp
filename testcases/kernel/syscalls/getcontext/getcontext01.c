// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2005.  All Rights Reserved.
 *               Author: Prashant P Yendigeri <prashant.yendigeri@wipro.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test getting user context functionality.
 */

#include "config.h"
#include "tst_test.h"

#ifdef HAVE_GETCONTEXT

#include <ucontext.h>

static void run(void)
{
	ucontext_t ptr;

	TST_EXP_PASS(getcontext(&ptr));
}

static struct tst_test test = {
	.test_all = run,
};

#else
TST_TEST_TCONF("system doesn't have getcontext support");
#endif
