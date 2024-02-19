// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 FNST, DAN LI <li.dan@cn.fujitsu.com>
 * Copyright (c) 2024 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that, mmap() call succeeds to create a file mapping with length
 * argument greater than the file size but any attempt to reference the
 * memory region which does not correspond to the file causes SIGBUS signal.
 */

#include <stdlib.h>
#include <setjmp.h>
#include "tst_test.h"

#define TEMPFILE	"mmapfile"
static size_t page_sz;
static char *addr;
static int fd;
static volatile sig_atomic_t pass;
static sigjmp_buf env;

static void sig_handler(int sig)
{
	if (sig == SIGBUS) {
		pass = 1;
		siglongjmp(env, 1);
	}
}

static void setup(void)
{
	SAFE_SIGNAL(SIGBUS, sig_handler);

	page_sz = getpagesize();

	fd = SAFE_OPEN(TEMPFILE, O_RDWR | O_CREAT, 0666);
	SAFE_FTRUNCATE(fd, page_sz / 2);
}

static void run(void)
{
	char *ch;

	addr = mmap(0, page_sz * 2, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED) {
		tst_res(TFAIL | TERRNO, "mmap() of %s failed", TEMPFILE);
		return;
	}

	if (sigsetjmp(env, 1) == 0) {
		ch = addr + page_sz + 1;
		*ch = 0;
	}

	if (pass == 1)
		tst_res(TPASS, "Received SIGBUS signal as expected");
	else
		tst_res(TFAIL, "SIGBUS signal not received");

	SAFE_MUNMAP(addr, page_sz * 2);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.needs_tmpdir = 1
};
