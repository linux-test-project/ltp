/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2012-2014 Cyril Hrubis chrubis@suse.cz
 * Copyright (C) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*
 * These functions helps you wait till a process with given pid changes state.
 * This is for example useful when you need to wait in parent until child blocks.
 */

#ifndef TST_PROCESS_STATE__
#define TST_PROCESS_STATE__

#include <unistd.h>

#ifdef TST_TEST_H__

/*
 * Waits for process state change.
 *
 * The state is one of the following:
 *
 * R - process is running
 * S - process is sleeping
 * D - process sleeping uninterruptibly
 * Z - zombie process
 * T - process is traced
 */
#define TST_PROCESS_STATE_WAIT(pid, state, msec_timeout) \
	tst_process_state_wait(__FILE__, __LINE__, NULL, \
			(pid), (state), (msec_timeout))

/*
 * Check that a given pid is present on the system
 */
#define TST_PROCESS_EXIT_WAIT(pid, msec_timeout) \
	tst_process_exit_wait((pid), (msec_timeout))

/*
 * Waits for thread state change.
 *
 * The state is one of the following:
 *
 * R - running
 * S - sleeping
 * D - disk sleep
 * T - stopped
 * t - tracing stopped
 * Z - zombie
 * X - dead
 */
#define TST_THREAD_STATE_WAIT(tid, state, msec_timeout) \
	tst_thread_state_wait((tid), (state), (msec_timeout))

int tst_thread_state_wait(pid_t tid, const char state,
				unsigned int msec_timeout);

#else
/*
 * The same as above but does not use tst_brkm() interface.
 *
 * This function is intended to be used from child processes.
 *
 * Returns zero on success, non-zero on failure.
 */
int tst_process_state_wait2(pid_t pid, const char state);

#define TST_PROCESS_STATE_WAIT(cleanup_fn, pid, state) \
	tst_process_state_wait(__FILE__, __LINE__, (cleanup_fn), \
			(pid), (state), 0)
#endif

int tst_process_state_wait(const char *file, const int lineno,
			   void (*cleanup_fn)(void), pid_t pid,
			   const char state, unsigned int msec_timeout);
int tst_process_exit_wait(pid_t pid, unsigned int msec_timeout);

#endif /* TST_PROCESS_STATE__ */
