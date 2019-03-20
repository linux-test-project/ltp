/*
 *
 *   Copyright (c) International Business Machines  Corp., 2014
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc.
 */
/*
 * NAME
 *        sendfile09.c
 *
 * DESCRIPTION
 *        Testcase copied from sendfile02.c to test the basic functionality of
 *        the sendfile(2) system call on large file. There is a kernel bug which
 *        introduced by commit 8f9c0119d7ba and fixed by commit 5d73320a96fcc.
 *
 * ALGORITHM
 *        1. call sendfile(2) with offset at 0
 *        2. call sendfile(2) with offset at 3GB
 *
 * USAGE:  <for command-line>
 *  sendfile09 [-c n] [-i n] [-I x] [-P x] [-t]
 *     where,
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 *
 * RESTRICTIONS
 *        Only supports 64bit systems and kernel 2.6.33 or above
 */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/abisize.h"

#ifndef OFF_T
#define OFF_T off_t
#endif /* Not def: OFF_T */

TCID_DEFINE(sendfile09);

static char *in_file = "in";
static char *out_file = "out";
static int fd;
static int in_fd;
static int out_fd;

static void cleanup(void);
static void setup(void);

#define ONE_GB (INT64_C(1) << 30)

static struct test_case_t {
	char *desc;
	OFF_T offset;
	int64_t count;
	int64_t exp_retval;
	int64_t exp_updated_offset;
} testcases[] = {
	{ "Test sendfile(2) with offset at 0",
		0, ONE_GB, ONE_GB, ONE_GB},
	{ "Test sendfile(2) with offset at 3GB",
		3*ONE_GB, ONE_GB, ONE_GB, 4*ONE_GB}
};

static int TST_TOTAL = ARRAY_SIZE(testcases);

void do_sendfile(struct test_case_t *t)
{
	off_t before_pos, after_pos;

	out_fd = SAFE_OPEN(cleanup, out_file, O_WRONLY);
	in_fd = SAFE_OPEN(cleanup, in_file, O_RDONLY);
	before_pos = SAFE_LSEEK(cleanup, in_fd, 0, SEEK_CUR);

	TEST(sendfile(out_fd, in_fd, &t->offset, t->count));
	if (TEST_RETURN == -1)
		tst_brkm(TBROK | TTERRNO, cleanup, "sendfile(2) failed");

	after_pos = SAFE_LSEEK(cleanup, in_fd, 0, SEEK_CUR);

	if (TEST_RETURN != t->exp_retval) {
		tst_resm(TFAIL, "sendfile(2) failed to return "
			"expected value, expected: %" PRId64 ", "
			"got: %ld", t->exp_retval,
			TEST_RETURN);
	} else if (t->offset != t->exp_updated_offset) {
		tst_resm(TFAIL, "sendfile(2) failed to update "
			"OFFSET parameter to expected value, "
			"expected: %" PRId64 ", got: %" PRId64,
			t->exp_updated_offset,
			(int64_t) t->offset);
	} else if (before_pos != after_pos) {
		tst_resm(TFAIL, "sendfile(2) updated the file position "
			" of in_fd unexpectedly, expected file position: %"
			PRId64 ", " " actual file position %" PRId64,
			(int64_t) before_pos, (int64_t) after_pos);
	} else {
		tst_resm(TPASS, "%s", t->desc);
	}

	close(in_fd);
	close(out_fd);
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{
	int i;

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	if (!tst_fs_has_free(NULL, ".", 5, TST_GB))
		tst_brkm(TCONF, cleanup, "sendfile(2) on large file"
			" needs 5G free space.");

	/* create a 4G file */
	fd = SAFE_CREAT(cleanup, in_file, 00700);
	for (i = 1; i <= (4 * 1024); i++) {
		SAFE_LSEEK(cleanup, fd, 1024 * 1024 - 1, SEEK_CUR);
		SAFE_WRITE(cleanup, 1, fd, "C", 1);
	}
	close(fd);

	fd = SAFE_CREAT(cleanup, out_file, 00700);
	close(fd);
}

void cleanup(void)
{
	if (fd > 0)
		close(fd);

	if (in_fd > 0)
		close(in_fd);

	if (out_fd > 0)
		close(out_fd);

	tst_rmdir();
}

int main(int ac, char **av)
{
	int i;
	int lc;

#ifdef TST_ABI32
	tst_brkm(TCONF, NULL, "This test is only for 64bit");
#endif

	if (tst_kvercmp(2, 6, 33) < 0) {
		tst_resm(TINFO, "sendfile(2) on large file "
			"skipped for kernels < 2.6.33");
		return 0;
	}

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/*
	 * The following loop checks looping state if -c option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; ++i)
			do_sendfile(&testcases[i]);
	}

	cleanup();
	tst_exit();
}
