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

/**
 * TST_PROCESS_STATE_WAIT() - Waits for a process state change.
 *
 * @pid: A process pid.
 * @state: A state to wait for.
 * @msec_timeout: A timeout for the wait.
 *
 * Polls `/proc/$PID/state` for a process state changes.
 *
 * Possible process states (see :manpage:`ps(1)`):
 *
 * - **R** Process is running.
 * - **S** Process is sleeping.
 * - **D** Process sleeping uninterruptibly.
 * - **Z** Zombie process.
 * - **T** Process is traced.
 * - **t** Tracing stopped.
 * - **X** Process id dead.
 */
#define TST_PROCESS_STATE_WAIT(pid, state, msec_timeout) \
	tst_process_state_wait(__FILE__, __LINE__, NULL, \
			(pid), (state), (msec_timeout))

/**
 * TST_PROCESS_EXIT_WAIT() - Waits while pid is present on the system.
 *
 * Loops until `kill($PID, 0)` succeds or timeout is reached.
 *
 * @pid: A process pid.
 * @msec_timeout: A timeout for the wait.
 */
#define TST_PROCESS_EXIT_WAIT(pid, msec_timeout) \
	tst_process_exit_wait((pid), (msec_timeout))

/**
 * TST_THREAD_STATE_WAIT() - Waits for a thread state change.
 *
 * Polls `/proc/self/task/$TID/state` for a thread state change.
 *
 * Possible thread states are the same as for TST_PROCESS_STATE_WAIT().
 *
 * @tid: A thread tid.
 * @state: A state to wait for.
 * @msec_timeout: A timeout for the wait.
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
