// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2025 Cyril Hrubis <chrubis@suse.cz>
 */

/**
 * DOC: Checkpoints introduction
 *
 * Checkpoints implements a futex based synchronization primitive for threads
 * and processes. When a process calls wait function its execution is suspended
 * until wake is called for a corresponding checkpoint. Checkpoints are
 * numbered from 0 and process can use at least hundred of them.
 *
 * In order to use checkpoints the test must set the tst_test.needs_checkpoints
 * flag.
 */

#ifndef TST_CHECKPOINT__
#define TST_CHECKPOINT__

#include "tst_checkpoint_fn.h"

/**
 * TST_CHECKPOINT_WAIT() - Waits for a checkpoint.
 *
 * @id: A checkpoint id a positive integer.
 *
 * Suspends thread/process execution until it's woken up with a wake. The call
 * does not wait indefinitely it gives up after 10 seconds. If an error
 * happened or timeout was reached the function calls tst_brk(TBROK, ...) which
 * exits the test.
 */
#define TST_CHECKPOINT_WAIT(id) \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, NULL, id, 0)

/**
 * TST_CHECKPOINT_WAIT2() - Waits for a checkpoint.
 *
 * @id: A checkpoint id a positive integer.
 * @msec_timeout: A timeout.
 *
 * Suspends thread/process execution until it's woken up with a wake. If an
 * error happened or timeout was reached the function calls tst_brk(TBROK, ...)
 * which exits the test.
 */
#define TST_CHECKPOINT_WAIT2(id, msec_timeout) \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, NULL, id, msec_timeout)

/**
 * TST_CHECKPOINT_WAKE() - Wakes up a checkpoint.
 *
 * @id: A checkpoint id a positive integer.
 *
 * Wakes up a process suspended on a checkpoint and retries if there is no
 * process suspended on the checkpoint yet. The call does not retry
 * indefinitely but gives up after 10 seconds. If an error happened or timeout
 * was reached the function calls tst_brk(TBROK, ...) which exits the test.
 */
#define TST_CHECKPOINT_WAKE(id) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, NULL, id, 1)

/**
 * TST_CHECKPOINT_WAKE2() - Wakes up several checkpoints.
 *
 * @id: A checkpoint id a positive integer.
 * @nr_wake: A number of processes to wake.
 *
 * Wakes up nr_wake processes suspended on a checkpoint and retries if there
 * wasn't enough process suspended on the checkpoint yet. The call does not
 * retry indefinitely but gives up if it does not wake nr_wake processes after
 * 10 seconds. If an error happened or timeout was reached the function calls
 * tst_brk(TBROK, ...) which exits the test.
 */
#define TST_CHECKPOINT_WAKE2(id, nr_wake) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, NULL, id, nr_wake)

/**
 * TST_CHECKPOINT_WAKE_AND_WAIT() - Wakes up a checkpoint and immediately waits on it.
 *
 * @id: A checkpoint id a positive integer.
 *
 * This is a combination of TST_CHECKPOINT_WAKE() and TST_CHECKPOINT_WAIT().
 */
#define TST_CHECKPOINT_WAKE_AND_WAIT(id) do { \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, NULL, id, 1); \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, NULL, id, 0); \
} while (0)

#endif /* TST_CHECKPOINT__ */
