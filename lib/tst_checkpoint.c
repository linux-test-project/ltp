// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2016-2025
 */

#include <stdint.h>
#include <limits.h>
#include <errno.h>
#include <sys/syscall.h>

#include "test.h"
#include "tso_safe_macros.h"
#include "lapi/futex.h"

#define DEFAULT_MSEC_TIMEOUT 10000

/*
 * Global futex array and size for checkpoint synchronization.
 *
 * NOTE: These are initialized by setup_ipc()/tst_reinit() in tst_test.c
 * when .needs_checkpoints is set in the tst_test struct.
 */
futex_t *tst_futexes;
unsigned int tst_max_futexes;

int tst_checkpoint_wait(unsigned int id, unsigned int msec_timeout)
{
	struct timespec timeout;
	int ret;

	if (!tst_max_futexes)
		tst_brkm(TBROK, NULL, "Set test.needs_checkpoints = 1");

	if (id >= tst_max_futexes) {
		errno = EOVERFLOW;
		return -1;
	}

	timeout.tv_sec = msec_timeout/1000;
	timeout.tv_nsec = (msec_timeout%1000) * 1000000;

	do {
		ret = syscall(SYS_futex, &tst_futexes[id], FUTEX_WAIT,
			      tst_futexes[id], &timeout);
	} while (ret == -1 && errno == EINTR);

	return ret;
}

int tst_checkpoint_wake(unsigned int id, unsigned int nr_wake,
                        unsigned int msec_timeout)
{
	unsigned int msecs = 0, waked = 0;

	if (!tst_max_futexes)
		tst_brkm(TBROK, NULL, "Set test.needs_checkpoints = 1");

	if (id >= tst_max_futexes) {
		errno = EOVERFLOW;
		return -1;
	}

	for (;;) {
		waked += syscall(SYS_futex, &tst_futexes[id], FUTEX_WAKE,
				 INT_MAX, NULL);

		if (waked == nr_wake)
			break;

		usleep(1000);
		msecs++;

		if (msecs >= msec_timeout) {
			errno = ETIMEDOUT;
			return -1;
		}
	}

	return 0;
}

void tst_safe_checkpoint_wait(const char *file, const int lineno,
                              void (*cleanup_fn)(void), unsigned int id,
			      unsigned int msec_timeout)
{
	int ret;

	if (!msec_timeout)
		msec_timeout = DEFAULT_MSEC_TIMEOUT;

	ret = tst_checkpoint_wait(id, msec_timeout);

	if (ret) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"tst_checkpoint_wait(%u, %i) failed", id,
			msec_timeout);
	}
}

void tst_safe_checkpoint_wake(const char *file, const int lineno,
                              void (*cleanup_fn)(void), unsigned int id,
                              unsigned int nr_wake)
{
	int ret = tst_checkpoint_wake(id, nr_wake, DEFAULT_MSEC_TIMEOUT);

	if (ret) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"tst_checkpoint_wake(%u, %u, %i) failed", id, nr_wake,
			DEFAULT_MSEC_TIMEOUT);
	}
}
