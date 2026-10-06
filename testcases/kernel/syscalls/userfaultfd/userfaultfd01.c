// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

/*\
 * Force a pagefault event and handle it using :manpage:`userfaultfd(2)`
 * from a different thread.
 */

#include <poll.h>
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_safe_pthread.h"
#include "lapi/userfaultfd.h"

#define BEFORE_5_11 1
#define AFTER_5_11 2
#define DESC(x) .flags = x, .desc = #x

static struct tcase {
	int flags;
	const char *desc;
	int kver;
} tcases[] = {
	{ DESC(O_CLOEXEC | O_NONBLOCK) },
	{ DESC(O_CLOEXEC | O_NONBLOCK | UFFD_USER_MODE_ONLY),  .kver = AFTER_5_11, },
};

static long page_size;
static char *page;
static void *copy_page;
static int uffd = -1;
static int kver;

static void setup(void)
{
	if (tst_kvercmp(5, 11, 0) >= 0)
		kver = AFTER_5_11;
	else
		kver = BEFORE_5_11;
}

static void set_pages(void)
{
	page_size = SAFE_SYSCONF(_SC_PAGE_SIZE);
	page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	copy_page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

static void reset_pages(void)
{
	if (page) {
		SAFE_MUNMAP(page, page_size);
		page = NULL;
	}
	if (copy_page) {
		SAFE_MUNMAP(copy_page, page_size);
		copy_page = NULL;
	}
	if (uffd != -1)
		SAFE_CLOSE(uffd);
}

static void *handle_thread(void *arg LTP_ATTRIBUTE_UNUSED)
{
	static struct uffd_msg msg;
	struct uffdio_copy uffdio_copy = {};

	struct pollfd pollfd;
	int nready;

	pollfd.fd = uffd;
	pollfd.events = POLLIN;
	nready = poll(&pollfd, 1, -1);
	if (nready == -1)
		tst_brk(TBROK | TERRNO, "Error on poll");

	SAFE_READ(SAFE_READ_ALL, uffd, &msg, sizeof(msg));

	if (msg.event != UFFD_EVENT_PAGEFAULT)
		tst_brk(TFAIL, "Received unexpected UFFD_EVENT %d", msg.event);

	memset(copy_page, 'X', page_size);

	uffdio_copy.src = (unsigned long) copy_page;

	uffdio_copy.dst = (unsigned long) msg.arg.pagefault.address
			& ~((unsigned long)page_size - 1);
	uffdio_copy.len = page_size;
	SAFE_IOCTL(uffd, UFFDIO_COPY, &uffdio_copy);

	SAFE_CLOSE(uffd);
	return NULL;
}

static void run(unsigned int i)
{
	pthread_t thr;
	struct uffdio_api uffdio_api = {};
	struct uffdio_register uffdio_register;
	struct tcase *tc = &tcases[i];

	if (tc->kver == AFTER_5_11 && kver == BEFORE_5_11)
		tst_brk(TCONF, "%s requires kernel >= 5.11", tc->desc);

	set_pages();

	uffd = SAFE_USERFAULTFD(tc->flags, false);

	uffdio_api.api = UFFD_API;
	SAFE_IOCTL(uffd, UFFDIO_API, &uffdio_api);

	uffdio_register.range.start = (unsigned long) page;
	uffdio_register.range.len = page_size;
	uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;

	SAFE_IOCTL(uffd, UFFDIO_REGISTER, &uffdio_register);

	SAFE_PTHREAD_CREATE(&thr, NULL, (void *) handle_thread, NULL);

	char c = page[0xf];

	if (c == 'X')
		tst_res(TPASS, "Pagefault handled!");
	else
		tst_res(TFAIL, "Pagefault not handled!");

	SAFE_PTHREAD_JOIN(thr, NULL);
	reset_pages();
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.cleanup = reset_pages,
};
