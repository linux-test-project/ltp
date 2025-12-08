// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2017-2020
 * Copyright (C) 2017  Red Hat, Inc.
 * Port to LTP <jracek@redhat.com>
 */

/*\
 * Test based on :kselftest:`memfd/memfd_test.c`.
 */

#define _GNU_SOURCE

#include <errno.h>
#include "tst_test.h"
#include "memfd_create_common.h"

/*
 * Do few basic sealing tests to see whether setting/retrieving seals works.
 */
static void test_basic(int fd)
{
	/* add basic seals */
	CHECK_MFD_HAS_SEALS(fd, 0);
	CHECK_MFD_ADD_SEALS(fd, F_SEAL_SHRINK | F_SEAL_WRITE);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_SHRINK | F_SEAL_WRITE);

	/* add them again */
	CHECK_MFD_ADD_SEALS(fd, F_SEAL_SHRINK | F_SEAL_WRITE);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_SHRINK | F_SEAL_WRITE);

	/* add more seals and seal against sealing */
	CHECK_MFD_ADD_SEALS(fd, F_SEAL_GROW | F_SEAL_SEAL);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_SHRINK | F_SEAL_GROW |
			F_SEAL_WRITE | F_SEAL_SEAL);

	/* verify that sealing no longer works */
	CHECK_MFD_FAIL_ADD_SEALS(fd, F_SEAL_GROW);
	CHECK_MFD_FAIL_ADD_SEALS(fd, 0);
}

/*
 * Verify that no sealing is possible when memfd is created without
 * MFD_ALLOW_SEALING flag.
 */
static void test_no_sealing_without_flag(int fd)
{
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_SEAL);
	CHECK_MFD_FAIL_ADD_SEALS(fd,
		F_SEAL_SHRINK | F_SEAL_GROW | F_SEAL_WRITE);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_SEAL);
}

/*
 * Test SEAL_WRITE
 * Test whether SEAL_WRITE actually prevents modifications.
 */
static void test_seal_write(int fd)
{
	CHECK_MFD_HAS_SEALS(fd, 0);
	CHECK_MFD_ADD_SEALS(fd, F_SEAL_WRITE);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_WRITE);

	CHECK_MFD_READABLE(fd);
	CHECK_MFD_NON_WRITEABLE(fd);
	CHECK_MFD_SHRINKABLE(fd);
	CHECK_MFD_GROWABLE(fd);
	CHECK_MFD_NON_GROWABLE_BY_WRITE(fd);
}

/*
 * Test SEAL_SHRINK
 * Test whether SEAL_SHRINK actually prevents shrinking
 */
static void test_seal_shrink(int fd)
{
	CHECK_MFD_HAS_SEALS(fd, 0);
	CHECK_MFD_ADD_SEALS(fd, F_SEAL_SHRINK);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_SHRINK);

	CHECK_MFD_READABLE(fd);
	CHECK_MFD_WRITEABLE(fd);
	CHECK_MFD_NON_SHRINKABLE(fd);
	CHECK_MFD_GROWABLE(fd);
	CHECK_MFD_GROWABLE_BY_WRITE(fd);
}

/*
 * Test SEAL_GROW
 * Test whether SEAL_GROW actually prevents growing
 */
static void test_seal_grow(int fd)
{
	CHECK_MFD_HAS_SEALS(fd, 0);
	CHECK_MFD_ADD_SEALS(fd, F_SEAL_GROW);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_GROW);

	CHECK_MFD_READABLE(fd);
	CHECK_MFD_WRITEABLE(fd);
	CHECK_MFD_SHRINKABLE(fd);
	CHECK_MFD_NON_GROWABLE(fd);
	CHECK_MFD_NON_GROWABLE_BY_WRITE(fd);
}

/*
 * Test SEAL_SHRINK | SEAL_GROW
 * Test whether SEAL_SHRINK | SEAL_GROW actually prevents resizing
 */
static void test_seal_resize(int fd)
{
	CHECK_MFD_HAS_SEALS(fd, 0);
	CHECK_MFD_ADD_SEALS(fd, F_SEAL_SHRINK | F_SEAL_GROW);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_SHRINK | F_SEAL_GROW);

	CHECK_MFD_READABLE(fd);
	CHECK_MFD_WRITEABLE(fd);
	CHECK_MFD_NON_SHRINKABLE(fd);
	CHECK_MFD_NON_GROWABLE(fd);
	CHECK_MFD_NON_GROWABLE_BY_WRITE(fd);
}

/*
 * Test sharing via dup()
 * Test that seals are shared between dupped FDs and they're all equal.
 */
static void test_share_dup(int fd)
{
	int fd2;

	CHECK_MFD_HAS_SEALS(fd, 0);

	fd2 = SAFE_DUP(fd);
	CHECK_MFD_HAS_SEALS(fd2, 0);

	CHECK_MFD_ADD_SEALS(fd, F_SEAL_WRITE);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_WRITE);
	CHECK_MFD_HAS_SEALS(fd2, F_SEAL_WRITE);

	CHECK_MFD_ADD_SEALS(fd2, F_SEAL_SHRINK);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_WRITE | F_SEAL_SHRINK);
	CHECK_MFD_HAS_SEALS(fd2, F_SEAL_WRITE | F_SEAL_SHRINK);

	CHECK_MFD_ADD_SEALS(fd, F_SEAL_SEAL);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_WRITE | F_SEAL_SHRINK | F_SEAL_SEAL);
	CHECK_MFD_HAS_SEALS(fd2, F_SEAL_WRITE | F_SEAL_SHRINK | F_SEAL_SEAL);

	CHECK_MFD_FAIL_ADD_SEALS(fd, F_SEAL_GROW);
	CHECK_MFD_FAIL_ADD_SEALS(fd2, F_SEAL_GROW);
	CHECK_MFD_FAIL_ADD_SEALS(fd, F_SEAL_SEAL);
	CHECK_MFD_FAIL_ADD_SEALS(fd2, F_SEAL_SEAL);

	SAFE_CLOSE(fd2);

	CHECK_MFD_FAIL_ADD_SEALS(fd, F_SEAL_GROW);
}

/*
 * Test sealing with active mmap()s
 * Modifying seals is only allowed if no other mmap() refs exist.
 */
static void test_share_mmap(int fd)
{
	void *p;

	CHECK_MFD_HAS_SEALS(fd, 0);

	/* shared/writable ref prevents sealing WRITE, but allows others */
	p = SAFE_MMAP(NULL, MFD_DEF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
		fd, 0);

	CHECK_MFD_FAIL_ADD_SEALS(fd, F_SEAL_WRITE);
	CHECK_MFD_HAS_SEALS(fd, 0);
	CHECK_MFD_ADD_SEALS(fd, F_SEAL_SHRINK);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_SHRINK);
	SAFE_MUNMAP(p, MFD_DEF_SIZE);

	/* readable ref allows sealing */
	p = SAFE_MMAP(NULL, MFD_DEF_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
	CHECK_MFD_ADD_SEALS(fd, F_SEAL_WRITE);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_WRITE | F_SEAL_SHRINK);
	SAFE_MUNMAP(p, MFD_DEF_SIZE);
}

/*
 * Test sealing with open(/proc/self/fd/%d)
 * Via /proc we can get access to a separate file-context for the same memfd.
 * This is *not* like dup(), but like a real separate open(). Make sure the
 * semantics are as expected and we correctly check for RDONLY / WRONLY / RDWR.
 */
static void test_share_open(int fd)
{
	int fd2;

	CHECK_MFD_HAS_SEALS(fd, 0);

	fd2 = CHECK_MFD_OPEN(fd, O_RDWR, 0);
	CHECK_MFD_ADD_SEALS(fd, F_SEAL_WRITE);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_WRITE);
	CHECK_MFD_HAS_SEALS(fd2, F_SEAL_WRITE);

	CHECK_MFD_ADD_SEALS(fd2, F_SEAL_SHRINK);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_WRITE | F_SEAL_SHRINK);
	CHECK_MFD_HAS_SEALS(fd2, F_SEAL_WRITE | F_SEAL_SHRINK);

	SAFE_CLOSE(fd);
	fd = CHECK_MFD_OPEN(fd2, O_RDONLY, 0);

	CHECK_MFD_FAIL_ADD_SEALS(fd, F_SEAL_SEAL);
	CHECK_MFD_HAS_SEALS(fd, F_SEAL_WRITE | F_SEAL_SHRINK);
	CHECK_MFD_HAS_SEALS(fd2, F_SEAL_WRITE | F_SEAL_SHRINK);

	SAFE_CLOSE(fd2);
}


static const struct tcase {
	int flags;
	void (*func)(int fd);
	const char *desc;
} tcases[] = {
	{MFD_ALLOW_SEALING, &test_basic, "Basic tests + set/get seals"},
	{0,                 &test_no_sealing_without_flag, "Disabled sealing"},

	{MFD_ALLOW_SEALING, &test_seal_write, "Write seal"},
	{MFD_ALLOW_SEALING, &test_seal_shrink, "Shrink seal"},
	{MFD_ALLOW_SEALING, &test_seal_grow, "Grow seal"},
	{MFD_ALLOW_SEALING, &test_seal_resize, "Resize seal"},

	{MFD_ALLOW_SEALING, &test_share_dup, "Seals shared for dup"},
	{MFD_ALLOW_SEALING, &test_share_mmap, "Seals shared for mmap"},
	{MFD_ALLOW_SEALING, &test_share_open, "Seals shared for open"},
};

static void verify_memfd_create(unsigned int n)
{
	int fd;
	const struct tcase *tc;

	tc = &tcases[n];

	tst_res(TINFO, "%s", tc->desc);

	fd = CHECK_MFD_NEW("ltp_memfd_create01", MFD_DEF_SIZE, tc->flags);

	tc->func(fd);

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	/*
	 * For now, all tests in this file require MFD_ALLOW_SEALING flag
	 * to be implemented, even though that flag isn't always set when
	 * memfd is created. So don't check anything else and TCONF right away
	 * is this flag is missing.
	 */
	if (!MFD_FLAGS_AVAILABLE(MFD_ALLOW_SEALING)) {
		tst_brk(TCONF | TERRNO,
			"memfd_create(%u) not implemented", MFD_ALLOW_SEALING);
	}
}

static struct tst_test test = {
	.test = verify_memfd_create,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
};
