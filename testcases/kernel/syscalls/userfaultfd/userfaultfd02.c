// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 * Author: Ricardo Branco <rbranco@suse.com>
 */

/*\
 * Force a pagefault event and handle it using :manpage:`userfaultfd(2)`
 * from a different thread using UFFDIO_MOVE.
 */

#include "config.h"
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_safe_pthread.h"
#include "lapi/userfaultfd.h"

static long page_size;
static char *page;
static void *move_page;
static int uffd = -1;

static void set_pages(void)
{
	page_size = SAFE_SYSCONF(_SC_PAGE_SIZE);
	page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	move_page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

static void reset_pages(void)
{
	if (page) {
		SAFE_MUNMAP(page, page_size);
		page = NULL;
	}
	if (move_page) {
		SAFE_MUNMAP(move_page, page_size);
		move_page = NULL;
	}
	if (uffd != -1)
		SAFE_CLOSE(uffd);
}

static void *handle_thread(void *arg LTP_ATTRIBUTE_UNUSED)
{
	static struct uffd_msg msg;
	struct uffdio_move uffdio_move = {};

	SAFE_READ(SAFE_READ_ALL, uffd, &msg, sizeof(msg));

	if (msg.event != UFFD_EVENT_PAGEFAULT)
		tst_brk(TFAIL, "Received unexpected UFFD_EVENT %d", msg.event);

	memset(move_page, 'X', page_size);

	uffdio_move.src = (unsigned long) move_page;

	uffdio_move.dst = (unsigned long) msg.arg.pagefault.address
			& ~((unsigned long)page_size - 1);
	uffdio_move.len = page_size;
	SAFE_IOCTL(uffd, UFFDIO_MOVE, &uffdio_move);

	SAFE_CLOSE(uffd);
	return NULL;
}

static void run(void)
{
	pthread_t thr;
	struct uffdio_api uffdio_api = {};
	struct uffdio_register uffdio_register;

	set_pages();

	uffd = SAFE_USERFAULTFD(O_CLOEXEC, false);

	uffdio_api.api = UFFD_API;
	SAFE_IOCTL(uffd, UFFDIO_API, &uffdio_api);

	uffdio_register.range.start = (unsigned long) page;
	uffdio_register.range.len = page_size;
	uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;

	SAFE_IOCTL(uffd, UFFDIO_REGISTER, &uffdio_register);

	SAFE_PTHREAD_CREATE(&thr, NULL, (void *) handle_thread, NULL);

	char c = page[0xf];

	if (c == 'X')
		tst_res(TPASS, "Pagefault handled via UFFDIO_MOVE");
	else
		tst_res(TFAIL, "Pagefault not handled via UFFDIO_MOVE");

	SAFE_PTHREAD_JOIN(thr, NULL);
	reset_pages();
}

static struct tst_test test = {
	.test_all = run,
	.min_kver = "6.8",
	.cleanup = reset_pages,
};
