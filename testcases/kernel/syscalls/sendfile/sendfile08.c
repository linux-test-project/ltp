// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2012 Red Hat, Inc.
 */

/*\
 * [Description]
 *
 * Bug in the splice code has caused the file position on the write side
 * of the sendfile system call to be incorrectly set to the read side file
 * position. This can result in the data being written to an incorrect offset.
 *
 * This is a regression test for kernel commit 2cb4b05e76478.
 */

#include <stdio.h>
#include <string.h>
#include <sys/sendfile.h>

#include "tst_test.h"

#define IN_FILE		"in_file"
#define OUT_FILE	"out_file"

#define TEST_MSG_IN	"world"
#define TEST_MSG_OUT	"hello"
#define TEST_MSG_ALL	(TEST_MSG_OUT TEST_MSG_IN)

static int in_fd;
static int out_fd;

static void run(void)
{
	TEST(sendfile(out_fd, in_fd, NULL, strlen(TEST_MSG_IN)));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sendfile() failed");

	char buf[BUFSIZ];

	SAFE_LSEEK(out_fd, 0, SEEK_SET);
	SAFE_READ(0, out_fd, buf, BUFSIZ);

	if (!strncmp(buf, TEST_MSG_ALL, strlen(TEST_MSG_ALL))) {
		tst_res(TPASS, "sendfile() copies data correctly");
		return;
	}

	tst_res(TFAIL, "sendfile() copies data incorrectly: '%s' expected: '%s%s'",
			buf, TEST_MSG_OUT, TEST_MSG_IN);
}

static void setup(void)
{
	in_fd = SAFE_CREAT(IN_FILE, 0700);
	SAFE_WRITE(SAFE_WRITE_ALL, in_fd, TEST_MSG_IN, strlen(TEST_MSG_IN));
	SAFE_CLOSE(in_fd);
	in_fd = SAFE_OPEN(IN_FILE, O_RDONLY);

	out_fd = SAFE_OPEN(OUT_FILE, O_TRUNC | O_CREAT | O_RDWR, 0777);
	SAFE_WRITE(SAFE_WRITE_ALL, out_fd, TEST_MSG_OUT, strlen(TEST_MSG_OUT));
}

static void cleanup(void)
{
	SAFE_CLOSE(in_fd);
	SAFE_CLOSE(out_fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.min_kver = "2.6.33",
	.tags = (const struct tst_tag[]) {
		{"linux-git", "2cb4b05e76478"},
		{}
	}
};
