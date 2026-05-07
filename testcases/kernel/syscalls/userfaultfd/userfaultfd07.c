// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 SUSE LLC
 * Author: Ricardo Branco <rbranco@suse.com>
 */

/*\
 * Force a pagefault event and handle it using :manpage:`userfaultfd(2)`
 * from a different thread testing UFFDIO_CONTINUE.
 */

#include "config.h"
#include <poll.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_safe_prw.h"
#include "tst_safe_pthread.h"
#include "lapi/memfd.h"
#include "lapi/userfaultfd.h"
#include "lapi/syscalls.h"

static long page_size;
static char *page;
static int uffd = -1;
static int memfd = -1;

static void setup(void)
{
	CHECK_UFFD_FEATURE(UFFD_FEATURE_MINOR_SHMEM);
}

static void set_pages(void)
{
	char ch = 'A';

	page_size = SAFE_SYSCONF(_SC_PAGE_SIZE);

	memfd = sys_memfd_create("ltp-uffd-continue", MFD_CLOEXEC);
	if (memfd < 0)
		tst_brk(TBROK | TERRNO, "memfd_create failed");

	SAFE_FTRUNCATE(memfd, page_size);

	/*
	 * Populate page cache so that after MADV_DONTNEED the next access
	 * can generate a MINOR fault rather than a MISSING fault.
	 */
	SAFE_PWRITE(1, memfd, &ch, 1, 0);

	page = SAFE_MMAP(NULL, page_size, PROT_READ, MAP_SHARED, memfd, 0);
}

static void reset_pages(void)
{
	if (page) {
		SAFE_MUNMAP(page, page_size);
		page = NULL;
	}

	if (memfd != -1)
		SAFE_CLOSE(memfd);

	if (uffd != -1)
		SAFE_CLOSE(uffd);
}

static void *handle_thread(void *arg LTP_ATTRIBUTE_UNUSED)
{
	static struct uffd_msg msg;
	struct uffdio_continue uffdio_continue = {};
	struct pollfd pollfd;
	int nready;
	char z = 'Z';

	pollfd.fd = uffd;
	pollfd.events = POLLIN;
	nready = poll(&pollfd, 1, -1);
	if (nready == -1)
		tst_brk(TBROK | TERRNO, "Error on poll");

	SAFE_READ(1, uffd, &msg, sizeof(msg));

	if (msg.event != UFFD_EVENT_PAGEFAULT)
		tst_brk(TFAIL, "Received unexpected UFFD_EVENT %d", msg.event);

	if (!(msg.arg.pagefault.flags & UFFD_PAGEFAULT_FLAG_MINOR)) {
		tst_brk(TFAIL, "expected MINOR fault, got flags=0x%llx",
			(unsigned long long)msg.arg.pagefault.flags);
	}

	/* Update the shmem page in page cache before resuming the fault. */
	SAFE_PWRITE(1, memfd, &z, 1, 0);

	uffdio_continue.range.start =
		msg.arg.pagefault.address & ~((unsigned long)page_size - 1);
	uffdio_continue.range.len = page_size;

	SAFE_IOCTL(uffd, UFFDIO_CONTINUE, &uffdio_continue);

	SAFE_CLOSE(uffd);
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
	uffdio_api.features = UFFD_FEATURE_MINOR_SHMEM;

	SAFE_IOCTL(uffd, UFFDIO_API, &uffdio_api);

	uffdio_register.range.start = (unsigned long)page;
	uffdio_register.range.len = page_size;
	uffdio_register.mode = UFFDIO_REGISTER_MODE_MINOR;

	SAFE_IOCTL(uffd, UFFDIO_REGISTER, &uffdio_register);

	/*
	 * Drop PTEs while retaining the cached shmem page so the next access
	 * faults in MINOR mode.
	 */
	if (madvise(page, page_size, MADV_DONTNEED) < 0)
		tst_brk(TBROK | TERRNO, "madvise MADV_DONTNEED failed");

	SAFE_PTHREAD_CREATE(&thr, NULL, (void *)handle_thread, NULL);

	char c = page[0];

	if (c == 'Z')
		tst_res(TPASS, "Pagefault handled via UFFDIO_CONTINUE");
	else
		tst_res(TFAIL, "pagefault not handled via UFFDIO_CONTINUE, got '%c'", c);

	SAFE_PTHREAD_JOIN(thr, NULL);
	reset_pages();
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	/*
	 * UFFDIO_CONTINUE is available since 5.13 but
	 * UFFD_FEATURE_MINOR_SHMEM appeared in 5.14
	 */
	.min_kver = "5.14",
	.cleanup = reset_pages,
};
