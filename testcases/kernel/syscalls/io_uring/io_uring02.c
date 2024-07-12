// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC
 * Author: Nicolai Stange <nstange@suse.de>
 * LTP port: Martin Doucha <mdoucha@suse.cz>
 *
 * CVE-2020-29373
 *
 * Check that io_uring does not bypass chroot. Fixed in:
 *
 *  commit 9392a27d88b9707145d713654eb26f0c29789e50
 *  Author: Jens Axboe <axboe@kernel.dk>
 *  Date:   Thu Feb 6 21:42:51 2020 -0700
 *
 *  io-wq: add support for inheriting ->fs
 *
 *  commit ff002b30181d30cdfbca316dadd099c3ca0d739c
 *  Author: Jens Axboe <axboe@kernel.dk>
 *  Date:   Fri Feb 7 16:05:21 2020 -0700
 *
 *  io_uring: grab ->fs as part of async preparation
 *
 * stable 5.4 specific backport:
 *
 *  commit c4a23c852e80a3921f56c6fbc851a21c84a6d06b
 *  Author: Nicolai Stange <nstange@suse.de>
 *  Date:   Wed Jan 27 14:34:43 2021 +0100
 */

#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "tst_test.h"
#include "tst_safe_io_uring.h"

#define CHROOT_DIR "test_root"
#define SOCK_NAME "sock"
#define SPAM_MARK 0xfa7
#define BEEF_MARK 0xbeef

static struct sockaddr_un addr;
static int sendsock = -1, recvsock = -1, sockpair[2] = {-1, -1};
static struct io_uring_params params;
static struct tst_io_uring uring = { .fd = -1 };
static char buf[16];
static struct iovec iov = {
	.iov_base = buf,
	.iov_len = sizeof(buf)
};

static struct msghdr spam_header = {
	.msg_name = NULL,
	.msg_namelen = 0,
	.msg_iov = &iov,
	.msg_iovlen = 1
};

static struct msghdr beef_header = {
	.msg_name = &addr,
	.msg_namelen = sizeof(addr),
	.msg_iov = &iov,
	.msg_iovlen = 1
};

static void setup(void)
{
	char *tmpdir = tst_tmpdir_path();
	int ret;

	addr.sun_family = AF_UNIX;
	ret = snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/%s", tmpdir,
		SOCK_NAME);

	if (ret >= (int)sizeof(addr.sun_path))
		tst_brk(TBROK, "Tempdir path is too long");

	io_uring_setup_supported_by_kernel();

	sendsock = SAFE_SOCKET(AF_UNIX, SOCK_DGRAM, 0);
	recvsock = SAFE_SOCKET(AF_UNIX, SOCK_DGRAM, 0);
	SAFE_BIND(recvsock, (struct sockaddr *)&addr, sizeof(addr));

	SAFE_MKDIR(CHROOT_DIR, 0755);
	SAFE_CHROOT(CHROOT_DIR);
}

static void drain_fallback(void)
{
	uint32_t i, count, tail;
	int beef_found = 0;
	struct io_uring_sqe *sqe_ptr = uring.sqr_entries;
	const struct io_uring_cqe *cqe_ptr;

	SAFE_SOCKETPAIR(AF_UNIX, SOCK_DGRAM, 0, sockpair);
	SAFE_SETSOCKOPT_INT(sockpair[0], SOL_SOCKET, SO_SNDBUF,
		32+sizeof(buf));
	SAFE_FCNTL(sockpair[0], F_SETFL, O_NONBLOCK);

	/* Add spam requests to force async processing of the real test */
	for (i = 0, tail = *uring.sqr_tail; i < 255; i++, tail++, sqe_ptr++) {
		memset(sqe_ptr, 0, sizeof(*sqe_ptr));
		sqe_ptr->opcode = IORING_OP_SENDMSG;
		sqe_ptr->flags = IOSQE_IO_DRAIN;
		sqe_ptr->fd = sockpair[0];
		sqe_ptr->addr = (__u64)&spam_header;
		sqe_ptr->user_data = SPAM_MARK;
		uring.sqr_array[tail & *uring.sqr_mask] = i;
	}

	/* Add the real test to queue */
	memset(sqe_ptr, 0, sizeof(*sqe_ptr));
	sqe_ptr->opcode = IORING_OP_SENDMSG;
	sqe_ptr->flags = IOSQE_IO_DRAIN;
	sqe_ptr->fd = sendsock;
	sqe_ptr->addr = (__u64)&beef_header;
	sqe_ptr->user_data = BEEF_MARK;
	uring.sqr_array[tail & *uring.sqr_mask] = i;
	count = ++i;
	tail++;

	__atomic_store(uring.sqr_tail, &tail, __ATOMIC_RELEASE);
	SAFE_IO_URING_ENTER(1, uring.fd, count, count, IORING_ENTER_GETEVENTS,
		NULL);

	/* Check test results */
	__atomic_load(uring.cqr_tail, &tail, __ATOMIC_ACQUIRE);

	for (i = *uring.cqr_head; i != tail; i++, count--) {
		cqe_ptr = uring.cqr_entries + (i & *uring.cqr_mask);
		TST_ERR = -cqe_ptr->res;

		if (cqe_ptr->user_data == SPAM_MARK) {
			if (cqe_ptr->res >= 0 || cqe_ptr->res == -EAGAIN)
				continue;

			tst_res(TFAIL | TTERRNO,
				"Spam request failed unexpectedly");
			continue;
		}

		if (cqe_ptr->user_data != BEEF_MARK) {
			tst_res(TFAIL, "Unexpected entry in completion queue");
			count++;
			continue;
		}

		beef_found = 1;

		if (cqe_ptr->res >= 0) {
			tst_res(TFAIL, "Write outside chroot succeeded.");
		} else if (cqe_ptr->res != -ENOENT) {
			tst_res(TFAIL | TTERRNO,
				"Write outside chroot failed unexpectedly");
		} else {
			tst_res(TPASS | TTERRNO,
				"Write outside chroot failed as expected");
		}
	}

	__atomic_store(uring.cqr_head, &i, __ATOMIC_RELEASE);

	if (!beef_found)
		tst_res(TFAIL, "Write outside chroot result not found");

	if (count)
		tst_res(TFAIL, "Wrong number of entries in completion queue");

	SAFE_CLOSE(sockpair[0]);
	SAFE_CLOSE(sockpair[1]);
}

static void check_result(void)
{
	const struct io_uring_cqe *cqe_ptr;

	cqe_ptr = uring.cqr_entries + (*uring.cqr_head & *uring.cqr_mask);
	++*uring.cqr_head;
	TST_ERR = -cqe_ptr->res;

	if (cqe_ptr->user_data != BEEF_MARK) {
		tst_res(TFAIL, "Unexpected entry in completion queue");
		return;
	}

	if (cqe_ptr->res == -EINVAL) {
		tst_res(TINFO, "IOSQE_ASYNC is not supported, using fallback");
		drain_fallback();
		return;
	}

	tst_res(TINFO, "IOSQE_ASYNC is supported");

	if (cqe_ptr->res >= 0) {
		tst_res(TFAIL, "Write outside chroot succeeded.");
		return;
	}

	if (cqe_ptr->res != -ENOENT) {
		tst_res(TFAIL | TTERRNO,
			"Write outside chroot failed unexpectedly");
		return;
	}

	tst_res(TPASS | TTERRNO, "Write outside chroot failed as expected");
}

static void run(void)
{
	uint32_t tail;
	struct io_uring_sqe *sqe_ptr;

	SAFE_IO_URING_INIT(512, &params, &uring);
	sqe_ptr = uring.sqr_entries;
	tail = *uring.sqr_tail;

	memset(sqe_ptr, 0, sizeof(*sqe_ptr));
	sqe_ptr->opcode = IORING_OP_SENDMSG;
	sqe_ptr->flags = IOSQE_ASYNC;
	sqe_ptr->fd = sendsock;
	sqe_ptr->addr = (__u64)&beef_header;
	sqe_ptr->user_data = BEEF_MARK;
	uring.sqr_array[tail & *uring.sqr_mask] = 0;
	tail++;

	__atomic_store(uring.sqr_tail, &tail, __ATOMIC_RELEASE);
	SAFE_IO_URING_ENTER(1, uring.fd, 1, 1, IORING_ENTER_GETEVENTS, NULL);
	check_result();
	SAFE_IO_URING_CLOSE(&uring);
}

static void cleanup(void)
{
	if (uring.fd >= 0)
		SAFE_IO_URING_CLOSE(&uring);

	if (sockpair[0] >= 0) {
		SAFE_CLOSE(sockpair[0]);
		SAFE_CLOSE(sockpair[1]);
	}

	if (recvsock >= 0)
		SAFE_CLOSE(recvsock);

	if (sendsock >= 0)
		SAFE_CLOSE(sendsock);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_SYS_CHROOT),
		{}
	},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/kernel/io_uring_disabled", "0",
			TST_SR_SKIP_MISSING | TST_SR_TCONF_RO},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "9392a27d88b9"},
		{"linux-git", "ff002b30181d"},
		{"linux-git", "d87683620489"},
		{"linux-stable-git", "c4a23c852e80"},
		{"linux-stable-git", "cac68d12c531"},
		{"CVE", "2020-29373"},
		{}
	}
};
