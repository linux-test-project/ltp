// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 * Copyright (c) 2025 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Basic test for O_PATH flag of :man2:`open`:
 *
 *    Obtain a file descriptor that can be used to perform operations
 *    that act purely at the file descriptor level, the file itself is
 *    not opened, the operations :man2:`read`, :man2:`write`, :man2:`fchmod`,
 *    :man2:`fchown` and :man2:`fgetxattr` fail with the error EBADF.
 *
 * The operations include but are not limited to the syscalls above.
 */

#include "config.h"

#include <sys/ioctl.h>
#include <linux/fs.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/types.h>
#include <sys/xattr.h>
#endif

#include "tst_test.h"
#include "tst_safe_macros.h"
#include "lapi/fcntl.h"

#define TESTFILE	"testfile"

static int path_fd = -1, dup_fd = -1;

static int verify_read(int fd);
static int verify_write(int fd);
static int verify_fchmod(int fd);
static int verify_fchown(int fd);
static int verify_ioctl(int fd);
static int verify_mmap(int fd);
#ifdef HAVE_SYS_XATTR_H
static int verify_fgetxattr(int fd);
#endif

static const struct {
	int (*func)(int fd);
	const char *name;
} testcases[] = {
	{verify_read, "read"},
	{verify_write, "write"},
	{verify_fchmod, "fchmod"},
	{verify_fchown, "fchown"},
	{verify_ioctl, "ioctl"},
	{verify_mmap, "mmap"},
#ifdef HAVE_SYS_XATTR_H
	{verify_fgetxattr, "fgetxattr"},
#endif
	{}
};

static void setup(void)
{
	path_fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, 0644);
	SAFE_CLOSE(path_fd);
	path_fd = SAFE_OPEN(TESTFILE, O_PATH);
	dup_fd = SAFE_DUP(path_fd);
}

static void run(void)
{
	int i;

	for (i = 0; testcases[i].func; i++) {
		TST_EXP_FAIL(testcases[i].func(path_fd), EBADF,
			"%s() on original FD", testcases[i].name);
		TST_EXP_FAIL(testcases[i].func(dup_fd), EBADF,
			"%s() on duplicated FD", testcases[i].name);
	}
}

static int verify_read(int fd)
{
	char buf[255];

	return read(fd, buf, sizeof(buf));
}

static int verify_write(int fd)
{
	return write(fd, "w", 1);
}

static int verify_fchmod(int fd)
{
	return fchmod(fd, 0666);
}

static int verify_fchown(int fd)
{
	return fchown(fd, 1000, 1000);
}

static int verify_ioctl(int fd)
{
	int arg;

	return ioctl(fd, FIGETBSZ, &arg);
}

static int verify_mmap(int fd)
{
	void *ptr;

	ptr = mmap(NULL, 1, PROT_READ, MAP_PRIVATE, fd, 0);

	if (ptr == MAP_FAILED)
		return -1;

	SAFE_MUNMAP(ptr, 1);
	return 0;
}

#ifdef HAVE_SYS_XATTR_H
static int verify_fgetxattr(int fd)
{
	return fgetxattr(fd, "tkey", NULL, 0);
}
#endif

static void cleanup(void)
{
	if (path_fd >= 0)
		SAFE_CLOSE(path_fd);

	if (dup_fd >= 0)
		SAFE_CLOSE(dup_fd);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.cleanup = cleanup,
	.needs_tmpdir = 1
};
