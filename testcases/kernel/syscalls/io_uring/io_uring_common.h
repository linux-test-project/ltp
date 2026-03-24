// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 IBM
 * Author: Sachin Sant <sachinp@linux.ibm.com>
 *
 * Common definitions and helper functions for io_uring tests
 */

#ifndef IO_URING_COMMON_H
#define IO_URING_COMMON_H

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "config.h"
#include "tst_test.h"
#include "lapi/io_uring.h"

/* Common structures for io_uring ring management */
struct io_sq_ring {
	unsigned int *head;
	unsigned int *tail;
	unsigned int *ring_mask;
	unsigned int *ring_entries;
	unsigned int *flags;
	unsigned int *array;
};

struct io_cq_ring {
	unsigned int *head;
	unsigned int *tail;
	unsigned int *ring_mask;
	unsigned int *ring_entries;
	struct io_uring_cqe *cqes;
};

struct io_uring_submit {
	int ring_fd;
	struct io_sq_ring sq_ring;
	struct io_uring_sqe *sqes;
	struct io_cq_ring cq_ring;
	void *sq_ptr;
	size_t sq_ptr_size;
	void *cq_ptr;
	size_t cq_ptr_size;
};

/*
 * Setup io_uring instance with specified queue depth and optional flags
 * Returns 0 on success, -1 on failure
 */
static inline int io_uring_setup_queue(struct io_uring_submit *s,
				       unsigned int queue_depth,
				       unsigned int flags)
{
	struct io_sq_ring *sring = &s->sq_ring;
	struct io_cq_ring *cring = &s->cq_ring;
	struct io_uring_params p;

	memset(&p, 0, sizeof(p));
	p.flags = flags;
	s->ring_fd = io_uring_setup(queue_depth, &p);
	if (s->ring_fd < 0) {
		tst_brk(TBROK | TERRNO, "io_uring_setup() failed");
		return -1;
	}

	s->sq_ptr_size = p.sq_off.array + p.sq_entries * sizeof(unsigned int);

	s->sq_ptr = SAFE_MMAP(0, s->sq_ptr_size, PROT_READ | PROT_WRITE,
			      MAP_SHARED | MAP_POPULATE, s->ring_fd,
			      IORING_OFF_SQ_RING);

	/* Save submission queue pointers */
	sring->head = s->sq_ptr + p.sq_off.head;
	sring->tail = s->sq_ptr + p.sq_off.tail;
	sring->ring_mask = s->sq_ptr + p.sq_off.ring_mask;
	sring->ring_entries = s->sq_ptr + p.sq_off.ring_entries;
	sring->flags = s->sq_ptr + p.sq_off.flags;
	sring->array = s->sq_ptr + p.sq_off.array;

	s->sqes = SAFE_MMAP(0, p.sq_entries * sizeof(struct io_uring_sqe),
			    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
			    s->ring_fd, IORING_OFF_SQES);

	s->cq_ptr_size = p.cq_off.cqes +
			 p.cq_entries * sizeof(struct io_uring_cqe);

	s->cq_ptr = SAFE_MMAP(0, s->cq_ptr_size, PROT_READ | PROT_WRITE,
			      MAP_SHARED | MAP_POPULATE, s->ring_fd,
			      IORING_OFF_CQ_RING);

	/* Save completion queue pointers */
	cring->head = s->cq_ptr + p.cq_off.head;
	cring->tail = s->cq_ptr + p.cq_off.tail;
	cring->ring_mask = s->cq_ptr + p.cq_off.ring_mask;
	cring->ring_entries = s->cq_ptr + p.cq_off.ring_entries;
	cring->cqes = s->cq_ptr + p.cq_off.cqes;

	return 0;
}

/*
 * Cleanup io_uring instance and unmap all memory regions
 */
static inline void io_uring_cleanup_queue(struct io_uring_submit *s,
					  unsigned int queue_depth)
{
	if (s->sqes)
		SAFE_MUNMAP(s->sqes, queue_depth * sizeof(struct io_uring_sqe));
	if (s->cq_ptr)
		SAFE_MUNMAP(s->cq_ptr, s->cq_ptr_size);
	if (s->sq_ptr)
		SAFE_MUNMAP(s->sq_ptr, s->sq_ptr_size);
	if (s->ring_fd > 0)
		SAFE_CLOSE(s->ring_fd);
}

/*
 * Internal helper to submit a single SQE to the submission queue
 * Used by both vectored and non-vectored I/O operations
 */
static inline void io_uring_submit_sqe_internal(struct io_uring_submit *s,
						int fd, int opcode,
						unsigned long addr,
						unsigned int len,
						off_t offset,
						unsigned long long user_data)
{
	struct io_sq_ring *sring = &s->sq_ring;
	unsigned int tail, index;
	struct io_uring_sqe *sqe;

	tail = *sring->tail;
	index = tail & *sring->ring_mask;
	sqe = &s->sqes[index];

	memset(sqe, 0, sizeof(*sqe));
	sqe->opcode = opcode;
	sqe->fd = fd;
	sqe->addr = addr;
	sqe->len = len;
	sqe->off = offset;
	sqe->user_data = user_data;

	sring->array[index] = index;
	tail++;

	*sring->tail = tail;
}

/*
 * Submit a single SQE to the submission queue
 * For basic read/write operations (non-vectored)
 */
static inline void io_uring_submit_sqe(struct io_uring_submit *s, int fd,
				       int opcode, void *buf, size_t len,
				       off_t offset)
{
	io_uring_submit_sqe_internal(s, fd, opcode, (unsigned long)buf,
				     len, offset, opcode);
}

/*
 * Submit a vectored SQE to the submission queue
 * For readv/writev operations
 */
static inline void io_uring_submit_sqe_vec(struct io_uring_submit *s, int fd,
					   int opcode, struct iovec *iovs,
					   int nr_vecs, off_t offset)
{
	io_uring_submit_sqe_internal(s, fd, opcode, (unsigned long)iovs,
				     nr_vecs, offset, opcode);
}

/*
 * Map io_uring operation code to human-readable name
 */
static inline const char *ioring_op_name(int op)
{
	switch (op) {
	case IORING_OP_READV:
		return "IORING_OP_READV";
	case IORING_OP_WRITEV:
		return "IORING_OP_WRITEV";
	case IORING_OP_READ:
		return "IORING_OP_READ";
	case IORING_OP_WRITE:
		return "IORING_OP_WRITE";
	default:
		return "UNKNOWN";
	}
}

/*
 * Wait for and validate a completion queue entry
 * Aborts test on failure using tst_brk()
 */
static inline void io_uring_wait_cqe(struct io_uring_submit *s,
				     int expected_res, int expected_opcode,
				     sigset_t *sig)
{
	struct io_cq_ring *cring = &s->cq_ring;
	struct io_uring_cqe *cqe;
	unsigned int head;
	int ret;

	ret = io_uring_enter(s->ring_fd, 1, 1, IORING_ENTER_GETEVENTS, sig);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "io_uring_enter() failed");

	head = *cring->head;
	if (head == *cring->tail)
		tst_brk(TBROK, "No completion event received");

	cqe = &cring->cqes[head & *cring->ring_mask];

	if (cqe->user_data != (uint64_t)expected_opcode) {
		*cring->head = head + 1;
		tst_brk(TBROK, "Unexpected user_data: got %llu, expected %d",
			(unsigned long long)cqe->user_data, expected_opcode);
	}

	if (cqe->res != expected_res) {
		*cring->head = head + 1;
		tst_brk(TBROK, "Operation failed: res=%d, expected=%d",
			cqe->res, expected_res);
	}

	*cring->head = head + 1;
}

/*
 * Initialize buffer with a repeating character pattern
 * Useful for creating test data with predictable patterns
 */
static inline void io_uring_init_buffer_pattern(char *buf, size_t size,
						char pattern)
{
	size_t i;

	for (i = 0; i < size; i++)
		buf[i] = pattern;
}

/*
 * Submit and wait for a non-vectored I/O operation
 * Combines io_uring_submit_sqe() and io_uring_wait_cqe() with result reporting
 */
static inline void io_uring_do_io_op(struct io_uring_submit *s, int fd,
				     int op, void *buf, size_t len,
				     off_t offset, sigset_t *sig)
{
	io_uring_submit_sqe(s, fd, op, buf, len, offset);
	io_uring_wait_cqe(s, len, op, sig);
	tst_res(TPASS, "OP=%s (%02x) fd=%i buf=%p len=%zu offset=%jd",
		ioring_op_name(op), op, fd, buf, len, (intmax_t)offset);
}

/*
 * Submit and wait for a vectored I/O operation
 * Combines io_uring_submit_sqe_vec() and io_uring_wait_cqe() with
 * result reporting
 */
static inline void io_uring_do_vec_io_op(struct io_uring_submit *s, int fd,
					 int op, struct iovec *iovs,
					 int nvecs, off_t offset,
					 int expected_size, sigset_t *sig)
{
	io_uring_submit_sqe_vec(s, fd, op, iovs, nvecs, offset);
	io_uring_wait_cqe(s, expected_size, op, sig);
	tst_res(TPASS, "OP=%s (%02x) fd=%i iovs=%p nvecs=%i offset=%jd "
		"expected_size=%i",
		ioring_op_name(op), op, fd, iovs, nvecs, (intmax_t)offset,
		expected_size);
}

#endif /* IO_URING_COMMON_H */
