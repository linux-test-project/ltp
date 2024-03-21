// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2011  Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2012-2024
 */

/*\
 * [Description]
 *
 * Basic tests for getxattr(2) and make sure getxattr(2) handles error
 * conditions correctly.
 *
 * 1. Get an non-existing attribute, getxattr(2) should return -1 and set errno
 *    to ENODATA.
 * 2. Buffer size is smaller than attribute value size, getxattr(2) should
 *    return -1 and set errno to ERANGE.
 * 3. Get attribute, getxattr(2) should succeed, and the attribute got by
 *    getxattr(2) should be same as the value we set.
 */

#include <stdlib.h>
#include "tst_test.h"
#include <sys/xattr.h>

#define XATTR_TEST_NOKEY	"user.nosuchkey"
#define XATTR_TEST_KEY		"user.testkey"
#define XATTR_TEST_VALUE	"this is a test value"
#define XATTR_TEST_VALUE_SIZE	20
#define BUFFSIZE		64

static char filename[BUFSIZ];

static struct tcase {
	char *key;
	char *value;
	size_t size;
	int exp_err;
} tcases[] = {
	{ .key = XATTR_TEST_NOKEY, .size = BUFFSIZE - 1, .exp_err = ENODATA },
	{ .key = XATTR_TEST_KEY, .size = 1, .exp_err = ERANGE },
	{ .key = XATTR_TEST_KEY, .size = BUFFSIZE - 1 },
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (tc->exp_err) {
		TST_EXP_FAIL(getxattr(filename, tc->key, tc->value, tc->size),
			     tc->exp_err);
		return;
	}

	TST_EXP_VAL(getxattr(filename, tc->key, tc->value, tc->size),
			XATTR_TEST_VALUE_SIZE);

	if (memcmp(tc->value, XATTR_TEST_VALUE, XATTR_TEST_VALUE_SIZE))
		tst_res(TFAIL, "Wrong value, expect '%s' got '%s'",
			XATTR_TEST_VALUE, tc->value);
	else
		tst_res(TPASS, "getxattr() retrieved expected value");
}

static void setup(void)
{
	int fd;
	unsigned int i;

	/* Create test file and setup initial xattr */
	snprintf(filename, BUFSIZ, "getxattr01testfile");
	fd = SAFE_CREAT(filename, 0644);
	SAFE_CLOSE(fd);

	SAFE_SETXATTR(filename, XATTR_TEST_KEY, XATTR_TEST_VALUE,
		      strlen(XATTR_TEST_VALUE), XATTR_CREATE);

	for (i = 0; i < ARRAY_SIZE(tcases); i++)
		tcases[i].value = SAFE_MALLOC(BUFFSIZE);
}

static void cleanup(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(tcases); i++)
		free(tcases[i].value);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
};
