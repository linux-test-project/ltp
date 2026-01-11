// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2002-2022
 */

/*\
 * Verify that :manpage:`unlink(2)`: fails with:
 *
 * - ENOENT when file does not exist
 * - ENOENT when pathname is empty
 * - ENOENT when a component in pathname does not exist
 * - EFAULT when pathname points outside the accessible address space
 * - ENOTDIR when a component used as a directory in pathname is not,
 *   in fact, a directory
 * - ENAMETOOLONG when pathname is too long
 */

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
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

	TST_EXP_FAIL(unlink(tc->name), tc->exp_errno, "%s", tc->desc);
}

static void setup(void)
{
	size_t n;

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
