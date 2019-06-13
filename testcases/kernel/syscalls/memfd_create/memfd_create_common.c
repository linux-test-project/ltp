// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017  Red Hat, Inc.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "lapi/fallocate.h"
#include "lapi/fcntl.h"
#include "lapi/memfd.h"

#include "lapi/syscalls.h"

#include "memfd_create_common.h"

int sys_memfd_create(const char *name, unsigned int flags)
{
	return tst_syscall(__NR_memfd_create, name, flags);
}

int check_fallocate(const char *filename, const int lineno, int fd,
			int mode, off_t offset, off_t len)
{
	int r;

	r = fallocate(fd, mode, offset, len);
	if (r < 0) {
		tst_brk_(filename, lineno, TFAIL | TERRNO,
			"fallocate(%d, %d, %ld, %ld) failed", fd, mode,
			offset, len);
	}

	tst_res_(filename, lineno, TPASS,
		"fallocate(%d, %d, %ld, %ld) succeeded", fd, mode,
		offset, len);

	return r;
}

int check_fallocate_fail(const char *filename, const int lineno, int fd,
				int mode, off_t offset, off_t len)
{
	int r;

	r = fallocate(fd, mode, offset, len);
	if (r >= 0) {
		tst_res_(filename, lineno, TFAIL,
			"fallocate(%d, %d, %ld, %ld) succeeded unexpectedly",
			fd, mode, offset, len);

		return r;
	}

	tst_res_(filename, lineno, TPASS | TERRNO,
		"fallocate(%d, %d, %ld, %ld) failed as expected", fd,
		mode, offset, len);

	return r;
}

void check_ftruncate(const char *filename, const int lineno, int fd,
			off_t length)
{
	safe_ftruncate(filename, lineno, fd, length);

	tst_res_(filename, lineno, TPASS, "ftruncate(%d, %ld) succeeded", fd,
		length);
}

void check_ftruncate_fail(const char *filename, const int lineno,
				int fd, off_t length)
{
	if (ftruncate(fd, length) >= 0) {
		tst_res_(filename, lineno, TFAIL,
			"ftruncate(%d, %ld) succeeded unexpectedly",
			fd, length);

		return;
	}

	tst_res_(filename, lineno, TPASS | TERRNO,
		"ftruncate(%d, %ld) failed as expected", fd, length);
}

int get_mfd_all_available_flags(const char *filename, const int lineno)
{
	unsigned int i;
	int flag;
	int flags2test[] = FLAGS_ALL_ARRAY_INITIALIZER;
	int flags_available = 0;

	if (!MFD_FLAGS_AVAILABLE(0)) {
		tst_brk_(filename, lineno, TCONF,
				"memfd_create(0) not implemented");
	}

	for (i = 0; i < ARRAY_SIZE(flags2test); i++) {
		flag = flags2test[i];

		if (MFD_FLAGS_AVAILABLE(flag))
			flags_available |= flag;
	}

	return flags_available;
}

int mfd_flags_available(const char *filename, const int lineno,
		unsigned int flags)
{
	TEST(sys_memfd_create("dummy_call", flags));
	if (TST_RET < 0) {
		if (TST_ERR != EINVAL) {
			tst_brk_(filename, lineno, TBROK | TTERRNO,
					"memfd_create() failed");
		}

		return 0;
	}

	SAFE_CLOSE(TST_RET);

	return 1;
}

int check_mfd_new(const char *filename, const int lineno,
			const char *name, loff_t sz, int flags)
{
	int fd;

	fd = sys_memfd_create(name, flags);
	if (fd < 0) {
		tst_brk_(filename, lineno, TBROK | TERRNO,
			"memfd_create(%s, %d) failed", name, flags);
	}

	tst_res_(filename, lineno, TPASS, "memfd_create(%s, %d) succeeded",
		name, flags);

	check_ftruncate(filename, lineno, fd, sz);

	return fd;
}

void check_mfd_fail_new(const char *filename, const int lineno,
			const char *name, int flags)
{
	int fd;

	fd = sys_memfd_create(name, flags);
	if (fd >= 0) {
		safe_close(filename, lineno, NULL, fd);
		tst_brk_(filename, lineno, TFAIL,
			 "memfd_create(%s, %d) succeeded unexpectedly",
			name, flags);
	}

	tst_res_(filename, lineno, TPASS | TERRNO,
		"memfd_create(%s, %d) failed as expected", name, flags);
}

void *check_mmap(const char *file, const int lineno, void *addr, size_t length,
		int prot, int flags, int fd, off_t offset)
{
	void *p;

	p = safe_mmap(file, lineno, addr, length, prot, flags, fd, offset);

	tst_res_(file, lineno, TPASS,
		"mmap(%p, %zu, %i, %i, %i, %li) succeeded", addr,
		length, prot, flags, fd, (long)offset);

	return p;
}

void check_mmap_fail(const char *file, const int lineno, void *addr,
		size_t length, int prot, int flags, int fd, off_t offset)
{
	if (mmap(addr, length, prot, flags, fd, offset) != MAP_FAILED) {
		safe_munmap(file, lineno, NULL, addr, length);
		tst_res_(file, lineno, TFAIL,
			"mmap(%p, %zu, %i, %i, %i, %li) succeeded unexpectedly",
			addr, length, prot, flags, fd, (long)offset);

		return;
	}

	tst_res_(file, lineno, TPASS | TERRNO,
		"mmap(%p, %zu, %i, %i, %i, %li) failed as expected",
		addr, length, prot, flags, fd, (long)offset);
}

void check_munmap(const char *file, const int lineno, void *p, size_t length)
{
	safe_munmap(file, lineno, NULL, p, length);

	tst_res_(file, lineno, TPASS, "munmap(%p, %zu) succeeded", p, length);
}

void check_mfd_has_seals(const char *file, const int lineno, int fd, int seals)
{
	int ret = SAFE_FCNTL((fd), F_GET_SEALS);
	if (ret	!= seals) {
		tst_brk_(file, lineno, TFAIL,
			"fd %d doesn't have expected seals (%d expected %d)",
			fd, ret, seals);
	}

	tst_res_(file, lineno, TPASS,
		 "fd %d has expected seals (%d)", fd, seals);
}

void check_mprotect(const char *file, const int lineno, void *addr,
		size_t length, int prot)
{
	if (mprotect(addr, length, prot) < 0) {
		tst_brk_(file, lineno, TFAIL | TERRNO,
			"mprotect(%p, %zu, %d) failed", addr, length, prot);
	}

	tst_res_(file, lineno, TPASS, "mprotect(%p, %zu, %d) succeeded", addr,
		length, prot);
}

void check_mfd_fail_add_seals(const char *filename, const int lineno,
				int fd, int seals)
{
	if (fcntl(fd, F_ADD_SEALS, seals) >= 0) {
		tst_brk_(filename, lineno, TFAIL,
			"fcntl(%d, F_ADD_SEALS) succeeded unexpectedly", fd);
	}

	tst_res_(filename, lineno, TPASS | TERRNO,
		"fcntl(%d, F_ADD_SEALS, %d) failed as expected", (fd),
		(seals));
}

void check_mfd_size(const char *filename, const int lineno, int fd,
			size_t size)
{
	struct stat st;

	safe_fstat(filename, lineno, fd, &st);

	if (st.st_size != (long)size) {
		tst_brk_(filename, lineno, TFAIL,
			"fstat(%d, &st): unexpected file size", fd);
	}

	tst_res_(filename, lineno, TPASS,
		"fstat(%d, &st): file size is correct", fd);
}

int check_mfd_open(const char *filename, const int lineno, int fd,
			int flags, mode_t mode)
{
	int r;
	char buf[512];

	sprintf(buf, "/proc/self/fd/%d", fd);

	r = safe_open(filename, lineno, NULL, buf, flags, mode);

	tst_res_(filename, lineno, TPASS, "open(%s, %d, %d) succeeded", buf,
		flags, mode);

	return r;
}

void check_mfd_fail_open(const char *filename, const int lineno, int fd,
				int flags, mode_t mode)
{
	char buf[512];

	sprintf(buf, "/proc/self/fd/%d", fd);

	fd = open(buf, flags, mode);
	if (fd > 0) {
		safe_close(filename, lineno, NULL, fd);
		tst_res_(filename, lineno, TFAIL,
			"open(%s, %d, %d) succeeded unexpectedly", buf,
			flags, mode);
	} else {
		tst_res_(filename, lineno, TPASS | TERRNO,
			"open(%s, %d, %d) failed as expected", buf,
			flags, mode);
	}
}

void check_mfd_readable(const char *filename, const int lineno, int fd)
{
	char buf[16];
	void *p;

	safe_read(filename, lineno, NULL, 1, fd, buf, sizeof(buf));
	tst_res_(filename, lineno, TPASS, "read(%d, %s, %zu) succeeded", fd,
		buf, sizeof(buf));

	/* verify PROT_READ *is* allowed */
	p = check_mmap(filename, lineno, NULL, MFD_DEF_SIZE, PROT_READ,
			MAP_PRIVATE, fd, 0);

	check_munmap(filename, lineno, p, MFD_DEF_SIZE);

	/* verify MAP_PRIVATE is *always* allowed (even writable) */
	p = check_mmap(filename, lineno, NULL, MFD_DEF_SIZE,
			PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

	check_munmap(filename, lineno, p, MFD_DEF_SIZE);
}

void check_mfd_writeable(const char *filename, const int lineno, int fd)
{
	void *p;

	/* verify write() succeeds */
	safe_write(filename, lineno, NULL, 1, fd, "\0\0\0\0", 4);
	tst_res_(filename, lineno, TPASS, "write(%d, %s, %d) succeeded", fd,
		"\\0\\0\\0\\0", 4);

	/* verify PROT_READ | PROT_WRITE is allowed */
	p = check_mmap(filename, lineno, NULL, MFD_DEF_SIZE,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	*(char *)p = 0;
	check_munmap(filename, lineno, p, MFD_DEF_SIZE);

	/* verify PROT_WRITE is allowed */
	p = check_mmap(filename, lineno, NULL, MFD_DEF_SIZE,
			PROT_WRITE, MAP_SHARED, fd, 0);

	*(char *)p = 0;
	check_munmap(filename, lineno, p, MFD_DEF_SIZE);

	/* verify PROT_READ with MAP_SHARED is allowed and a following
	 * mprotect(PROT_WRITE) allows writing
	 */
	p = check_mmap(filename, lineno, NULL, MFD_DEF_SIZE,
			PROT_READ, MAP_SHARED, fd, 0);

	check_mprotect(filename, lineno, p, MFD_DEF_SIZE,
			PROT_READ | PROT_WRITE);

	*(char *)p = 0;
	check_munmap(filename, lineno, p, MFD_DEF_SIZE);

	/* verify PUNCH_HOLE works */
	check_fallocate(filename, lineno, fd,
			FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, 0,
			MFD_DEF_SIZE);
}

void check_mfd_non_writeable(const char *filename, const int lineno,
				int fd)
{
	void *p;

	/* verify write() fails */
	TEST(write(fd, "data", 4));
	if (TST_RET < 0) {
		if (TST_ERR != EPERM) {
			tst_brk_(filename, lineno, TFAIL | TTERRNO,
				"write() didn't fail as expected");
		}
	} else {
		tst_brk_(filename, lineno, TFAIL,
			"write() succeeded unexpectedly");
	}
	tst_res_(filename, lineno, TPASS | TTERRNO, "write failed as expected");

	/* verify PROT_READ | PROT_WRITE is not allowed */
	check_mmap_fail(filename, lineno, NULL, MFD_DEF_SIZE,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	/* verify PROT_WRITE is not allowed */
	check_mmap_fail(filename, lineno, NULL, MFD_DEF_SIZE,
			PROT_WRITE, MAP_SHARED, fd, 0);

	/* Verify PROT_READ with MAP_SHARED with a following mprotect is not
	 * allowed. Note that for r/w the kernel already prevents the mmap.
	 */
	p = mmap(NULL, MFD_DEF_SIZE, PROT_READ, MAP_SHARED, fd, 0);
	if (p != MAP_FAILED) {
		if (mprotect(p, MFD_DEF_SIZE, PROT_READ | PROT_WRITE) >= 0) {
			tst_brk_(filename, lineno, TFAIL | TERRNO,
				"mmap()+mprotect() succeeded unexpectedly");
		}
	}

	/* verify PUNCH_HOLE fails */
	check_fallocate_fail(filename, lineno, fd,
			FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, 0,
			MFD_DEF_SIZE);
}

void check_mfd_shrinkable(const char *filename, const int lineno, int fd)
{
	int fd2;

	check_ftruncate(filename, lineno, fd, MFD_DEF_SIZE / 2);
	check_mfd_size(filename, lineno, fd, MFD_DEF_SIZE / 2);

	fd2 = check_mfd_open(filename, lineno, fd,
			O_RDWR | O_CREAT | O_TRUNC, 0600);
	safe_close(filename, lineno, NULL, fd2);

	check_mfd_size(filename, lineno, fd, 0);
}

void check_mfd_non_shrinkable(const char *filename, const int lineno, int fd)
{
	check_ftruncate_fail(filename, lineno, fd,  MFD_DEF_SIZE / 2);
	check_mfd_fail_open(filename, lineno, fd,
			O_RDWR | O_CREAT | O_TRUNC, 0600);
}

void check_mfd_growable(const char *filename, const int lineno, int fd)
{
	check_ftruncate(filename, lineno, fd, MFD_DEF_SIZE * 2);
	check_mfd_size(filename, lineno, fd, MFD_DEF_SIZE * 2);

	check_fallocate(filename, lineno, fd, 0, 0, MFD_DEF_SIZE * 4);
	check_mfd_size(filename, lineno, fd, MFD_DEF_SIZE * 4);
}

void check_mfd_non_growable(const char *filename, const int lineno, int fd)
{
	check_ftruncate_fail(filename, lineno, fd, MFD_DEF_SIZE * 2);
	check_fallocate_fail(filename, lineno, fd, 0, 0, MFD_DEF_SIZE * 4);
}

void check_mfd_growable_by_write(const char *filename, const int lineno, int fd)
{
	char buf[MFD_DEF_SIZE * 8];

	if (pwrite(fd, buf, sizeof(buf), 0) != sizeof(buf)) {
		tst_res_(filename, lineno, TFAIL | TERRNO,
			"pwrite(%d, %s, %zu, %d) failed",
			fd, buf, sizeof(buf), 0);

		return;
	}

	tst_res_(filename, lineno, TPASS, "pwrite(%d, %s, %zu, %d) succeeded",
		fd, buf, sizeof(buf), 0);

	check_mfd_size(filename, lineno, fd, MFD_DEF_SIZE * 8);
}

void check_mfd_non_growable_by_write(const char *filename, const int lineno,
					int fd)
{
	char buf[MFD_DEF_SIZE * 8];

	if (pwrite(fd, buf, sizeof(buf), 0) == sizeof(buf)) {
		tst_res_(filename, lineno, TFAIL,
			"pwrite(%d, %s, %zu, %d) didn't fail as expected",
			fd, buf, sizeof(buf), 0);

		return;
	}

	tst_res_(filename, lineno, TPASS, "pwrite(%d, %s, %zu, %d) succeeded",
		fd, buf, sizeof(buf), 0);
}
