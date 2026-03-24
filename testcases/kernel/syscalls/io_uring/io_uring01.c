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
#include "io_uring_common.h"

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

static struct io_uring_submit s;
static sigset_t sig;
static struct iovec *iov;

static int setup_io_uring_test(struct io_uring_submit *s, struct tcase *tc)
{
	int ret;

	ret = io_uring_setup_queue(s, QUEUE_DEPTH, tc->setup_flags);
	if (ret == 0)
		tst_res(TPASS, "io_uring_setup() passed");

	return ret;
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

static void drain_uring_cq(struct io_uring_submit *s, unsigned int exp_events)
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

static int submit_to_uring_sq(struct io_uring_submit *s, struct tcase *tc)
{
	int ret;
	int fd;

	memset(iov->iov_base, 0, iov->iov_len);

	ret = io_uring_register(s->ring_fd, tc->register_opcode,
				iov, QUEUE_DEPTH);
	if (ret == 0) {
		tst_res(TPASS, "io_uring_register() passed");
	} else {
		tst_res(TFAIL | TERRNO, "io_uring_register() failed");
		return 1;
	}

	fd = SAFE_OPEN(TEST_FILE, O_RDONLY);

	/* Submit SQE using common helper */
	io_uring_submit_sqe_internal(s, fd, tc->enter_flags,
				     (unsigned long)iov->iov_base,
				     BLOCK_SZ, 0,
				     (unsigned long long)iov);

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
	io_uring_register(s.ring_fd, IORING_UNREGISTER_BUFFERS,
			  NULL, QUEUE_DEPTH);
	io_uring_cleanup_queue(&s, QUEUE_DEPTH);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (setup_io_uring_test(&s, tc))
		return;

	if (!submit_to_uring_sq(&s, tc))
		drain_uring_cq(&s, 1);

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
