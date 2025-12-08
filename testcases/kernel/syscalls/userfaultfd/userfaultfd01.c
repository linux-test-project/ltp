// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

 /*\
 * Force a pagefault event and handle it using :man2:`userfaultfd`
 * from a different thread.
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

static int sys_userfaultfd(int flags)
{
	return tst_syscall(__NR_userfaultfd, flags);
}

static void set_pages(void)
{
	page_size = sysconf(_SC_PAGE_SIZE);
	page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	copy_page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

static void handle_thread(void)
{
	static struct uffd_msg msg;
	struct uffdio_copy uffdio_copy;

	struct pollfd pollfd;
	int nready;

	pollfd.fd = uffd;
	pollfd.events = POLLIN;
	nready = poll(&pollfd, 1, -1);
	if (nready == -1)
		tst_brk(TBROK | TERRNO, "Error on poll");

	SAFE_READ(1, uffd, &msg, sizeof(msg));

	if (msg.event != UFFD_EVENT_PAGEFAULT)
		tst_brk(TBROK | TERRNO, "Received unexpected UFFD_EVENT");

	memset(copy_page, 'X', page_size);

	uffdio_copy.src = (unsigned long) copy_page;

	uffdio_copy.dst = (unsigned long) msg.arg.pagefault.address
			& ~(page_size - 1);
	uffdio_copy.len = page_size;
	uffdio_copy.mode = 0;
	uffdio_copy.copy = 0;
	SAFE_IOCTL(uffd, UFFDIO_COPY, &uffdio_copy);

	close(uffd);
}

static void run(void)
{
	pthread_t thr;
	struct uffdio_api uffdio_api;
	struct uffdio_register uffdio_register;

	set_pages();

	TEST(sys_userfaultfd(O_CLOEXEC | O_NONBLOCK));

	if (TST_RET == -1) {
		if (TST_ERR == EPERM) {
			tst_res(TCONF, "Hint: check /proc/sys/vm/unprivileged_userfaultfd");
			tst_brk(TCONF | TTERRNO,
				"userfaultfd() requires CAP_SYS_PTRACE on this system");
		} else
			tst_brk(TBROK | TTERRNO,
				"Could not create userfault file descriptor");
	}

	uffd = TST_RET;
	uffdio_api.api = UFFD_API;
	uffdio_api.features = 0;
	SAFE_IOCTL(uffd, UFFDIO_API, &uffdio_api);

	uffdio_register.range.start = (unsigned long) page;
	uffdio_register.range.len = page_size;
	uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;

	SAFE_IOCTL(uffd, UFFDIO_REGISTER, &uffdio_register);

	SAFE_PTHREAD_CREATE(&thr, NULL,
			(void * (*)(void *)) handle_thread, NULL);

	char c = page[0xf];

	if (c == 'X')
		tst_res(TPASS, "Pagefault handled!");
	else
		tst_res(TFAIL, "Pagefault not handled!");

	SAFE_PTHREAD_JOIN(thr, NULL);
}

static struct tst_test test = {
	.test_all = run,
	.min_kver = "4.3",
};
