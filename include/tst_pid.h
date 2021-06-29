/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_PID_H__
#define TST_PID_H__

#include <sys/types.h>

/*
 * Get a pid value not used by the OS
 */
pid_t tst_get_unused_pid_(void (*cleanup_fn)(void));

/*
 * Returns number of free pids by subtraction of the number of pids
 * currently used ('ps -eT') from maximum number of processes.
 * The limit of processes come from kernel pid_max and cgroup session limits
 * (e.g. configured by systemd user.slice).
 */
int tst_get_free_pids_(void (*cleanup_fn)(void));

#ifdef TST_TEST_H__
static inline pid_t tst_get_unused_pid(void)
{
	return tst_get_unused_pid_(NULL);
}

static inline int tst_get_free_pids(void)
{
	return tst_get_free_pids_(NULL);
}
#else
static inline pid_t tst_get_unused_pid(void (*cleanup_fn)(void))
{
	return tst_get_unused_pid_(cleanup_fn);
}

static inline int tst_get_free_pids(void (*cleanup_fn)(void))
{
	return tst_get_free_pids_(cleanup_fn);
}
#endif

#endif /* TST_PID_H__ */
