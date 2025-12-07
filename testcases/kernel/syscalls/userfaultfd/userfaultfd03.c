// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 * Author: Ricardo Branco <rbranco@suse.com>
 */

/*\
 * Force a pagefault event and handle it using :manpage:`userfaultfd(2)`
 * from a different thread using /dev/userfaultfd instead of syscall,
 * using USERFAULTFD_IOC_NEW ioctl to create the uffd & UFFDIO_COPY.
 */

#include "config.h"
#include <poll.h>
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_safe_pthread.h"
#include "lapi/userfaultfd.h"

static int page_size;
static char *page;
static void *copy_page;
static int uffd;

static int open_userfaultfd(int flags)
{
	int fd, fd2;

	fd = SAFE_OPEN("/dev/userfaultfd", O_RDWR);

	fd2 = SAFE_IOCTL(fd, USERFAULTFD_IOC_NEW, flags);

	SAFE_CLOSE(fd);

	return fd2;
}

static void set_pages(void)
{
	page_size = sysconf(_SC_PAGE_SIZE);
	page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	copy_page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

static void reset_pages(void)
{
	SAFE_MUNMAP(page, page_size);
	SAFE_MUNMAP(copy_page, page_size);
}

static void *handle_thread(void)
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

	SAFE_READ(1, uffd, &msg, sizeof(msg));

	if (msg.event != UFFD_EVENT_PAGEFAULT)
		tst_brk(TBROK | TERRNO, "Received unexpected UFFD_EVENT %d", msg.event);

	memset(copy_page, 'X', page_size);

	uffdio_copy.src = (unsigned long) copy_page;

	uffdio_copy.dst = (unsigned long) msg.arg.pagefault.address
			& ~(page_size - 1);
	uffdio_copy.len = page_size;
	SAFE_IOCTL(uffd, UFFDIO_COPY, &uffdio_copy);

	close(uffd);
	return NULL;
}

static void run(void)
{
	pthread_t thr;
	struct uffdio_api uffdio_api = {};
	struct uffdio_register uffdio_register;

	set_pages();

	uffd = open_userfaultfd(O_CLOEXEC | O_NONBLOCK);

	uffdio_api.api = UFFD_API;
	SAFE_IOCTL(uffd, UFFDIO_API, &uffdio_api);

	uffdio_register.range.start = (unsigned long) page;
	uffdio_register.range.len = page_size;
	uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;

	SAFE_IOCTL(uffd, UFFDIO_REGISTER, &uffdio_register);

	SAFE_PTHREAD_CREATE(&thr, NULL, (void *) handle_thread, NULL);

	char c = page[0xf];

	if (c == 'X')
		tst_res(TPASS, "Pagefault handled via /dev/userfaultfd");
	else
		tst_res(TFAIL, "Pagefault not handled via /dev/userfaultfd");

	SAFE_PTHREAD_JOIN(thr, NULL);
	reset_pages();
}

static struct tst_test test = {
	.needs_root = 1,
	.test_all = run,
	.min_kver = "5.11",
};
