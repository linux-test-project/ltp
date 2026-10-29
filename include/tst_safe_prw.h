/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2010-2017 Linux Test Project
 */

#ifndef TST_SAFE_PRW_H__
#define TST_SAFE_PRW_H__

#include "lapi/uio.h"

static inline ssize_t safe_pread(const char *file, const int lineno,
		char len_strict, int fildes, void *buf, size_t nbyte,
		off_t offset)
{
	ssize_t rval;

	rval = pread(fildes, buf, nbyte, offset);

	if (rval == -1 || (len_strict && (size_t)rval != nbyte)) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"pread(%d,%p,%zu,%lld) failed",
			fildes, buf, nbyte, (long long)offset);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid pread(%d,%p,%zu,%lld) return value %zd",
			fildes, buf, nbyte, (long long)offset, rval);
	}

	return rval;
}
#define SAFE_PREAD(len_strict, fildes, buf, nbyte, offset) \
	safe_pread(__FILE__, __LINE__, (len_strict), (fildes), \
	           (buf), (nbyte), (offset))

static inline ssize_t safe_pwrite(const char *file, const int lineno,
		char len_strict, int fildes, const void *buf, size_t nbyte,
		off_t offset)
{
	ssize_t rval;

	rval = pwrite(fildes, buf, nbyte, offset);
	if (rval == -1 || (len_strict && (size_t)rval != nbyte)) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"pwrite(%d,%p,%zu,%lld) failed",
			fildes, buf, nbyte, (long long)offset);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid pwrite(%d,%p,%zu,%lld) return value %zd",
			fildes, buf, nbyte, (long long)offset, rval);
	}

	return rval;
}
#define SAFE_PWRITE(len_strict, fildes, buf, nbyte, offset) \
	safe_pwrite(__FILE__, __LINE__, (len_strict), (fildes), \
	            (buf), (nbyte), (offset))

static inline ssize_t safe_preadv(const char *file, const int lineno,
	char len_strict, int fildes, const struct iovec *iov, int iovcnt,
	off_t offset)
{
	ssize_t rval, nbyte;
	int i;

	for (i = 0, nbyte = 0; i < iovcnt; i++)
		nbyte += iov[i].iov_len;

	rval = preadv(fildes, iov, iovcnt, offset);

	if (rval == -1 || (len_strict && rval != nbyte)) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"preadv(%d,%p,%d,%lld) failed",
			fildes, iov, iovcnt, (long long)offset);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid preadv(%d,%p,%d,%lld) return value %zd",
			fildes, iov, iovcnt, (long long)offset, rval);
	}

	return rval;
}
#define SAFE_PREADV(len_strict, fildes, iov, iovcnt, offset) \
	safe_preadv(__FILE__, __LINE__, (len_strict), (fildes), \
		(iov), (iovcnt), (offset))

static inline ssize_t safe_pwritev(const char *file, const int lineno,
	char len_strict, int fildes, const struct iovec *iov, int iovcnt,
	off_t offset)
{
	ssize_t rval, nbyte;
	int i;

	for (i = 0, nbyte = 0; i < iovcnt; i++)
		nbyte += iov[i].iov_len;

	rval = pwritev(fildes, iov, iovcnt, offset);

	if (rval == -1 || (len_strict && rval != nbyte)) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"pwritev(%d,%p,%d,%lld) failed",
			fildes, iov, iovcnt, (long long)offset);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid pwritev(%d,%p,%d,%lld) return value %zd",
			fildes, iov, iovcnt, (long long)offset, rval);
	}

	return rval;
}
#define SAFE_PWRITEV(len_strict, fildes, iov, iovcnt, offset) \
	safe_pwritev(__FILE__, __LINE__, (len_strict), (fildes), \
		(iov), (iovcnt), (offset))

#endif /* SAFE_PRW_H__ */
