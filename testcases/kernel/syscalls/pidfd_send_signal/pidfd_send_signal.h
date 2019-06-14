// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

#ifndef PIDFD_SEND_SIGNAL_H
#define PIDFD_SEND_SIGNAL_H

#include "tst_test.h"
#include "lapi/syscalls.h"

static void check_syscall_support(void)
{
	/* allow the tests to fail early */
	tst_syscall(__NR_pidfd_send_signal);
}

#ifndef HAVE_PIDFD_SEND_SIGNAL
static int pidfd_send_signal(int pidfd, int sig, siginfo_t *info,
				 unsigned int flags)
{
	tst_res(TINFO, "Testing syscall(__NR_pidfd_send_signal)");
	return tst_syscall(__NR_pidfd_send_signal, pidfd, sig, info, flags);
}
#endif /* HAVE_PIDFD_SEND_SIGNAL */

#endif /* PIDFD_SEND_SIGNAL_H */
