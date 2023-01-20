// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 SUSE LLC <mdoucha@suse.cz>
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_safe_io_uring.h"

int safe_io_uring_init(const char *file, const int lineno,
	unsigned int entries, struct io_uring_params *params,
	struct tst_io_uring *uring)
{
	errno = 0;
	uring->fd = io_uring_setup(entries, params);

	if (uring->fd == -1) {
		if (errno == EOPNOTSUPP)
			tst_brk(TCONF, "CONFIG_IO_URING is not enabled");

		tst_brk_(file, lineno, TBROK | TERRNO,
			"io_uring_setup() failed");
		return uring->fd;
	} else if (uring->fd < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"io_uring_setup() returned invalid value %d",
			uring->fd);
		return uring->fd;
	}

	uring->sqr_size = params->sq_entries;
	uring->cqr_size = params->cq_entries;
	uring->sqr_mapsize = params->sq_off.array +
		params->sq_entries * sizeof(__u32);
	uring->cqr_mapsize = params->cq_off.cqes +
		params->cq_entries * sizeof(struct io_uring_cqe);

	uring->sqr_base = safe_mmap(file, lineno, NULL, uring->sqr_mapsize,
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, uring->fd,
		IORING_OFF_SQ_RING);

	if (uring->sqr_base == MAP_FAILED)
		return -1;

	uring->sqr_entries = safe_mmap(file, lineno, NULL,
		params->sq_entries * sizeof(struct io_uring_sqe),
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, uring->fd,
		IORING_OFF_SQES);

	if (uring->sqr_entries == MAP_FAILED)
		return -1;

	uring->cqr_base = safe_mmap(file, lineno, NULL, uring->cqr_mapsize,
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, uring->fd,
		IORING_OFF_CQ_RING);

	if (uring->cqr_base == MAP_FAILED)
		return -1;

	uring->sqr_head = uring->sqr_base + params->sq_off.head;
	uring->sqr_tail = uring->sqr_base + params->sq_off.tail;
	uring->sqr_mask = uring->sqr_base + params->sq_off.ring_mask;
	uring->sqr_flags = uring->sqr_base + params->sq_off.flags;
	uring->sqr_dropped = uring->sqr_base + params->sq_off.dropped;
	uring->sqr_array = uring->sqr_base + params->sq_off.array;

	uring->cqr_head = uring->cqr_base + params->cq_off.head;
	uring->cqr_tail = uring->cqr_base + params->cq_off.tail;
	uring->cqr_mask = uring->cqr_base + params->cq_off.ring_mask;
	uring->cqr_overflow = uring->cqr_base + params->cq_off.overflow;
	uring->cqr_entries = uring->cqr_base + params->cq_off.cqes;
	return uring->fd;
}

int safe_io_uring_close(const char *file, const int lineno,
	struct tst_io_uring *uring)
{
	int ret;

	safe_munmap(file, lineno, NULL, uring->cqr_base, uring->cqr_mapsize);
	safe_munmap(file, lineno, NULL, uring->sqr_entries,
		uring->sqr_size * sizeof(struct io_uring_sqe));
	safe_munmap(file, lineno, NULL, uring->sqr_base, uring->sqr_mapsize);
	ret = safe_close(file, lineno, NULL, uring->fd);
	uring->fd = -1;
	return ret;
}

int safe_io_uring_enter(const char *file, const int lineno, int strict,
	int fd, unsigned int to_submit, unsigned int min_complete,
	unsigned int flags, sigset_t *sig)
{
	int ret;

	errno = 0;
	ret = io_uring_enter(fd, to_submit, min_complete, flags, sig);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"io_uring_enter() failed");
	} else if (ret < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid io_uring_enter() return value %d", ret);
	} else if (strict && to_submit != (unsigned int)ret) {
		tst_brk_(file, lineno, TBROK,
			"io_uring_enter() submitted %d items (expected %d)",
			ret, to_submit);
	}

	return ret;
}
