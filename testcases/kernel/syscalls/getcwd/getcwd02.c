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
#include <sys/types.h>
#include <sys/stat.h>

#include "tst_test.h"

static char exp_buf[PATH_MAX];
static char buffer[PATH_MAX];

static struct t_case {
	char *buf;
	size_t size;
} tcases[] = {
	{buffer, sizeof(buffer)},
	{NULL, 0},
	{NULL, PATH_MAX}
};

static int dir_exists(const char *dirpath)
{
	struct stat sb;

	if (!stat(dirpath, &sb) && S_ISDIR(sb.st_mode))
		return 1;

	return 0;
}

static const char *get_tmpdir_path(void)
{
	char *tmpdir = "/tmp";

	if (dir_exists(tmpdir))
		goto done;

	/* fallback to $TMPDIR */
	tmpdir = getenv("TMPDIR");
	if (!tmpdir)
		tst_brk(TBROK | TERRNO, "Failed to get $TMPDIR");

	if (tmpdir[0] != '/')
		tst_brk(TBROK, "$TMPDIR must be an absolute path");

	if (!dir_exists(tmpdir))
		tst_brk(TBROK | TERRNO, "TMPDIR '%s' doesn't exist", tmpdir);

done:
	return tmpdir;
}

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
	const char *tmpdir = get_tmpdir_path();

	SAFE_CHDIR(tmpdir);

	if (!realpath(tmpdir, exp_buf))
		tst_brk(TBROK | TERRNO, "realpath() failed");

	tst_res(TINFO, "Expected path '%s'", exp_buf);
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_getcwd
};
