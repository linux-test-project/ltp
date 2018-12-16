// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 * Email: code@zilogic.com
 */

/*
 * Test statx
 *
 * This code tests the following flags:
 * 1) AT_EMPTY_PATH
 * 2) AT_SYMLINK_NOFOLLOW
 *
 * A test file and a link for it is created.
 *
 * To check empty path flag, test file fd alone is passed.
 * Predefined size of testfile is checked against obtained value.
 *
 * To check symlink no follow flag, the linkname is statxed.
 * To ensure that link is not dereferenced, obtained inode is compared
 * with test file inode.
 * Minimum kernel version required is 4.11.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <inttypes.h>
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "lapi/stat.h"

#define TESTFILE "test_temp"
#define LINK_FILE "test_temp_ln"
#define MODE 0644
#define SIZE 14

static int file_fd;

static void test_empty_path(void)
{
	struct statx buf;

	TEST(statx(file_fd, "", AT_EMPTY_PATH, 0, &buf));
	if (TST_RET == 0)
		tst_res(TPASS,
			"statx(file_fd, \"\", AT_EMPTY_PATH, 0, &buf)");
	else
		tst_brk(TFAIL | TTERRNO,
			"statx(file_fd, \"\", AT_EMPTY_PATH, 0, &buff)");

	if (buf.stx_size == SIZE)
		tst_res(TPASS,
			"stx_size(%"PRIu64") is correct", buf.stx_size);
	else
		tst_res(TFAIL,
			"stx_size(%"PRIu64") is not same as expected(%u)",
			buf.stx_size, SIZE);

}

static void test_sym_link(void)
{
	struct statx fbuf;
	struct statx lbuf;

	TEST(statx(AT_FDCWD, TESTFILE, 0, 0, &fbuf));

	if (TST_RET == 0)
		tst_res(TPASS,
			"statx(AT_FDCWD, %s, 0, 0, &fbuf)", TESTFILE);
	else
		tst_brk(TFAIL | TTERRNO,
			"statx(AT_FDCWD, %s, 0, 0, &fbuf)", TESTFILE);

	TEST(statx(AT_FDCWD, LINK_FILE, AT_SYMLINK_NOFOLLOW, 0, &lbuf));

	if (TST_RET == 0)
		tst_res(TPASS,
			"statx(AT_FDCWD, %s, AT_SYMLINK_NOFOLLOW, 0,&lbuf)",
			LINK_FILE);
	else
		tst_brk(TFAIL | TTERRNO,
			"statx(AT_FDCWD, %s, AT_SYMLINK_NOFOLLOW, 0,&lbuf)",
			LINK_FILE);

	if (fbuf.stx_ino != lbuf.stx_ino)
		tst_res(TPASS, "Statx symlink flag worked as expected");
	else
		tst_res(TFAIL,
			"Statx symlink flag failed to work as expected");
}

struct tcase {
	void (*tfunc)(void);
} tcases[] = {
	{&test_empty_path},
	{&test_sym_link}
};

static void run(unsigned int i)
{
	tcases[i].tfunc();
}

static void setup(void)
{
	char data_buf[SIZE] = "LinusTorvalds";

	file_fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, MODE);
	SAFE_WRITE(0, file_fd, data_buf, sizeof(data_buf));

	SAFE_SYMLINK(TESTFILE, LINK_FILE);
}

static void cleanup(void)
{
	if (file_fd > 0)
		SAFE_CLOSE(file_fd);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "4.11",
	.needs_tmpdir = 1,
};
