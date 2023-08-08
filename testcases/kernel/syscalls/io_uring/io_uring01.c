// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 ARM Ltd. All rights reserved.
 * Author: Vikas Kumar <vikas.kumar2@arm.com>
 *
 * Copyright (C) 2020 Cyril Hrubis <chrubis@suse.cz>
 *
 * Tests for asynchronous I/O raw API i.e io_uring_setup(), io_uring_register()
 * and io_uring_enter(). This tests validate basic API operation by creating a
 * submission queue and a completion queue using io_uring_setup(). User buffer
 * registered in the kernel for long term operation using io_uring_register().
 * This tests initiates I/O operations with the help of io_uring_enter().
 */
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "config.h"
#include "tst_test.h"
#include "lapi/io_uring.h"

#define TEST_FILE "test_file"

#define QUEUE_DEPTH 1
#define BLOCK_SZ    1024

static struct tcase {
	unsigned int setup_flags;
	unsigned int register_opcode;
	unsigned int enter_flags;
} tcases[] = {
	{0, IORING_REGISTER_BUFFERS, IORING_OP_READ_FIXED},
};

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

struct submitter {
	int ring_fd;
	struct io_sq_ring sq_ring;
	struct io_uring_sqe *sqes;
	struct io_cq_ring cq_ring;
};

static struct submitter sub_ring;
static struct submitter *s = &sub_ring;
static sigset_t sig;
static struct iovec *iov;


static void *sptr;
static size_t sptr_size;
static void *cptr;
static size_t cptr_size;

static int setup_io_uring_test(struct submitter *s, struct tcase *tc)
{
	struct io_sq_ring *sring = &s->sq_ring;
	struct io_cq_ring *cring = &s->cq_ring;
	struct io_uring_params p;

	memset(&p, 0, sizeof(p));
	p.flags |= tc->setup_flags;
	s->ring_fd = io_uring_setup(QUEUE_DEPTH, &p);
	if (s->ring_fd != -1) {
		tst_res(TPASS, "io_uring_setup() passed");
	} else {
		tst_res(TFAIL | TERRNO, "io_uring_setup() failed");
		return 1;
	}

	sptr_size = p.sq_off.array + p.sq_entries * sizeof(unsigned int);

	/* Submission queue ring buffer mapping */
	sptr = SAFE_MMAP(0, sptr_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_POPULATE,
			s->ring_fd, IORING_OFF_SQ_RING);

	/* Save global submission queue struct info */
	sring->head = sptr + p.sq_off.head;
	sring->tail = sptr + p.sq_off.tail;
	sring->ring_mask = sptr + p.sq_off.ring_mask;
	sring->ring_entries = sptr + p.sq_off.ring_entries;
	sring->flags = sptr + p.sq_off.flags;
	sring->array = sptr + p.sq_off.array;

	/* Submission queue entries ring buffer mapping */
	s->sqes = SAFE_MMAP(0, p.sq_entries *
			sizeof(struct io_uring_sqe),
			PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_POPULATE,
			s->ring_fd, IORING_OFF_SQES);

	cptr_size = p.cq_off.cqes + p.cq_entries * sizeof(struct io_uring_cqe);

	/* Completion queue ring buffer mapping */
	cptr = SAFE_MMAP(0, cptr_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_POPULATE,
			s->ring_fd, IORING_OFF_CQ_RING);

	/* Save global completion queue struct info */
	cring->head = cptr + p.cq_off.head;
	cring->tail = cptr + p.cq_off.tail;
	cring->ring_mask = cptr + p.cq_off.ring_mask;
	cring->ring_entries = cptr + p.cq_off.ring_entries;
	cring->cqes = cptr + p.cq_off.cqes;

	return 0;
}

static void check_buffer(char *buffer, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++) {
		if (buffer[i] != 'a') {
			tst_res(TFAIL, "Wrong data at offset %zu", i);
			break;
		}
	}

	if (i == len)
		tst_res(TPASS, "Buffer filled in correctly");
}

static void drain_uring_cq(struct submitter *s, unsigned int exp_events)
{
	struct io_cq_ring *cring = &s->cq_ring;
	unsigned int head = *cring->head;
	unsigned int events = 0;

	for (head = *cring->head; head != *cring->tail; head++) {
		struct io_uring_cqe *cqe = &cring->cqes[head & *s->cq_ring.ring_mask];

		events++;

		if (cqe->res < 0) {
			tst_res(TFAIL, "CQE result %s", tst_strerrno(-cqe->res));
		} else {
			struct iovec *iovecs = (void*)cqe->user_data;

			if (cqe->res == BLOCK_SZ)
				tst_res(TPASS, "CQE result %i", cqe->res);
			else
				tst_res(TFAIL, "CQE result %i expected %i", cqe->res, BLOCK_SZ);

			check_buffer(iovecs[0].iov_base, cqe->res);
		}
	}

	*cring->head = head;

	if (exp_events == events) {
		tst_res(TPASS, "Got %u completion events", events);
		return;
	}

	tst_res(TFAIL, "Got %u completion events expected %u",
	        events, exp_events);
}

static int submit_to_uring_sq(struct submitter *s, struct tcase *tc)
{
	unsigned int index = 0, tail = 0, next_tail = 0;
	struct io_sq_ring *sring = &s->sq_ring;
	struct io_uring_sqe *sqe;
	int ret;

	memset(iov->iov_base, 0, iov->iov_len);

	ret = io_uring_register(s->ring_fd, tc->register_opcode,
				iov, QUEUE_DEPTH);
	if (ret == 0) {
		tst_res(TPASS, "io_uring_register() passed");
	} else {
		tst_res(TFAIL | TERRNO, "io_uring_register() failed");
		return 1;
	}

	int fd = SAFE_OPEN(TEST_FILE, O_RDONLY);

	/* Submission queue entry addition to SQE ring buffer tail */
	tail = *sring->tail;
	next_tail = tail + 1;
	index = tail & *s->sq_ring.ring_mask;
	sqe = &s->sqes[index];
	sqe->flags = 0;
	sqe->fd = fd;
	sqe->opcode = tc->enter_flags;
	sqe->addr = (unsigned long)iov->iov_base;
	sqe->len = BLOCK_SZ;
	sqe->off = 0;
	sqe->user_data = (unsigned long long)iov;
	sring->array[index] = index;
	tail = next_tail;

	/* Kernel to notice the tail update */
	if (*sring->tail != tail)
		*sring->tail = tail;

	ret = io_uring_enter(s->ring_fd, 1, 1, IORING_ENTER_GETEVENTS, &sig);
	if (ret == 1) {
		tst_res(TPASS, "io_uring_enter() waited for 1 event");
	} else {
		tst_res(TFAIL | TERRNO, "io_uring_enter() returned %i", ret);
		SAFE_CLOSE(fd);
		return 1;
	}

	SAFE_CLOSE(fd);
	return 0;
}

static void cleanup_io_uring_test(void)
{
	io_uring_register(s->ring_fd, IORING_UNREGISTER_BUFFERS,
			  NULL, QUEUE_DEPTH);
	SAFE_MUNMAP(s->sqes, sizeof(struct io_uring_sqe));
	SAFE_MUNMAP(cptr, cptr_size);
	SAFE_MUNMAP(sptr, sptr_size);
	SAFE_CLOSE(s->ring_fd);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (setup_io_uring_test(s, tc))
		return;

	if (!submit_to_uring_sq(s, tc))
		drain_uring_cq(s, 1);

	cleanup_io_uring_test();
}

static void setup(void)
{
	io_uring_setup_supported_by_kernel();
	tst_fill_file(TEST_FILE, 'a', 1024, 1);
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.needs_tmpdir = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.bufs = (struct tst_buffers []) {
		{&iov, .iov_sizes = (int[]){BLOCK_SZ, -1}},
		{}
	},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/kernel/io_uring_disabled", "0",
			TST_SR_SKIP_MISSING | TST_SR_TCONF_RO},
		{}
	}
};
