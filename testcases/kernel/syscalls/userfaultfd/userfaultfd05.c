// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 * Author: Ricardo Branco <rbranco@suse.com>
 */

/*\
 * Force a pagefault event and handle it using :manpage:`userfaultfd(2)`
 * from a different thread testing UFFDIO_WRITEPROTECT_MODE_WP.
 */

#include "config.h"
#include <poll.h>
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_safe_pthread.h"
#include "lapi/userfaultfd.h"

static int page_size;
static char *page;
static int uffd;
static volatile int wp_fault_seen;

static void set_pages(void)
{
	page_size = sysconf(_SC_PAGE_SIZE);
	page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	memset(page, 0, page_size);
}

static void reset_pages(void)
{
	SAFE_MUNMAP(page, page_size);
}

static void *handle_thread(void)
{
	static struct uffd_msg msg;
	struct uffdio_writeprotect uffdio_writeprotect = {};

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

	if (!(msg.arg.pagefault.flags & UFFD_PAGEFAULT_FLAG_WP) ||
	    !(msg.arg.pagefault.flags & UFFD_PAGEFAULT_FLAG_WRITE)) {
		tst_brk(TFAIL,
			"Expected WP+WRITE fault but flags=%lx",
			(unsigned long)msg.arg.pagefault.flags);
	}

	/* While the WP fault is pending, the write must NOT be visible. */
	if (page[0xf] != 0)
		tst_brk(TFAIL,
			"Write became visible while page was write-protected!");

	wp_fault_seen = 1;

	/* Resolve the fault by clearing WP so the writer can resume. */
	uffdio_writeprotect.range.start	= msg.arg.pagefault.address & ~(page_size - 1);
	uffdio_writeprotect.range.len	= page_size;

	SAFE_IOCTL(uffd, UFFDIO_WRITEPROTECT, &uffdio_writeprotect);

	close(uffd);
	return NULL;
}

static void run(void)
{
	pthread_t thr;
	struct uffdio_api uffdio_api;
	struct uffdio_register uffdio_register;
	struct uffdio_writeprotect uffdio_writeprotect;

	set_pages();

	uffd = SAFE_USERFAULTFD(O_CLOEXEC | O_NONBLOCK, false);

	uffdio_api.api = UFFD_API;
	uffdio_api.features = UFFD_FEATURE_PAGEFAULT_FLAG_WP;
	if (ioctl(uffd, UFFDIO_API, &uffdio_api) < 0) {
		if (!(uffdio_api.features & UFFD_FEATURE_PAGEFAULT_FLAG_WP))
			tst_brk(TCONF, "UFFD write-protect unsupported");

		tst_brk(TBROK | TERRNO, "ioctl() on userfaultfd failed");
	}

	uffdio_register.range.start = (unsigned long) page;
	uffdio_register.range.len = page_size;
	uffdio_register.mode = UFFDIO_REGISTER_MODE_WP;

	SAFE_IOCTL(uffd, UFFDIO_REGISTER, &uffdio_register);

	uffdio_writeprotect.range.start	= (unsigned long)page;
	uffdio_writeprotect.range.len	= page_size;
	uffdio_writeprotect.mode	= UFFDIO_WRITEPROTECT_MODE_WP;

	SAFE_IOCTL(uffd, UFFDIO_WRITEPROTECT, &uffdio_writeprotect);

	SAFE_PTHREAD_CREATE(&thr, NULL, (void *) handle_thread, NULL);

	/* Try to write */
	page[0xf] = 'W';

	SAFE_PTHREAD_JOIN(thr, NULL);
	reset_pages();

	if (wp_fault_seen)
		tst_res(TPASS, "WRITEPROTECT pagefault handled!");
	else
		tst_res(TFAIL, "No WRITEPROTECT pagefault observed");
}

static struct tst_test test = {
	.test_all = run,
	.min_kver = "5.7",
};
