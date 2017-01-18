/*
 * Copyright (c) International Business Machines  Corp., 2001
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
 * along with this program.
 */

/*
 * DESCRIPTION
 * Testcase to check the basic functionality of the getcwd(2) system call.
 * 1) getcwd(2) works fine if buf and size are valid.
 * 2) getcwd(2) works fine if buf points to NULL and size is set to 0.
 * 3) getcwd(2) works fine if buf points to NULL and size is greater than strlen(path).
 */

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "tst_test.h"

static const char *exp_buf = "/tmp";
static char buffer[5];

static struct t_case {
	char *buf;
	size_t size;
} tcases[] = {
	{buffer, sizeof(buffer)},
	{NULL, 0},
	{NULL, 5}
};

static void verify_getcwd(unsigned int n)
{
	struct t_case *tc = &tcases[n];
	char *res = NULL;

	errno = 0;
	res = getcwd(tc->buf, tc->size);
	TEST_ERRNO = errno;
	if (!res) {
		tst_res(TFAIL | TTERRNO, "getcwd() failed");
		goto end;
	}

	if (strcmp(exp_buf, res)) {
		tst_res(TFAIL, "getcwd() returned unexpected directory: %s, "
			"expected: %s", res, exp_buf);
		goto end;
	}

	tst_res(TPASS, "getcwd() returned expected directory: %s", res);

end:
	if (!tc->buf)
		free(res);
}

static void setup(void)
{
	SAFE_CHDIR("/tmp");
}

static struct tst_test test = {
	.tid = "getcwd02",
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_getcwd
};
