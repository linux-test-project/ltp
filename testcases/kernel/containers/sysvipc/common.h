// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2007
 *               Rishikesh K Rajak <risrajak@in.ibm.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/sched.h"

enum {
	T_CLONE,
	T_UNSHARE,
	T_NONE,
};

static inline int get_clone_unshare_enum(const char *str_op)
{
	int use_clone;

	use_clone = T_NONE;

	if (!str_op || !strcmp(str_op, "none"))
		use_clone = T_NONE;
	else if (!strcmp(str_op, "clone"))
		use_clone = T_CLONE;
	else if (!strcmp(str_op, "unshare"))
		use_clone = T_UNSHARE;
	else
		tst_brk(TBROK, "Test execution mode <clone|unshare|none>");

	return use_clone;
}

static void clone_test(unsigned long clone_flags, void (*fn1)())
{
	const struct tst_clone_args clone_args = {
		.flags = clone_flags,
		.exit_signal = SIGCHLD,
	};
	int pid;

	pid = SAFE_CLONE(&clone_args);
	if (!pid) {
		fn1();
		exit(0);
	}
}

static void unshare_test(unsigned long clone_flags, void (*fn1)())
{
	int pid;

	pid = SAFE_FORK();
	if (!pid) {
		SAFE_UNSHARE(clone_flags);

		fn1();
		exit(0);
	}
}

static void plain_test(void (*fn1)())
{
	int pid;

	pid = SAFE_FORK();
	if (!pid) {
		fn1();
		exit(0);
	}
}

static void clone_unshare_test(int use_clone, unsigned long clone_flags, void (*fn1)())
{
	switch (use_clone) {
	case T_NONE:
		plain_test(fn1);
	break;
	case T_CLONE:
		clone_test(clone_flags, fn1);
	break;
	case T_UNSHARE:
		unshare_test(clone_flags, fn1);
	break;
	default:
		tst_brk(TBROK, "%s: bad use_clone option: %d", __FUNCTION__, use_clone);
	break;
	}
}

#endif
