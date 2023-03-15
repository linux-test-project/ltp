// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2008 Vijay Kumar B. <vijaykumar@bravegnu.org>
 * Copyright (c) Linux Test Project, 2008-2022
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test whether counter overflow is detected and handled correctly.
 *
 * It is not possible to directly overflow the counter using the
 * write() syscall. Overflows occur when the counter is incremented
 * from kernel space, in an IRQ context, when it is not possible to
 * block the calling thread of execution.
 *
 * The AIO subsystem internally uses eventfd mechanism for
 * notification of completion of read or write requests. In this test
 * we trigger a counter overflow, by setting the counter value to the
 * max possible value initially. When the AIO subsystem notifies
 * through the eventfd counter, the counter overflows.
 *
 * If the counter starts from an initial value of 0, it will
 * take decades for an overflow to occur. But since we set the initial
 * value to the max possible counter value, we are able to cause it to
 * overflow with a single increment.
 *
 * When the counter overflows, the following is tested:
 *
 * - POLLERR event occurs in poll() for the eventfd
 * - readfd_set/writefd_set is set in select() for the eventfd
 * - the counter value is UINT64_MAX
 */

#include "tst_test.h"

#ifdef HAVE_LIBAIO
#ifdef HAVE_IO_SET_EVENTFD

#include <poll.h>
#include <libaio.h>
#include <stdlib.h>
#include <sys/eventfd.h>

#define MAXEVENTS 16
#define BUFSIZE 1024

static int fd;
static int evfd;
static io_context_t ctx;

static void async_write(void)
{
	struct iocb iocb;
	struct iocb *iocbap[1];
	struct io_event ioev;
	static char buf[BUFSIZE];

	memset(buf, 1, BUFSIZE);

	io_prep_pwrite(&iocb, fd, buf, sizeof(buf), 0);
	io_set_eventfd(&iocb, evfd);

	iocbap[0] = &iocb;
	TEST(io_submit(ctx, 1, iocbap));
	if (TST_RET < 0)
		tst_brk(TBROK, "io_submit() failed: %s", tst_strerrno(-TST_RET));

	TEST(io_getevents(ctx, 1, 1, &ioev, NULL));
	if (TST_RET < 0)
		tst_brk(TBROK, "io_getevents() failed: %s", tst_strerrno(-TST_RET));
}

static void clear_counter(void)
{
	uint64_t val;
	uint64_t max =  UINT64_MAX - 1;

	TEST(read(evfd, &val, sizeof(val)));
	if (TST_RET == -1 && TST_ERR != EAGAIN)
		tst_brk(TBROK | TERRNO, "read");

	SAFE_WRITE(0, evfd, &max, sizeof(max));
}

static void test_select(void)
{
	fd_set readfds;
	uint64_t count;
	struct timeval timeout = { 10, 0 };

	clear_counter();
	async_write();

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	tst_res(TINFO, "Checking if select() detects counter overflow");

	TEST(select(fd + 1, NULL, &readfds, NULL, &timeout));
	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "select");

	TST_EXP_EQ_LI(FD_ISSET(fd, &readfds), 1);

	SAFE_READ(0, evfd, &count, sizeof(count));
	TST_EXP_EQ_LI(count, UINT64_MAX);
}

static void test_poll(void)
{
	uint64_t count;
	struct pollfd pollfd;

	clear_counter();
	async_write();

	pollfd.fd = evfd;
	pollfd.events = POLLIN;
	pollfd.revents = 0;

	tst_res(TINFO, "Checking if poll() detects counter overflow");

	TEST(poll(&pollfd, 1, 10000));
	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "poll");

	TST_EXP_EQ_LI(pollfd.revents & POLLERR, POLLERR);

	SAFE_READ(0, evfd, &count, sizeof(count));
	TST_EXP_EQ_LI(count, UINT64_MAX);
}

static void setup(void)
{
	TEST(io_setup(MAXEVENTS, &ctx));
	if (TST_RET < 0)
		tst_brk(TBROK, "io_setup() failed: %s", tst_strerrno(-TST_RET));

	fd = SAFE_OPEN("testfile", O_RDWR | O_CREAT, 0644);
	evfd = TST_EXP_FD(eventfd(0, EFD_NONBLOCK));
}

static void cleanup(void)
{
	SAFE_CLOSE(evfd);
	io_destroy(ctx);
}

static void run(void)
{
	test_select();
	test_poll();
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_EVENTFD",
		NULL
	},
};

#else /* HAVE_IO_SET_EVENTFD */
TST_TEST_TCONF("eventfd support is not available in AIO subsystem");
#endif
#else /* HAVE_LIBAIO */
TST_TEST_TCONF("libaio is not available");
#endif
