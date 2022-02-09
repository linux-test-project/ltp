/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdint.h>
#include <limits.h>
#include <errno.h>
#include <sys/syscall.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/futex.h"

#define DEFAULT_MSEC_TIMEOUT 10000

futex_t *tst_futexes;
unsigned int tst_max_futexes;

void tst_checkpoint_init(const char *file, const int lineno,
                         void (*cleanup_fn)(void))
{
	int fd;
	unsigned int page_size;

	if (tst_futexes) {
		tst_brkm_(file, lineno, TBROK, cleanup_fn,
			"checkpoints already initialized");
		return;
	}

	/*
	 * The parent test process is responsible for creating the temporary
	 * directory and therefore must pass non-zero cleanup (to remove the
	 * directory if something went wrong).
	 *
	 * We cannot do this check unconditionally because if we need to init
	 * the checkpoint from a binary that was started by exec() the
	 * tst_tmpdir_created() will return false because the tmpdir was
	 * created by parent. In this case we expect the subprogram can call
	 * the init as a first function with NULL as cleanup function.
	 */
	if (cleanup_fn && !tst_tmpdir_created()) {
		tst_brkm_(file, lineno, TBROK, cleanup_fn,
			"You have to create test temporary directory "
			"first (call tst_tmpdir())");
		return;
	}

	page_size = getpagesize();

	fd = SAFE_OPEN(cleanup_fn, "checkpoint_futex_base_file",
	               O_RDWR | O_CREAT, 0666);

	SAFE_FTRUNCATE(cleanup_fn, fd, page_size);

	tst_futexes = SAFE_MMAP(cleanup_fn, NULL, page_size,
	                    PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	tst_max_futexes = page_size / sizeof(uint32_t);

	SAFE_CLOSE(cleanup_fn, fd);
}

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
