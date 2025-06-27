// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015-2025 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_CHECKPOINT_FN__
#define TST_CHECKPOINT_FN__

/*
 * Waits for wakeup.
 *
 * @id: Checkpoint id, positive number
 * @msec_timeout: Timeout in milliseconds, 0 == no timeout
 */
int tst_checkpoint_wait(unsigned int id, unsigned int msec_timeout);

/*
 * Wakes up sleeping process(es)/thread(s).
 *
 * @id: Checkpoint id, positive number
 * @nr_wake: Number of processes/threads to wake up
 * @msec_timeout: Timeout in milliseconds, 0 == no timeout
 */
int tst_checkpoint_wake(unsigned int id, unsigned int nr_wake,
                        unsigned int msec_timeout);

void tst_safe_checkpoint_wait(const char *file, const int lineno,
                              void (*cleanup_fn)(void), unsigned int id,
			      unsigned int msec_timeout);

void tst_safe_checkpoint_wake(const char *file, const int lineno,
                              void (*cleanup_fn)(void), unsigned int id,
                              unsigned int nr_wake);

#endif /* TST_CHECKPOINT_FN__ */
