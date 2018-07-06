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
 * The testcase checks the various errnos of the unlink(2).
 * 1) unlink() returns ENOENT if file doesn't exist.
 * 2) unlink() returns ENOENT if path is empty.
 * 3) unlink() returns ENOENT if path contains a non-existent file.
 * 4) unlink() returns EFAULT if address is invalid.
 * 5) unlink() returns ENOTDIR if path contains a regular file.
 * 6) unlink() returns ENAMETOOLONG if path contains a regular file.
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>	/* for PATH_MAX */
#include "tst_test.h"

static char longpathname[PATH_MAX + 2];

static struct test_case_t {
	char *name;
	char *desc;
	int exp_errno;
} tcases[] = {
	{"nonexistfile", "non-existent file", ENOENT},
	{"", "path is empty string", ENOENT},
	{"nefile/file", "path contains a non-existent file", ENOENT},
	{NULL, "invalid address", EFAULT},
	{"file/file", "path contains a regular file", ENOTDIR},
	{longpathname, "pathname too long", ENAMETOOLONG},
};

static void verify_unlink(unsigned int n)
{
	struct test_case_t *tc = &tcases[n];

	TEST(unlink(tc->name));
	if (TST_RET != -1) {
		tst_res(TFAIL, "unlink(<%s>) succeeded unexpectedly",
			tc->desc);
		return;
	}

	if (TST_ERR == tc->exp_errno) {
		tst_res(TPASS | TTERRNO, "unlink(<%s>) failed as expected",
			tc->desc);
	} else {
		tst_res(TFAIL | TTERRNO,
			"unlink(<%s>) failed, expected errno: %s",
			tc->desc, tst_strerrno(tc->exp_errno));
	}
}

static void setup(void)
{
	unsigned int n;

	SAFE_TOUCH("file", 0777, NULL);

	memset(longpathname, 'a', PATH_MAX + 2);

	for (n = 0; n < ARRAY_SIZE(tcases); n++) {
		if (!tcases[n].name)
			tcases[n].name = tst_get_bad_addr(NULL);
	}
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_unlink,
};
