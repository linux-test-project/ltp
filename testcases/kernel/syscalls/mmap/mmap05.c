// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that, mmap() call with 'PROT_NONE' and a file descriptor which is
 * open for read and write, succeeds to map the file creating mapped memory,
 * but any attempt to access the contents of the mapped region causes the
 * SIGSEGV signal.
 */

#include <stdlib.h>
#include <setjmp.h>
#include "tst_test.h"

#define TEMPFILE "mmapfile"
static size_t page_sz;
static volatile char *addr;
static int fd;
static volatile int sig_flag;
static sigjmp_buf env;

static void sig_handler(int sig)
{
	if (sig == SIGSEGV) {
		sig_flag = 1;
		siglongjmp(env, 1);
	}
}

static void setup(void)
{
	char *buf;

	SAFE_SIGNAL(SIGSEGV, sig_handler);

	page_sz = getpagesize();
	buf = SAFE_MALLOC(page_sz);
	memset(buf, 'A', page_sz);

	fd = SAFE_OPEN(TEMPFILE, O_RDWR | O_CREAT, 0666);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, page_sz);
	free(buf);
}

static void run(void)
{
	addr = mmap(NULL, page_sz, PROT_NONE, MAP_FILE | MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED) {
		tst_res(TFAIL | TERRNO, "mmap() of %s failed", TEMPFILE);
		return;
	}

	if (sigsetjmp(env, 1) == 0)
		tst_res(TINFO, "Trying to access mapped region: %c", addr[0]);

	if (sig_flag)
		tst_res(TPASS, "Received SIGSEGV signal as expected");
	else
		tst_res(TFAIL, "SIGSEGV signal not received");

	SAFE_MUNMAP((char *)addr, page_sz);

	sig_flag = 0;
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
