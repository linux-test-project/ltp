// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) Red Hat Inc., 2007
 * 11/2007 Copied from sendfile02.c by Masatake YAMATO
 */

/*\
 * Testcase to test that sendfile(2) system call returns EFAULT when passing
 * wrong offset pointer.
 *
 * [Algorithm]
 *
 * Given wrong address or protected buffer as OFFSET argument to sendfile:
 *
 * - a wrong address is created by munmap a buffer allocated by mmap
 * - a protected buffer is created by mmap with specifying protection
 */

#include <sys/sendfile.h>
#include "tst_test.h"

static int in_fd;
static int out_fd;

struct test_case_t {
	int protection;
	int pass_unmapped_buffer;
	const char *desc;
} tc[] = {
	{PROT_NONE, 0, "pass_mapped_buffer"},
	{PROT_READ, 0, "pass_mapped_buffer"},
	{PROT_EXEC, 0, "pass_mapped_buffer"},
	{PROT_EXEC | PROT_READ, 0, "pass_mapped_buffer"},
	{PROT_READ | PROT_WRITE, 1, "pass_unmapped_buffer"}
};

static void setup(void)
{
	in_fd = SAFE_OPEN("in_file", O_CREAT | O_RDWR, 0600);
	out_fd = SAFE_CREAT("out_file", 0600);
}

static void cleanup(void)
{
	SAFE_CLOSE(in_fd);
	SAFE_CLOSE(out_fd);
}

static void run(unsigned int i)
{
	off_t *protected_buffer;
	protected_buffer = SAFE_MMAP(NULL, sizeof(*protected_buffer),
			             tc[i].protection,
				     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (tc[i].pass_unmapped_buffer)
		SAFE_MUNMAP(protected_buffer, sizeof(*protected_buffer));

	TST_EXP_FAIL2(sendfile(out_fd, in_fd, protected_buffer, 1),
		     EFAULT, "sendfile(..) with %s, protection=%d",
		     tc[i].desc, tc[i].protection);

	if (!tc[i].pass_unmapped_buffer)
		SAFE_MUNMAP(protected_buffer, sizeof(*protected_buffer));
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.needs_tmpdir = 1,
	.cleanup = cleanup,
	.setup = setup,
	.test = run,
};
