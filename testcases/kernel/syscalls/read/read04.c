// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * [Description]
 *
 * Testcase to check if read() returns the number of bytes read correctly.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include "tst_test.h"

static const char *fname = "test_file";
static const char palfa[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
#define PALFA_LEN (sizeof(palfa)-1)

static void verify_read(void)
{
	int fd;
	char prbuf[BUFSIZ];

	fd = SAFE_OPEN(fname, O_RDONLY);
	TEST(read(fd, prbuf, BUFSIZ));

	if (TST_RET != PALFA_LEN) {
		tst_res(TFAIL, "Bad read count - got %ld - expected %zu",
				TST_RET, PALFA_LEN);
		goto out;
	}

	if (memcmp(palfa, prbuf, PALFA_LEN)) {
		tst_res(TFAIL, "read buffer not equal to write buffer");
		goto out;
	}

	tst_res(TPASS, "read() data correctly");

out:
	SAFE_CLOSE(fd);
}

static void setup(void)
{
	int fd;

	fd = SAFE_CREAT(fname, 0777);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, palfa, PALFA_LEN);
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test_all = verify_read,
};
