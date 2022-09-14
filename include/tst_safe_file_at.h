// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 SUSE LLC <rpalethorpe@suse.com>
 */

#ifndef TST_SAFE_FILE_AT_H
#define TST_SAFE_FILE_AT_H

#include <sys/types.h>
#include <stdarg.h>

#define SAFE_OPENAT(dirfd, path, oflags, ...)			\
	safe_openat(__FILE__, __LINE__,				\
		    (dirfd), (path), (oflags), ## __VA_ARGS__)

#define SAFE_FILE_READAT(dirfd, path, buf, nbyte)			\
	safe_file_readat(__FILE__, __LINE__,				\
			 (dirfd), (path), (buf), (nbyte))


#define SAFE_FILE_PRINTFAT(dirfd, path, fmt, ...)			\
	safe_file_printfat(__FILE__, __LINE__,				\
			   (dirfd), (path), (fmt), __VA_ARGS__)

#define SAFE_UNLINKAT(dirfd, path, flags)				\
	safe_unlinkat(__FILE__, __LINE__, (dirfd), (path), (flags))

#define SAFE_FCHOWNAT(dirfd, path, owner, group, flags)			\
	safe_fchownat(__FILE__, __LINE__,				\
			(dirfd), (path), (owner), (group), (flags))

#define SAFE_FSTATAT(dirfd, path, statbuf, flags)			\
	safe_fstatat(__FILE__, __LINE__, (dirfd), (path), (statbuf), (flags))

const char *tst_decode_fd(const int fd)
			  __attribute__((warn_unused_result));

int safe_openat(const char *const file, const int lineno, const int dirfd,
                const char *const path, const int oflags, ...)
		__attribute__((nonnull, warn_unused_result));

ssize_t safe_file_readat(const char *const file, const int lineno,
			 const int dirfd, const char *const path,
			 char *const buf, const size_t nbyte)
			 __attribute__ ((nonnull));

int tst_file_vprintfat(const int dirfd, const char *const path,
		       const char *const fmt, va_list va)
		       __attribute__((nonnull));
int tst_file_printfat(const int dirfd, const char *const path,
		      const char *const fmt, ...)
		      __attribute__ ((format (printf, 3, 4), nonnull));

int safe_file_vprintfat(const char *const file, const int lineno,
			const int dirfd, const char *const path,
			const char *const fmt, va_list va)
			__attribute__ ((nonnull));

int safe_file_printfat(const char *const file, const int lineno,
		       const int dirfd, const char *const path,
		       const char *const fmt, ...)
		       __attribute__ ((format (printf, 5, 6), nonnull));

int safe_unlinkat(const char *const file, const int lineno,
		  const int dirfd, const char *const path, const int flags)
		  __attribute__ ((nonnull));

int safe_fchownat(const char *const file, const int lineno,
		  const int dirfd, const char *const path, uid_t owner,
		  gid_t group, int flags)
		  __attribute__ ((nonnull));

int safe_fstatat(const char *const file, const int lineno,
		 const int dirfd, const char *const path, struct stat *statbuf,
		 int flags)
		 __attribute__ ((nonnull));
#endif
