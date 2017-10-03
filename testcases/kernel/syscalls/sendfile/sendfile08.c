/*
 * Copyright (C) 2012 Red Hat, Inc.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Bug in the splice code has caused the file position on the write side
 * of the sendfile system call to be incorrectly set to the read side file
 * position. This can result in the data being written to an incorrect offset.
 *
 * This is a regression test for kernel commit
 * 2cb4b05e7647891b46b91c07c9a60304803d1688
 */

#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "safe_macros.h"

#define TEST_MSG_IN "world"
#define TEST_MSG_OUT "hello"
#define TEST_MSG_ALL (TEST_MSG_OUT TEST_MSG_IN)

TCID_DEFINE(sendfile08);
int TST_TOTAL = 1;

static int in_fd;
static int out_fd;
static char *in_file = "sendfile08.in";
static char *out_file = "sendfile08.out";

static void cleanup(void);
static void setup(void);

int main(int argc, char *argv[])
{
	int lc;
	int ret;
	char buf[BUFSIZ];

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		TEST(sendfile(out_fd, in_fd, NULL, strlen(TEST_MSG_IN)));

		if (TEST_RETURN == -1)
			tst_brkm(TBROK | TTERRNO, cleanup, "sendfile() failed");

		ret = SAFE_LSEEK(cleanup, out_fd, 0, SEEK_SET);
		ret = read(out_fd, buf, BUFSIZ);
		if (ret == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "read %s failed",
				 out_file);

		if (!strncmp(buf, TEST_MSG_ALL, strlen(TEST_MSG_ALL)))
			tst_resm(TPASS, "sendfile(2) copies data correctly");
		else
			tst_resm(TFAIL, "sendfile(2) copies data incorrectly."
				 " Expect \"%s%s\", got \"%s\"", TEST_MSG_OUT,
				 TEST_MSG_IN, buf);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int ret;

	/* Disable test if the version of the kernel is less than 2.6.33 */
	if ((tst_kvercmp(2, 6, 33)) < 0) {
		tst_resm(TCONF, "The out_fd must be socket before kernel");
		tst_brkm(TCONF, NULL, "2.6.33, see kernel commit cc56f7d");
	}

	TEST_PAUSE;

	tst_tmpdir();

	in_fd = SAFE_CREAT(cleanup, in_file, 0700);

	ret = write(in_fd, TEST_MSG_IN, strlen(TEST_MSG_IN));
	if (ret == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Write %s failed", in_file);
	close(in_fd);

	in_fd = SAFE_OPEN(cleanup, in_file, O_RDONLY);
	out_fd = SAFE_OPEN(cleanup, out_file, O_TRUNC | O_CREAT | O_RDWR, 0777);

	ret = write(out_fd, TEST_MSG_OUT, strlen(TEST_MSG_OUT));
	if (ret == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Write %s failed", out_file);
}

static void cleanup(void)
{
	close(out_fd);
	close(in_fd);

	tst_rmdir();
}
