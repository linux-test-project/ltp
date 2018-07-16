/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Description:
 * The testcase checks the basic functionality of the unlink(2).
 * 1) unlink() can delete regular file successfully.
 * 2) unlink() can delete fifo file successfully.
 */

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "tst_test.h"

static void file_create(char *);
static void fifo_create(char *);

static struct test_case_t {
	void (*setupfunc)(char *);
	char *desc;
} tcases[] = {
	{file_create, "file"},
	{fifo_create, "fifo"},
};

static void file_create(char *name)
{
	sprintf(name, "tfile_%d", getpid());
	SAFE_TOUCH(name, 0777, NULL);
}

static void fifo_create(char *name)
{
	sprintf(name, "tfifo_%d", getpid());
	SAFE_MKFIFO(name, 0777);
}

static void verify_unlink(unsigned int n)
{
	char fname[255];
	struct test_case_t *tc = &tcases[n];

	tc->setupfunc(fname);

	TEST(unlink(fname));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "unlink(%s) failed", tc->desc);
		return;
	}

	if (!access(fname, F_OK)) {
		tst_res(TFAIL, "unlink(%s) succeeded, but %s still existed",
			tc->desc, tc->desc);
		return;
	}

	tst_res(TPASS, "unlink(%s) succeeded", tc->desc);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_unlink,
};
