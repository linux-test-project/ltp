/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) Linux Test Project, 2021
 */

#ifndef TST_IO_URING_H__
#define TST_IO_URING_H__

#include "config.h"
#include "lapi/io_uring.h"

struct tst_io_uring {
	int fd;
	void *sqr_base, *cqr_base;
	/* buffer sizes in bytes for unmapping */
	size_t sqr_mapsize, cqr_mapsize;

	/* Number of entries in the ring buffers */
	uint32_t sqr_size, cqr_size;

	/* Submission queue pointers */
	struct io_uring_sqe *sqr_entries;
	const uint32_t *sqr_head, *sqr_mask, *sqr_flags, *sqr_dropped;
	uint32_t *sqr_tail, *sqr_array;

	/* Completion queue pointers */
	const struct io_uring_cqe *cqr_entries;
	const uint32_t *cqr_tail, *cqr_mask, *cqr_overflow;
	uint32_t *cqr_head;
};

/*
 * Call io_uring_setup() with given arguments and prepare memory mappings
 * into the tst_io_uring structure passed in the third argument.
 */
#define SAFE_IO_URING_INIT(entries, params, uring) \
	safe_io_uring_init(__FILE__, __LINE__, (entries), (params), (uring))
int safe_io_uring_init(const char *file, const int lineno,
	unsigned int entries, struct io_uring_params *params,
	struct tst_io_uring *uring);

/*
 * Release io_uring mappings and close the file descriptor. uring->fd will
 * be set to -1 after close.
 */
#define SAFE_IO_URING_CLOSE(uring) \
	safe_io_uring_close(__FILE__, __LINE__, (uring))
int safe_io_uring_close(const char *file, const int lineno,
	struct tst_io_uring *uring);

/*
 * Call io_uring_enter() and check for errors. The "strict" argument controls
 * pedantic check whether return value is equal to "to_submit" argument.
 */
#define SAFE_IO_URING_ENTER(strict, fd, to_submit, min_complete, flags, sig) \
	safe_io_uring_enter(__FILE__, __LINE__, (strict), (fd), (to_submit), \
		(min_complete), (flags), (sig))
int safe_io_uring_enter(const char *file, const int lineno, int strict,
	int fd, unsigned int to_submit, unsigned int min_complete,
	unsigned int flags, sigset_t *sig);

#endif /* TST_IO_URING_H__ */
