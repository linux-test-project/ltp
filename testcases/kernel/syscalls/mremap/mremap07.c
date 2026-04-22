// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Wei Gao <wegao@suse.com>
 */

/*\
 * LTP test case for mremap() with MREMAP_DONTUNMAP and userfaultfd.
 *
 * Test mremap() with MREMAP_DONTUNMAP and verify that accessing the
 * old memory region triggers a page fault, which is then correctly
 * handled by a userfaultfd handler.
 */

#define _GNU_SOURCE
#include <poll.h>
#include <pthread.h>

#include "tst_test.h"
#include "tst_safe_pthread.h"
#include "lapi/userfaultfd.h"
#include "lapi/mmap.h"

static int page_size;
static int uffd = -1;
static char *old_addr;
static char *new_addr;

#define TEST_STRING_A "ABCD"
#define TEST_STRING_B "EFGH"

static void *fault_handler_thread(void *arg LTP_ATTRIBUTE_UNUSED)
{
	struct uffd_msg msg;
	struct uffdio_copy uffdio_copy;
	struct pollfd pollfd = { .fd = uffd, .events = POLLIN };

	TST_CHECKPOINT_WAIT(0);

	if (poll(&pollfd, 1, -1) == -1)
		tst_brk(TBROK | TERRNO, "poll() failed");

	SAFE_READ(1, uffd, &msg, sizeof(msg));

	if (msg.event != UFFD_EVENT_PAGEFAULT)
		tst_brk(TBROK, "Received unexpected UFFD_EVENT: %d", msg.event);

	if (msg.arg.pagefault.address != (unsigned long)old_addr)
		tst_brk(TBROK, "Page fault on unexpected address: %p",
				(void *)msg.arg.pagefault.address);

	tst_res(TINFO, "Userfaultfd handler caught a page fault at %p",
			(void *)msg.arg.pagefault.address);

	uffdio_copy.src = (unsigned long)new_addr;
	uffdio_copy.dst = (unsigned long)old_addr;
	uffdio_copy.len = page_size;
	uffdio_copy.mode = 0;
	uffdio_copy.copy = 0;

	SAFE_IOCTL(uffd, UFFDIO_COPY, &uffdio_copy);
	tst_res(TPASS, "Userfaultfd handler successfully handled the fault");

	return NULL;
}

static void check_mremap_dontunmap(void)
{
	char *test = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	char *tmp = mremap(test, page_size, page_size,
				MREMAP_DONTUNMAP | MREMAP_MAYMOVE, NULL);

	if (tmp == MAP_FAILED) {
		if (errno == EINVAL)
			tst_brk(TCONF | TERRNO,
					"MREMAP_DONTUNMAP not supported");
		tst_brk(TBROK | TERRNO, "mremap failed");
	}

	SAFE_MUNMAP(tmp, page_size);
	SAFE_MUNMAP(test, page_size);
}

static void setup(void)
{
	struct uffdio_api uffdio_api;
	struct uffdio_register uffdio_register;

	page_size = getpagesize();
	check_mremap_dontunmap();
	uffd = SAFE_USERFAULTFD(O_CLOEXEC | O_NONBLOCK, true);

	uffdio_api.api = UFFD_API;
	uffdio_api.features = 0;
	SAFE_IOCTL(uffd, UFFDIO_API, &uffdio_api);

	old_addr = SAFE_MMAP(NULL, page_size,
			     PROT_READ | PROT_WRITE,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1, 0);

	tst_res(TINFO, "Original mapping created at %p", (void *)old_addr);

	memcpy(old_addr, TEST_STRING_A, sizeof(TEST_STRING_A));

	uffdio_register.range.start = (unsigned long)old_addr;
	uffdio_register.range.len = page_size;
	uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;
	SAFE_IOCTL(uffd, UFFDIO_REGISTER, &uffdio_register);
}

static void cleanup(void)
{
	if (new_addr && new_addr != MAP_FAILED)
		SAFE_MUNMAP(new_addr, page_size);

	if (old_addr && old_addr != MAP_FAILED)
		SAFE_MUNMAP(old_addr, page_size);

	if (uffd != -1)
		SAFE_CLOSE(uffd);
}

static void run(void)
{
	new_addr = NULL;
	pthread_t handler_thread;

	SAFE_PTHREAD_CREATE(&handler_thread, NULL,
				fault_handler_thread, NULL);

	new_addr = mremap(old_addr, page_size, page_size,
				MREMAP_DONTUNMAP | MREMAP_MAYMOVE, NULL);
	if (new_addr == MAP_FAILED)
		tst_brk(TBROK | TERRNO, "mremap failed");

	tst_res(TINFO, "New mapping created at %p", (void *)new_addr);

	TST_EXP_EQ_STR(new_addr, TEST_STRING_A);
	memcpy(new_addr, TEST_STRING_B, sizeof(TEST_STRING_B));

	TST_CHECKPOINT_WAKE(0);

	tst_res(TINFO, "Main thread accessing old address %p to trigger fault",
		(void *)old_addr);

	(void)*(volatile char *)old_addr;

	SAFE_PTHREAD_JOIN(handler_thread, NULL);

	TST_EXP_EQ_STR(old_addr, TEST_STRING_B);

	SAFE_MUNMAP(new_addr, page_size);
	memcpy(old_addr, TEST_STRING_A, sizeof(TEST_STRING_A));
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_checkpoints = 1,
	.cleanup = cleanup,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USERFAULTFD=y",
		NULL,
	},
};
