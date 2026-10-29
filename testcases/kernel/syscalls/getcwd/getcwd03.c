// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * Testcase to check the basic functionality of the getcwd(2)
 * system call on a symbolic link.
 *
 * [Algorithm]
 *
 * 1. create a directory, and create a symbolic link to it at the
 *    same directory level.
 * 2. get the working directory of a directory, and its pathname.
 * 3. get the working directory of a symbolic link, and its pathname,
 *    and its readlink info.
 * 4. compare the working directories and link information.
 */

#define _GNU_SOURCE 1
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include "tst_test.h"

static char dir[BUFSIZ], dir_link[BUFSIZ];

static void verify_getcwd(void)
{
	char link[BUFSIZ];
	char *res1 = NULL;
	char *res2 = NULL;

	SAFE_CHDIR(dir);

	res1 = getcwd(NULL, 0);
	if (!res1) {
		tst_res(TFAIL | TERRNO, "getcwd() failed to "
			"get working directory of a directory");
		goto end;
	}

	SAFE_CHDIR("..");
	SAFE_CHDIR(dir_link);

	res2 = getcwd(NULL, 0);
	if (!res2) {
		tst_res(TFAIL | TERRNO, "getcwd() failed to get "
			"working directory of a symbolic link");
		goto end;
	}

	if (strcmp(res1, res2)) {
		tst_res(TFAIL,
			"getcwd() got mismatched working directories (%s, %s)",
			res1, res2);
		goto end;
	}

	SAFE_CHDIR("..");
	SAFE_READLINK(dir_link, link, sizeof(link));

	if (strcmp(link, SAFE_BASENAME(res1))) {
		tst_res(TFAIL,
			"link information didn't match the working directory");
		goto end;
	}

	tst_res(TPASS, "getcwd() succeeded on a symbolic link");

end:
	free(res1);
	free(res2);
}

static void setup(void)
{
	sprintf(dir, "getcwd1.%d", getpid());
	sprintf(dir_link, "getcwd2.%d", getpid());
	SAFE_MKDIR(dir, 0755);
	SAFE_SYMLINK(dir, dir_link);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test_all = verify_getcwd
};
