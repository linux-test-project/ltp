/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2010-2024 Linux Test Project
 * Copyright (c) 2011-2015 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_SAFE_MACROS_INLINE_H__
#define TST_SAFE_MACROS_INLINE_H__

/*
 * Following functions are inline because the behaviour may depend on
 * -D_FILE_OFFSET_BITS=64 compile flag (type off_t or structures containing
 *  off_t fields), see man off_t(3type).
 *
 * Do not add other functions here.
 */

static inline int safe_ftruncate(const char *file, const int lineno,
	int fd, off_t length)
{
	int rval;

	rval = ftruncate(fd, length);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"ftruncate(%d,%ld) failed", fd, (long)length);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid ftruncate(%d,%ld) return value %d", fd,
			(long)length, rval);
	}

	return rval;
}

#define SAFE_FTRUNCATE(fd, length) \
	safe_ftruncate(__FILE__, __LINE__, (fd), (length))

static inline int safe_posix_fadvise(const char *file, const int lineno,
	int fd, off_t offset, off_t len, int advice)
{
	int rval;

	rval = posix_fadvise(fd, offset, len, advice);

	if (rval)
		tst_brk_(file, lineno, TBROK,
			"posix_fadvise(%d,%ld,%ld,%d) failed: %s",
			fd, (long)offset, (long)len, advice, tst_strerrno(rval));

	return rval;
}

#define SAFE_POSIX_FADVISE(fd, offset, len, advice) \
	safe_posix_fadvise(__FILE__, __LINE__, (fd), (offset), (len), (advice))

static inline int safe_truncate(const char *file, const int lineno,
	const char *path, off_t length)
{
	int rval;

	rval = truncate(path, length);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"truncate(%s,%ld) failed", path, (long)length);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid truncate(%s,%ld) return value %d", path,
			(long)length, rval);
	}

	return rval;
}

#define SAFE_TRUNCATE(path, length) \
	safe_truncate(__FILE__, __LINE__, (path), (length))

static inline int safe_stat(const char *file, const int lineno,
	const char *path, struct stat *buf)
{
	int rval;

	rval = stat(path, buf);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"stat(%s,%p) failed", path, buf);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid stat(%s,%p) return value %d", path, buf,
			rval);
	}

	return rval;
}

#define SAFE_STAT(path, buf) \
	safe_stat(__FILE__, __LINE__, (path), (buf))

static inline int safe_fstat(const char *file, const int lineno,
	int fd, struct stat *buf)
{
	int rval;

	rval = fstat(fd, buf);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"fstat(%d,%p) failed", fd, buf);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid fstat(%d,%p) return value %d", fd, buf, rval);
	}

	return rval;
}
#define SAFE_FSTAT(fd, buf) \
	safe_fstat(__FILE__, __LINE__, (fd), (buf))

static inline int safe_lstat(const char *file, const int lineno,
	const char *path, struct stat *buf)
{
	int rval;

	rval = lstat(path, buf);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"lstat(%s,%p) failed", path, buf);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid lstat(%s,%p) return value %d", path, buf,
			rval);
	}

	return rval;
}
#define SAFE_LSTAT(path, buf) \
	safe_lstat(__FILE__, __LINE__, (path), (buf))

static inline int safe_statfs(const char *file, const int lineno,
	const char *path, struct statfs *buf)
{
	int rval;

	rval = statfs(path, buf);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"statfs(%s,%p) failed", path, buf);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid statfs(%s,%p) return value %d", path, buf,
			rval);
	}

	return rval;
}

#define SAFE_STATFS(path, buf) \
	safe_statfs(__FILE__, __LINE__, (path), (buf))

static inline off_t safe_lseek(const char *file, const int lineno,
	int fd, off_t offset, int whence)
{
	off_t rval;

	rval = lseek(fd, offset, whence);

	if (rval == (off_t) -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"lseek(%d,%ld,%d) failed", fd, (long)offset, whence);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid lseek(%d,%ld,%d) return value %ld", fd,
			(long)offset, whence, (long)rval);
	}

	return rval;
}

#define SAFE_LSEEK(fd, offset, whence) \
	safe_lseek(__FILE__, __LINE__, (fd), (offset), (whence))

static inline int safe_getrlimit(const char *file, const int lineno,
	int resource, struct rlimit *rlim)
{
	int rval;

	rval = getrlimit(resource, rlim);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"getrlimit(%d,%p) failed", resource, rlim);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid getrlimit(%d,%p) return value %d", resource,
			rlim, rval);
	}

	return rval;
}

#define SAFE_GETRLIMIT(resource, rlim) \
	safe_getrlimit(__FILE__, __LINE__, (resource), (rlim))

static inline int safe_setrlimit(const char *file, const int lineno,
	int resource, const struct rlimit *rlim)
{
	int rval;

	rval = setrlimit(resource, rlim);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"setrlimit(%d,%p) failed", resource, rlim);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid setrlimit(%d,%p) return value %d", resource,
			rlim, rval);
	}

	return rval;
}

#define SAFE_SETRLIMIT(resource, rlim) \
	safe_setrlimit(__FILE__, __LINE__, (resource), (rlim))

void tst_prot_to_str(const int prot, char *buf);

static inline void *safe_mmap(const char *file, const int lineno,
	void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	void *rval;
	char prot_buf[512];

	tst_prot_to_str(prot, prot_buf);

	tst_res_(file, lineno, TDEBUG,
		"mmap(%p, %zu, %s(%x), %d, %d, %lld)",
		addr, length, prot_buf, prot, flags, fd, (long long int)offset);

	rval = mmap(addr, length, prot, flags, fd, offset);
	if (rval == MAP_FAILED) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"mmap(%p,%zu,%s(%x),%d,%d,%ld) failed",
			addr, length, prot_buf, prot, flags, fd, (long) offset);
	}

	return rval;
}

#define SAFE_MMAP(addr, length, prot, flags, fd, offset) \
	safe_mmap(__FILE__, __LINE__, (addr), (length), (prot), \
	(flags), (fd), (offset))

#endif /* TST_SAFE_MACROS_INLINE_H__ */
