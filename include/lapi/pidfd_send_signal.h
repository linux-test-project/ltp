// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

#ifndef LAPI_PIDFD_SEND_SIGNAL_H__
#define LAPI_PIDFD_SEND_SIGNAL_H__

#include "tst_test.h"
#include "lapi/syscalls.h"

static inline void pidfd_send_signal_supported(void)
{
	/* allow the tests to fail early */
	tst_syscall(__NR_pidfd_send_signal);
}

#ifndef HAVE_PIDFD_SEND_SIGNAL
static inline int pidfd_send_signal(int pidfd, int sig, siginfo_t *info,
                                    unsigned int flags)
{
	return tst_syscall(__NR_pidfd_send_signal, pidfd, sig, info, flags);
}
#endif /* HAVE_PIDFD_SEND_SIGNAL */

#endif /* LAPI_PIDFD_SEND_SIGNAL_H__ */
