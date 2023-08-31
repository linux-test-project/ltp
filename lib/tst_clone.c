// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2021 SUSE LLC
 * Richard Palethorpe <rpalethorpe@suse.com>
 */

#define TST_NO_DEFAULT_MAIN

#include <stddef.h>

#include "tst_test.h"
#include "lapi/sched.h"

pid_t tst_clone(const struct tst_clone_args *tst_args)
{
	struct clone_args args = {
		.flags = tst_args->flags,
		.exit_signal = tst_args->exit_signal,
		.cgroup = tst_args->cgroup,
	};
	int flags;
	pid_t pid = -1;

	tst_flush();

	errno = ENOSYS;
	if (__NR_clone3 != __LTP__NR_INVALID_SYSCALL)
		pid = syscall(__NR_clone3, &args, sizeof(args));

	if (pid == -1 && errno != ENOSYS)
		return -1;

	if (pid != -1)
		return pid;

	flags = args.exit_signal | args.flags;

#ifdef __s390x__
	pid = syscall(__NR_clone, NULL, flags);
#else
	pid = syscall(__NR_clone, flags, NULL);
#endif

	if (pid == -1)
		return -2;

	return pid;
}
