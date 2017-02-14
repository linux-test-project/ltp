/*
 * Safe macros for commonly used syscalls to reduce code duplication in LTP
 * testcases, and to ensure all errors are caught in said testcases as
 * gracefully as possible.
 *
 * Also satiates some versions of gcc/glibc when the warn_unused_result
 * attribute is applied to the function call.
 *
 * Licensed under the GPLv2.
 */

#ifndef __TEST_H__
#error "you must include test.h before this file"
#else

#ifndef __SAFE_MACROS_H__
#define __SAFE_MACROS_H__

#include "safe_macros_fn.h"
#include "old_safe_stdio.h"
#include "old_safe_net.h"

#define SAFE_BASENAME(cleanup_fn, path)	\
	safe_basename(__FILE__, __LINE__, (cleanup_fn), (path))

#define SAFE_CHDIR(cleanup_fn, path)	\
	safe_chdir(__FILE__, __LINE__, (cleanup_fn), (path))

#define SAFE_CLOSE(cleanup_fn, fd) ({ \
		int ret = safe_close(__FILE__, __LINE__, (cleanup_fn), (fd)); \
		fd = -1; \
		ret; \
	})

#define SAFE_CREAT(cleanup_fn, pathname, mode)	\
	safe_creat(__FILE__, __LINE__, cleanup_fn, (pathname), (mode))

#define SAFE_DIRNAME(cleanup_fn, path)	\
	safe_dirname(__FILE__, __LINE__, (cleanup_fn), (path))

#define	SAFE_GETCWD(cleanup_fn, buf, size)	\
	safe_getcwd(__FILE__, __LINE__, (cleanup_fn), (buf), (size))

#define SAFE_GETPWNAM(cleanup_fn, name)	\
	safe_getpwnam(__FILE__, __LINE__, cleanup_fn, (name))

#define SAFE_GETRUSAGE(cleanup_fn, who, usage) \
	safe_getrusage(__FILE__, __LINE__, (cleanup_fn), (who), (usage))

#define SAFE_MALLOC(cleanup_fn, size)	\
	safe_malloc(__FILE__, __LINE__, (cleanup_fn), (size))

#define SAFE_MKDIR(cleanup_fn, pathname, mode)	\
	safe_mkdir(__FILE__, __LINE__, (cleanup_fn), (pathname), (mode))

#define SAFE_RMDIR(cleanup_fn, pathname) \
	safe_rmdir(__FILE__, __LINE__, (cleanup_fn), (pathname))

#define SAFE_MUNMAP(cleanup_fn, addr, length)	\
	safe_munmap(__FILE__, __LINE__, (cleanup_fn), (addr), (length))

#define SAFE_OPEN(cleanup_fn, pathname, oflags, ...)	\
	safe_open(__FILE__, __LINE__, (cleanup_fn), (pathname), (oflags), \
	    ##__VA_ARGS__)

#define SAFE_PIPE(cleanup_fn, fildes)	\
	safe_pipe(__FILE__, __LINE__, cleanup_fn, (fildes))

#define SAFE_READ(cleanup_fn, len_strict, fildes, buf, nbyte)	\
	safe_read(__FILE__, __LINE__, cleanup_fn, (len_strict), (fildes), \
	    (buf), (nbyte))

#define SAFE_SETEGID(cleanup_fn, egid)	\
	safe_setegid(__FILE__, __LINE__, cleanup_fn, (egid))

#define SAFE_SETEUID(cleanup_fn, euid)	\
	safe_seteuid(__FILE__, __LINE__, cleanup_fn, (euid))

#define SAFE_SETGID(cleanup_fn, gid)	\
	safe_setgid(__FILE__, __LINE__, cleanup_fn, (gid))

#define SAFE_SETUID(cleanup_fn, uid) \
	safe_setuid(__FILE__, __LINE__, cleanup_fn, (uid))

#define SAFE_GETRESUID(cleanup_fn, ruid, euid, suid) \
	safe_getresuid(__FILE__, __LINE__, cleanup_fn, (ruid), (euid), (suid))

#define SAFE_GETRESGID(cleanup_fn, rgid, egid, sgid) \
	safe_getresgid(__FILE__, __LINE__, cleanup_fn, (rgid), (egid), (sgid))

#define SAFE_UNLINK(cleanup_fn, pathname) \
	safe_unlink(__FILE__, __LINE__, cleanup_fn, (pathname))

#define SAFE_LINK(cleanup_fn, oldpath, newpath) \
        safe_link(__FILE__, __LINE__, cleanup_fn, (oldpath), (newpath))

#define SAFE_LINKAT(cleanup_fn, olddirfd, oldpath, newdirfd, newpath, flags) \
	safe_linkat(__FILE__, __LINE__, cleanup_fn, (olddirfd), (oldpath), \
		    (newdirfd), (newpath), (flags))

#define SAFE_READLINK(cleanup_fn, path, buf, bufsize) \
	safe_readlink(__FILE__, __LINE__, cleanup_fn, (path), (buf), (bufsize))

#define SAFE_SYMLINK(cleanup_fn, oldpath, newpath) \
        safe_symlink(__FILE__, __LINE__, cleanup_fn, (oldpath), (newpath))

#define SAFE_WRITE(cleanup_fn, len_strict, fildes, buf, nbyte)	\
	safe_write(__FILE__, __LINE__, cleanup_fn, (len_strict), (fildes), \
	    (buf), (nbyte))

#define SAFE_STRTOL(cleanup_fn, str, min, max) \
	safe_strtol(__FILE__, __LINE__, cleanup_fn, (str), (min), (max))

#define SAFE_STRTOUL(cleanup_fn, str, min, max) \
	safe_strtoul(__FILE__, __LINE__, cleanup_fn, (str), (min), (max))

#define SAFE_SYSCONF(cleanup_fn, name) \
	safe_sysconf(__FILE__, __LINE__, cleanup_fn, name)

#define SAFE_CHMOD(cleanup_fn, path, mode) \
	safe_chmod(__FILE__, __LINE__, (cleanup_fn), (path), (mode))

#define SAFE_FCHMOD(cleanup_fn, fd, mode) \
	safe_fchmod(__FILE__, __LINE__, (cleanup_fn), (fd), (mode))

#define SAFE_CHOWN(cleanup_fn, path, owner, group) \
	safe_chown(__FILE__, __LINE__, (cleanup_fn), (path), (owner), (group))

#define SAFE_FCHOWN(cleanup_fn, fd, owner, group) \
	safe_fchown(__FILE__, __LINE__, (cleanup_fn), (fd), (owner), (group))

#define SAFE_WAIT(cleanup_fn, status) \
        safe_wait(__FILE__, __LINE__, (cleanup_fn), (status))

#define SAFE_WAITPID(cleanup_fn, pid, status, opts) \
        safe_waitpid(__FILE__, __LINE__, (cleanup_fn), (pid), (status), (opts))

#define SAFE_KILL(cleanup_fn, pid, sig) \
	safe_kill(__FILE__, __LINE__, (cleanup_fn), (pid), (sig))

#define SAFE_MEMALIGN(cleanup_fn, alignment, size) \
	safe_memalign(__FILE__, __LINE__, (cleanup_fn), (alignment), (size))

#define SAFE_MKFIFO(cleanup_fn, pathname, mode) \
	safe_mkfifo(__FILE__, __LINE__, (cleanup_fn), (pathname), (mode))

#define SAFE_RENAME(cleanup_fn, oldpath, newpath) \
	safe_rename(__FILE__, __LINE__, (cleanup_fn), (oldpath), (newpath))

#define SAFE_MOUNT(cleanup_fn, source, target, filesystemtype, \
		   mountflags, data) \
	safe_mount(__FILE__, __LINE__, (cleanup_fn), (source), (target), \
		   (filesystemtype), (mountflags), (data))

#define SAFE_UMOUNT(cleanup_fn, target) \
	safe_umount(__FILE__, __LINE__, (cleanup_fn), (target))

/*
 * following functions are inline because the behaviour may depend on
 * -D_FILE_OFFSET_BITS=64 -DOFF_T=__off64_t compile flags
 */

static inline void *safe_mmap(const char *file, const int lineno,
	void (*cleanup_fn)(void), void *addr, size_t length,
	int prot, int flags, int fd, off_t offset)
{
	void *rval;

	rval = mmap(addr, length, prot, flags, fd, offset);
	if (rval == MAP_FAILED) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: mmap(%p,%zu,%d,%d,%d,%ld) failed",
			 file, lineno, addr, length, prot, flags, fd,
			 (long) offset);
	}

	return rval;
}
#define SAFE_MMAP(cleanup_fn, addr, length, prot, flags, fd, offset) \
	safe_mmap(__FILE__, __LINE__, (cleanup_fn), (addr), (length), (prot), \
	(flags), (fd), (offset))

static inline int safe_ftruncate(const char *file, const int lineno,
	void (cleanup_fn) (void), int fd, off_t length)
{
	int rval;

	rval = ftruncate(fd, length);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: ftruncate(%d,%ld) failed",
			 file, lineno, fd, (long)length);
	}

	return rval;
}
#define SAFE_FTRUNCATE(cleanup_fn, fd, length) \
	safe_ftruncate(__FILE__, __LINE__, cleanup_fn, (fd), (length))

static inline int safe_truncate(const char *file, const int lineno,
	void (cleanup_fn) (void), const char *path, off_t length)
{
	int rval;

	rval = truncate(path, length);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: truncate(%s,%ld) failed",
			 file, lineno, path, (long)length);
	}

	return rval;
}
#define SAFE_TRUNCATE(cleanup_fn, path, length) \
	safe_truncate(__FILE__, __LINE__, cleanup_fn, (path), (length))

static inline int safe_stat(const char *file, const int lineno,
	void (cleanup_fn)(void), const char *path, struct stat *buf)
{
	int rval;

	rval = stat(path, buf);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: stat(%s,%p) failed", file, lineno, path, buf);
	}

	return rval;
}
#define SAFE_STAT(cleanup_fn, path, buf) \
	safe_stat(__FILE__, __LINE__, (cleanup_fn), (path), (buf))

static inline int safe_fstat(const char *file, const int lineno,
	void (cleanup_fn)(void), int fd, struct stat *buf)
{
	int rval;

	rval = fstat(fd, buf);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: fstat(%d,%p) failed", file, lineno, fd, buf);
	}

	return rval;
}
#define SAFE_FSTAT(cleanup_fn, fd, buf) \
	safe_fstat(__FILE__, __LINE__, (cleanup_fn), (fd), (buf))

static inline int safe_lstat(const char *file, const int lineno,
	void (cleanup_fn)(void), const char *path, struct stat *buf)
{
	int rval;

	rval = lstat(path, buf);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: lstat(%s,%p) failed", file, lineno, path, buf);
	}

	return rval;
}
#define SAFE_LSTAT(cleanup_fn, path, buf) \
	safe_lstat(__FILE__, __LINE__, (cleanup_fn), (path), (buf))

static inline off_t safe_lseek(const char *file, const int lineno,
	void (cleanup_fn)(void), int fd, off_t offset, int whence)
{
	off_t rval;

	rval = lseek(fd, offset, whence);

	if (rval == (off_t) -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			"%s:%d: lseek(%d,%ld,%d) failed",
			file, lineno, fd, (long)offset, whence);
	}

	return rval;
}
#define SAFE_LSEEK(cleanup_fn, fd, offset, whence) \
	safe_lseek(__FILE__, __LINE__, cleanup_fn, (fd), (offset), (whence))

static inline int safe_getrlimit(const char *file, const int lineno,
	void (cleanup_fn)(void), int resource, struct rlimit *rlim)
{
	int rval;

	rval = getrlimit(resource, rlim);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: getrlimit(%d,%p) failed",
			 file, lineno, resource, rlim);
	}

	return rval;
}
#define SAFE_GETRLIMIT(cleanup_fn, resource, rlim) \
	safe_getrlimit(__FILE__, __LINE__, (cleanup_fn), (resource), (rlim))

static inline int safe_setrlimit(const char *file, const int lineno,
	void (cleanup_fn)(void), int resource, const struct rlimit *rlim)
{
	int rval;

	rval = setrlimit(resource, rlim);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: setrlimit(%d,%p) failed",
			 file, lineno, resource, rlim);
	}

	return rval;
}
#define SAFE_SETRLIMIT(cleanup_fn, resource, rlim) \
	safe_setrlimit(__FILE__, __LINE__, (cleanup_fn), (resource), (rlim))

#define SAFE_OPENDIR(cleanup_fn, name) \
	safe_opendir(__FILE__, __LINE__, (cleanup_fn), (name))

#define SAFE_CLOSEDIR(cleanup_fn, dirp) \
	safe_closedir(__FILE__, __LINE__, (cleanup_fn), (dirp))

#define SAFE_READDIR(cleanup_fn, dirp) \
	safe_readdir(__FILE__, __LINE__, (cleanup_fn), (dirp))


#define SAFE_IOCTL(cleanup_fn, fd, request, ...)                   \
	({int ret = ioctl(fd, request, __VA_ARGS__);               \
	  if (ret < 0)                                             \
		tst_brkm(TBROK | TERRNO, cleanup_fn,               \
		         "ioctl(%i,%s,...) failed", fd, #request); \
	  ret;})

#endif /* __SAFE_MACROS_H__ */
#endif /* __TEST_H__ */
