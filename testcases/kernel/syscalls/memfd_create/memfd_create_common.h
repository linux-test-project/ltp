/*
 * Copyright (C) 2017  Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef MEMFD_TEST_COMMON
#define MEMFD_TEST_COMMON

#include <sys/types.h>
#include <lapi/fcntl.h>
#include <lapi/memfd.h>

/* change macros accordingly if any flags need to be added in the future */
#define FLAGS_ALL_ARRAY_INITIALIZER {MFD_CLOEXEC, MFD_ALLOW_SEALING}
#define FLAGS_ALL_MASK              (MFD_CLOEXEC | MFD_ALLOW_SEALING)

#define MFD_DEF_SIZE 8192

#define GET_MFD_ALL_AVAILABLE_FLAGS() \
	get_mfd_all_available_flags(__FILE__, __LINE__)

#define MFD_FLAGS_AVAILABLE(flags) \
	mfd_flags_available(__FILE__, __LINE__, (flags))

#define CHECK_MFD_NEW(name, sz, flags) \
	check_mfd_new(__FILE__, __LINE__, (name), (sz), (flags))

#define CHECK_MFD_FAIL_NEW(name, flags) \
	check_mfd_fail_new(__FILE__, __LINE__, (name), (flags))

#define CHECK_MMAP(addr, length, prot, flags, fd, offset) \
	check_mmap(__FILE__, __LINE__, (addr), (length), (prot), \
			(flags), (fd), (offset))

#define CHECK_MMAP_FAIL(addr, length, prot, flags, fd, offset) \
	check_mmap_fail(__FILE__, __LINE__, (addr), (length), (prot), \
			(flags), (fd), (offset))

#define CHECK_MUNMAP(p, length) \
	check_munmap(__FILE__, __LINE__, (p), (length))

#define CHECK_MFD_HAS_SEALS(fd, seals) \
	check_mfd_has_seals(__FILE__, __LINE__, (fd), (seals));

#define CHECK_MFD_ADD_SEALS(fd, seals) \
	({int r = SAFE_FCNTL((fd), F_ADD_SEALS, (seals)); \
	tst_res(TPASS, "fcntl(%d, F_ADD_SEALS, %d) succeeded", \
		(fd), (seals)); r; })

#define CHECK_MFD_FAIL_ADD_SEALS(fd, seals) \
	check_mfd_fail_add_seals(__FILE__, __LINE__, (fd), (seals))

#define CHECK_MFD_SIZE(fd, size) \
	check_mfd_size(__FILE__, __LINE__, (fd), (size))

#define CHECK_MFD_OPEN(fd, flags, mode) \
	check_mfd_open(__FILE__, __LINE__, (fd), (flags), (mode))

#define CHECK_MFD_FAIL_OPEN(fd, flags, mode) \
	check_mfd_fail_open(__FILE__, __LINE__, (fd), (flags), (mode))

#define CHECK_MFD_READABLE(fd) \
	check_mfd_readable(__FILE__, __LINE__, (fd))

#define CHECK_MFD_WRITEABLE(fd) \
	check_mfd_writeable(__FILE__, __LINE__, (fd))

#define CHECK_MFD_NON_WRITEABLE(fd) \
	check_mfd_non_writeable(__FILE__, __LINE__, (fd))

#define CHECK_MFD_SHRINKABLE(fd) \
	check_mfd_shrinkable(__FILE__, __LINE__, (fd))

#define CHECK_MFD_NON_SHRINKABLE(fd) \
	check_mfd_non_shrinkable(__FILE__, __LINE__, (fd))

#define CHECK_MFD_GROWABLE(fd) \
	check_mfd_growable(__FILE__, __LINE__, (fd))

#define CHECK_MFD_NON_GROWABLE(fd) \
	check_mfd_non_growable(__FILE__, __LINE__, (fd))

#define CHECK_MFD_GROWABLE_BY_WRITE(fd) \
	check_mfd_growable_by_write(__FILE__, __LINE__, (fd))

#define CHECK_MFD_NON_GROWABLE_BY_WRITE(fd) \
	check_mfd_non_growable_by_write(__FILE__, __LINE__, (fd))

int mfd_flags_available(const char *filename, const int lineno,
		unsigned int flags);

int get_mfd_all_available_flags(const char *filename, const int lineno);

int sys_memfd_create(const char *name, unsigned int flags);

int check_fallocate(const char *filename, const int lineno, int fd,
			int mode, off_t offset, off_t len);
int check_fallocate_fail(const char *filename, const int lineno, int fd,
			int mode, off_t offset, off_t len);
void check_ftruncate(const char *filename, const int lineno, int fd,
			off_t length);
void check_ftruncate_fail(const char *filename, const int lineno, int fd,
			off_t length);

int check_mfd_new(const char *filename, const int lineno,
			const char *name, loff_t sz, int flags);
void check_mfd_fail_new(const char *filename, const int lineno,
			const char *name, int flags);

void *check_mmap(const char *file, const int lineno, void *addr, size_t length,
		int prot, int flags, int fd, off_t offset);
void check_mmap_fail(const char *file, const int lineno, void *addr,
		size_t length, int prot, int flags, int fd, off_t offset);

void check_munmap(const char *file, const int lineno, void *p, size_t length);

void check_mfd_has_seals(const char *file, const int lineno, int fd, int seals);

void check_mprotect(const char *file, const int lineno, void *addr,
		size_t length, int prot);

void check_mfd_fail_add_seals(const char *filename, const int lineno, int fd,
			int seals);

void check_mfd_size(const char *filename, const int lineno, int fd,
		size_t size);

int check_mfd_open(const char *filename, const int lineno, int fd,
			int flags, mode_t mode);
void check_mfd_fail_open(const char *filename, const int lineno, int fd,
			int flags, mode_t mode);

void check_mfd_readable(const char *filename, const int lineno, int fd);

void check_mfd_writeable(const char *filename, const int lineno, int fd);
void check_mfd_non_writeable(const char *filename, const int lineno, int fd);

void check_mfd_shrinkable(const char *filename, const int lineno, int fd);
void check_mfd_non_shrinkable(const char *filename, const int lineno, int fd);

void check_mfd_growable(const char *filename, const int lineno, int fd);
void check_mfd_non_growable(const char *filename, const int lineno, int fd);

void check_mfd_growable_by_write(const char *filename, const int lineno,
				int fd);
void check_mfd_non_growable_by_write(const char *filename, const int lineno,
				int fd);

#endif
