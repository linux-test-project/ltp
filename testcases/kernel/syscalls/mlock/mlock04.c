// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2010  Red Hat, Inc.
 */

/*\
 * [Description]
 *
 * This is a reproducer copied from one of LKML patch submission
 * which subject is
 *
 * [PATCH] mlock: revert the optimization for dirtying pages and triggering writeback.
 * url see https://www.spinics.net/lists/kernel/msg1141090.html
 *
 * "In 5ecfda0, we do some optimization in mlock, but it causes
 * a very basic test case(attached below) of mlock to fail. So
 * this patch revert it with some tiny modification so that it
 * apply successfully with the lastest 38-rc2 kernel."
 *
 * This bug was fixed by kernel
 * commit fdf4c587a7 ("mlock: operate on any regions with protection != PROT_NONE")
 *
 * As this case does, mmaps a file with PROT_WRITE permissions but without
 * PROT_READ, so attempt to not unnecessarity break COW during mlock ended up
 * causing mlock to fail with a permission problem on unfixed kernel.
 */

#include <sys/mman.h>
#include <stdio.h>
#include <sys/types.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

static int fd = -1, file_len = 40960;
static char *testfile = "test_mlock";

static void verify_mlock(void)
{
	char *buf;

	buf = SAFE_MMAP(NULL, file_len, PROT_WRITE, MAP_SHARED, fd, 0);
	TST_EXP_PASS(mlock(buf, file_len), "mlock(%p, %d)", buf, file_len);
	SAFE_MUNLOCK(buf, file_len);
	SAFE_MUNMAP(buf, file_len);
}

static void setup(void)
{
	fd = SAFE_OPEN(testfile, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	SAFE_FTRUNCATE(fd, file_len);
}

static void cleanup(void)
{
	if (fd > -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_mlock,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "fdf4c587a793"},
		{}
	}
};
