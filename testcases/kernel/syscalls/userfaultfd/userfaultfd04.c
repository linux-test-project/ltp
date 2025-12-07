// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 * Author: Ricardo Branco <rbranco@suse.com>
 */

/*\
 * Force a pagefault event and handle it using :manpage:`userfaultfd(2)`
 * from a different thread using UFFDIO_ZEROPAGE.
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

static void set_pages(void)
{
	page_size = sysconf(_SC_PAGE_SIZE);
	page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

static void reset_pages(void)
{
	SAFE_MUNMAP(page, page_size);
}

static void *handle_thread(void)
{
	static struct uffd_msg msg;
	struct uffdio_zeropage uffdio_zeropage = {};

	struct pollfd pollfd;
	int nready;

	pollfd.fd = uffd;
	pollfd.events = POLLIN;
	nready = poll(&pollfd, 1, -1);
	if (nready == -1)
		tst_brk(TBROK | TERRNO, "Error on poll");

	SAFE_READ(1, uffd, &msg, sizeof(msg));

	if (msg.event != UFFD_EVENT_PAGEFAULT)
		tst_brk(TBROK | TERRNO, "Received unexpected UFFD_EVENT %d", msg.event);

	uffdio_zeropage.range.start	= msg.arg.pagefault.address
					& ~(page_size - 1);
	uffdio_zeropage.range.len	= page_size;

	SAFE_IOCTL(uffd, UFFDIO_ZEROPAGE, &uffdio_zeropage);

	close(uffd);
	return NULL;
}

static void run(void)
{
	pthread_t thr;
	struct uffdio_api uffdio_api = {};
	struct uffdio_register uffdio_register;

	set_pages();

	uffd = SAFE_USERFAULTFD(O_CLOEXEC | O_NONBLOCK, false);

	uffdio_api.api = UFFD_API;
	SAFE_IOCTL(uffd, UFFDIO_API, &uffdio_api);

	uffdio_register.range.start = (unsigned long) page;
	uffdio_register.range.len = page_size;
	uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;

	SAFE_IOCTL(uffd, UFFDIO_REGISTER, &uffdio_register);

	SAFE_PTHREAD_CREATE(&thr, NULL, (void *) handle_thread, NULL);

	for (int i = 0; i < page_size; i++) {
		if (page[i] != 0) {
			tst_res(TFAIL, "page[%d]=0x%x not zero", i, page[i]);
			return;
		}
	}

	tst_res(TPASS, "Pagefault handled with UFFDIO_ZEROPAGE");

	SAFE_PTHREAD_JOIN(thr, NULL);
	reset_pages();
}

static struct tst_test test = {
	.test_all = run,
};
