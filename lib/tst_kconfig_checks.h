// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_KCONFIG_CHECKS_H
#define TST_KCONFIG_CHECKS_H

#include <unistd.h>
#include <stdbool.h>

static inline bool tst_user_ns_enabled(void)
{
	return access("/proc/self/ns/user", F_OK) == 0;
}

static inline bool tst_net_ns_enabled(void)
{
	return access("/proc/self/ns/net", F_OK) == 0;
}

static inline bool tst_pid_ns_enabled(void)
{
	return access("/proc/self/ns/pid", F_OK) == 0;
}

static inline bool tst_mnt_ns_enabled(void)
{
	return access("/proc/self/ns/mnt", F_OK) == 0;
}

static inline bool tst_ipc_ns_enabled(void)
{
	return access("/proc/self/ns/ipc", F_OK) == 0;
}

#endif /* TST_KCONFIG_CHECKS_H */
