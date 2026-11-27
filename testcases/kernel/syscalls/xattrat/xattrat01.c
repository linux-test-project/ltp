// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Dacheng Lu
 */

/*\
 * Basic coverage for the *xattrat() syscall family:
 * setxattrat(2), getxattrat(2), listxattrat(2), removexattrat(2).
 *
 * The family was added in Linux 6.13.  This test exercises the plain path
 * case, AT_EMPTY_PATH with a readable file descriptor, size queries, and the
 * argument validation performed by the syscall entry points.
 */

#define _GNU_SOURCE
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/xattr.h>

#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

struct xattrat_args {
	uint64_t value;
	uint32_t size;
	uint32_t flags;
};

#define FNAME "xattrat_file"
#define XNAME "user.hermes"
#define XVAL  "hello"

static int list_contains(const char *list, long len, const char *name)
{
	const char *p = list;

	while (p < list + len) {
		if (!strcmp(p, name))
			return 1;
		p += strlen(p) + 1;
	}

	return 0;
}

static void test_basic(void)
{
	char buf[32] = {0};
	char list[128] = {0};
	struct xattrat_args args = {
		.value = (uintptr_t)XVAL,
		.size = sizeof(XVAL) - 1,
	};
	struct xattrat_args get = {
		.value = (uintptr_t)buf,
		.size = sizeof(buf),
	};

	tst_res(TINFO, "set/get/list/remove a user xattr");

	TST_EXP_PASS(tst_syscall(__NR_setxattrat, AT_FDCWD, FNAME, 0,
				 XNAME, &args, sizeof(args)));

	TST_EXP_VAL(tst_syscall(__NR_getxattrat, AT_FDCWD, FNAME, 0,
				XNAME, &get, sizeof(get)), (long)sizeof(XVAL) - 1);
	TST_EXP_EQ_STR(buf, XVAL);

	TST_EXP_POSITIVE(tst_syscall(__NR_listxattrat, AT_FDCWD, FNAME, 0,
				     list, sizeof(list)));
	if (TST_PASS)
		TST_EXP_EQ_LI(list_contains(list, TST_RET, XNAME), 1);

	TST_EXP_PASS(tst_syscall(__NR_removexattrat, AT_FDCWD, FNAME, 0,
				 XNAME));

	TST_EXP_FAIL(tst_syscall(__NR_getxattrat, AT_FDCWD, FNAME, 0,
				 XNAME, &get, sizeof(get)), ENODATA);
}

static void test_empty_path(void)
{
	char buf[32] = {0};
	int fd;
	struct xattrat_args args = {
		.value = (uintptr_t)XVAL,
		.size = sizeof(XVAL) - 1,
	};
	struct xattrat_args get = {
		.value = (uintptr_t)buf,
		.size = sizeof(buf),
	};

	tst_res(TINFO, "use AT_EMPTY_PATH with a readable fd");

	fd = SAFE_OPEN(FNAME, O_RDONLY);
	TST_EXP_PASS(tst_syscall(__NR_setxattrat, fd, "", AT_EMPTY_PATH,
				 XNAME, &args, sizeof(args)));
	TST_EXP_VAL(tst_syscall(__NR_getxattrat, fd, "", AT_EMPTY_PATH,
				XNAME, &get, sizeof(get)), (long)sizeof(XVAL) - 1);
	TST_EXP_EQ_STR(buf, XVAL);

	/* The fd-relative write must be visible by path. */
	memset(buf, 0, sizeof(buf));
	TST_EXP_VAL(getxattr(FNAME, XNAME, buf, sizeof(buf)),
		    (long)sizeof(XVAL) - 1);
	TST_EXP_EQ_STR(buf, XVAL);

	SAFE_CLOSE(fd);
	TST_EXP_PASS(tst_syscall(__NR_removexattrat, AT_FDCWD, FNAME, 0,
				 XNAME));
}

static void test_zero_length(void)
{
	struct xattrat_args empty = {0};
	struct xattrat_args get = {0};

	tst_res(TINFO, "set and query a zero-length value");

	TST_EXP_PASS(tst_syscall(__NR_setxattrat, AT_FDCWD, FNAME, 0,
				 "user.empty", &empty, sizeof(empty)));
	TST_EXP_VAL(tst_syscall(__NR_getxattrat, AT_FDCWD, FNAME, 0,
				"user.empty", &get, sizeof(get)), 0);
	TST_EXP_PASS(tst_syscall(__NR_removexattrat, AT_FDCWD, FNAME, 0,
				 "user.empty"));
}

static void test_errors(void)
{
	char buf[32] = {0};
	long page_size = getpagesize();
	struct xattrat_args args = {
		.value = (uintptr_t)XVAL,
		.size = sizeof(XVAL) - 1,
	};
	struct xattrat_args get = {
		.value = (uintptr_t)buf,
		.size = sizeof(buf),
	};

	tst_res(TINFO, "validate argument checking");

	TST_EXP_PASS(tst_syscall(__NR_setxattrat, AT_FDCWD, FNAME, 0,
				 XNAME, &args, sizeof(args)));

	/* size query for a non-empty value */
	get.size = 0;
	get.value = 0;
	TST_EXP_VAL(tst_syscall(__NR_getxattrat, AT_FDCWD, FNAME, 0,
				XNAME, &get, sizeof(get)), (long)sizeof(XVAL) - 1);
	TST_EXP_POSITIVE(tst_syscall(__NR_listxattrat, AT_FDCWD, FNAME, 0,
				     NULL, 0));

	get.size = 1;
	get.value = (uintptr_t)buf;
	TST_EXP_FAIL(tst_syscall(__NR_getxattrat, AT_FDCWD, FNAME, 0,
				 XNAME, &get, sizeof(get)), ERANGE);

	get.size = sizeof(buf);
	get.flags = 1;
	TST_EXP_FAIL(tst_syscall(__NR_getxattrat, AT_FDCWD, FNAME, 0,
				 XNAME, &get, sizeof(get)), EINVAL);
	get.flags = 0;

	TST_EXP_FAIL(tst_syscall(__NR_getxattrat, AT_FDCWD, FNAME, 0,
				 XNAME, &get, 8), EINVAL);

	TST_EXP_FAIL(tst_syscall(__NR_getxattrat, AT_FDCWD, FNAME, 0,
				 XNAME, &get, page_size + 1), E2BIG);

	TST_EXP_FAIL(tst_syscall(__NR_setxattrat, AT_FDCWD, FNAME,
				 0x80000000U, XNAME, &args, sizeof(args)),
		     EINVAL);

	args.flags = 0x80000000U;
	TST_EXP_FAIL(tst_syscall(__NR_setxattrat, AT_FDCWD, FNAME, 0,
				 XNAME, &args, sizeof(args)), EINVAL);
	args.flags = XATTR_CREATE;
	TST_EXP_FAIL(tst_syscall(__NR_setxattrat, AT_FDCWD, FNAME, 0,
				 XNAME, &args, sizeof(args)), EEXIST);
	args.flags = 0;

	TST_EXP_FAIL(tst_syscall(__NR_setxattrat, AT_FDCWD, FNAME, 0,
				 XNAME, &args, 8), EINVAL);

	TST_EXP_FAIL(tst_syscall(__NR_setxattrat, AT_FDCWD, FNAME, 0,
				 XNAME, &args, page_size + 1), E2BIG);

	TST_EXP_PASS(tst_syscall(__NR_removexattrat, AT_FDCWD, FNAME, 0,
				 XNAME));

	args.flags = XATTR_REPLACE;
	TST_EXP_FAIL(tst_syscall(__NR_setxattrat, AT_FDCWD, FNAME, 0,
				 XNAME, &args, sizeof(args)), ENODATA);

	TST_EXP_FAIL(tst_syscall(__NR_removexattrat, AT_FDCWD, FNAME, 0,
				 "user.missing"), ENODATA);
}

static void run(void)
{
	test_basic();
	test_empty_path();
	test_zero_length();
	test_errors();
}

static void setup(void)
{
	int fd;

	fd = SAFE_OPEN(FNAME, O_CREAT | O_RDWR, 0644);
	SAFE_CLOSE(fd);

	if (setxattr(FNAME, XNAME, XVAL, sizeof(XVAL) - 1, 0) < 0) {
		if (errno == EOPNOTSUPP)
			tst_brk(TCONF, "filesystem does not support user xattrs");
		tst_brk(TBROK | TERRNO, "setxattr() support probe failed");
	}
	SAFE_REMOVEXATTR(FNAME, XNAME);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_tmpdir = 1,
	.min_kver = "6.13",
	.tags = (const struct tst_tag[]) {
		{"linux-git", "6140be90ec70"},
		{}
	}
};
