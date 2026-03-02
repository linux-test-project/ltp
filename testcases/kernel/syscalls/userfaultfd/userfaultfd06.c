// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 SUSE LLC
 * Author: Ricardo Branco <rbranco@suse.com>
 */

/*\
 * Force a pagefault event and handle it using :manpage:`userfaultfd(2)`
 * from a different thread testing UFFDIO_POISON.
 */

#include "config.h"
#include <poll.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_safe_pthread.h"
#include "lapi/userfaultfd.h"

static int page_size;
static char *page;
static int uffd;
static int poison_fault_seen;
static volatile int sigbus_seen;
static sigjmp_buf jmpbuf;

static void sigbus_handler(int sig)
{
	if (sig == SIGBUS) {
		sigbus_seen = 1;
		siglongjmp(jmpbuf, 1);
	}
}

static void setup(void)
{
	struct sigaction sa = {};

	sa.sa_handler = sigbus_handler;
	sigemptyset(&sa.sa_mask);
	SAFE_SIGACTION(SIGBUS, &sa, NULL);
}

static void set_pages(void)
{
	page_size = sysconf(_SC_PAGE_SIZE);
	page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

static void reset_pages(void)
{
	if (page) {
		SAFE_MUNMAP(page, page_size);
		page = NULL;
	}
}

static void *handle_thread(void)
{
	static struct uffd_msg msg;
	struct uffdio_poison uffdio_poison = {};
	struct pollfd pollfd;
	int nready;

	pollfd.fd = uffd;
	pollfd.events = POLLIN;
	nready = poll(&pollfd, 1, -1);
	if (nready == -1)
		tst_brk(TBROK | TERRNO, "Error on poll");

	SAFE_READ(1, uffd, &msg, sizeof(msg));

	if (msg.event != UFFD_EVENT_PAGEFAULT)
		tst_brk(TFAIL, "Received unexpected UFFD_EVENT %d", msg.event);

	tst_atomic_store(1, &poison_fault_seen);

	/* Poison the page that triggered the fault */
	uffdio_poison.range.start = msg.arg.pagefault.address & ~(page_size - 1);
	uffdio_poison.range.len = page_size;

	SAFE_IOCTL(uffd, UFFDIO_POISON, &uffdio_poison);

	close(uffd);
	return NULL;
}

static void run(void)
{
	pthread_t thr;
	struct uffdio_api uffdio_api = {};
	struct uffdio_register uffdio_register;
	char dummy;

	poison_fault_seen = 0;
	sigbus_seen = 0;
	set_pages();

	uffd = SAFE_USERFAULTFD(O_CLOEXEC | O_NONBLOCK, false);

	uffdio_api.api = UFFD_API;
	uffdio_api.features = UFFD_FEATURE_POISON;

	SAFE_IOCTL(uffd, UFFDIO_API, &uffdio_api);

	if (!(uffdio_api.features & UFFD_FEATURE_POISON))
		tst_brk(TCONF, "UFFD_FEATURE_POISON not supported");

	uffdio_register.range.start = (unsigned long) page;
	uffdio_register.range.len = page_size;
	uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;

	SAFE_IOCTL(uffd, UFFDIO_REGISTER, &uffdio_register);

	SAFE_PTHREAD_CREATE(&thr, NULL, (void *) handle_thread, NULL);

	/* Try to read from the page: should trigger fault, get poisoned, then SIGBUS */
	if (sigsetjmp(jmpbuf, 1) == 0) {
		LTP_VAR_USED(dummy) = page[0];
	}

	SAFE_PTHREAD_JOIN(thr, NULL);
	reset_pages();

	int poisoned = tst_atomic_load(&poison_fault_seen);

	if (poisoned && sigbus_seen)
		tst_res(TPASS, "POISON successfully triggered SIGBUS");
	else if (poisoned && !sigbus_seen)
		tst_res(TFAIL, "POISON fault seen but no SIGBUS received");
	else if (!poisoned && sigbus_seen)
		tst_res(TFAIL, "SIGBUS received but no poison fault seen");
	else
		tst_res(TFAIL, "No poison fault or SIGBUS observed");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = reset_pages,
};
