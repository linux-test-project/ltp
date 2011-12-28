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

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdarg.h>
#include <unistd.h>

char*	safe_basename(const char *file, const int lineno,
	    void (*cleanup_fn)(void), char *path);
#define SAFE_BASENAME(cleanup_fn, path)	\
	safe_basename(__FILE__, __LINE__, (cleanup_fn), (path))

int	safe_chdir(const char *file, const int lineno,
	    void (*cleanup_fn)(void), const char *path);
#define SAFE_CHDIR(cleanup_fn, path)	\
	safe_chdir(__FILE__, __LINE__, (cleanup_fn), (path))

int	safe_close(const char *file, const int lineno,
	    void (*cleanup_fn)(void), int fildes);
#define SAFE_CLOSE(cleanup_fn, fildes)	\
	safe_close(__FILE__, __LINE__, (cleanup_fn), (fildes))

int	safe_creat(const char *file, const int lineno,
	    void (*cleanup_fn)(void), char *pathname, mode_t mode);
#define SAFE_CREAT(cleanup_fn, pathname, mode)	\
	safe_creat(__FILE__, __LINE__, cleanup_fn, (pathname), (mode))

char*	safe_dirname(const char *file, const int lineno,
	    void (*cleanup_fn)(void), char *path);
#define SAFE_DIRNAME(cleanup_fn, path)	\
	safe_dirname(__FILE__, __LINE__, (cleanup_fn), (path))

char*	safe_getcwd(const char *file, const int lineno,
	    void (*cleanup_fn)(void), char *buf, size_t size);
#define	SAFE_GETCWD(cleanup_fn, buf, size)	\
	safe_getcwd(__FILE__, __LINE__, (cleanup_fn), (buf), (size))

struct passwd*	safe_getpwnam(const char *file, const int lineno,
	    void (*cleanup_fn)(void), const char *name);
#define SAFE_GETPWNAM(cleanup_fn, name)	\
	safe_getpwnam(__FILE__, __LINE__, cleanup_fn, (name))

int     safe_getrusage(const char *file, const int lineno,
	    void (*cleanup_fn)(void), int who, struct rusage *usage);
#define SAFE_GETRUSAGE(cleanup_fn, who, usage) \
	safe_getrusage(__FILE__, __LINE__, (cleanup_fn), (who), (usage))

void*	safe_malloc(const char *file, const int lineno,
	    void (*cleanup_fn)(void), size_t size);
#define SAFE_MALLOC(cleanup_fn, size)	\
	safe_malloc(__FILE__, __LINE__, (cleanup_fn), (size))

int	safe_mkdir(const char *file, const int lineno,
	    void (*cleanup_fn)(void), const char *pathname, mode_t mode);
#define SAFE_MKDIR(cleanup_fn, pathname, mode)	\
	safe_mkdir(__FILE__, __LINE__, (cleanup_fn), (pathname), (mode))

void*	safe_mmap(const char *file, const int lineno,
	    void (*cleanup_fn)(void), void *addr, size_t length, int prot,
	    int flags, int fd, off_t offset);
#define SAFE_MMAP(cleanup_fn, addr, length, prot, flags, fd, offset)	\
	safe_mmap(__FILE__, __LINE__, (cleanup_fn), (addr), (length), (prot), \
	    (flags), (fd), (offset))

int	safe_munmap(const char *file, const int lineno,
	    void (*cleanup_fn)(void), void *addr, size_t length);
#define SAFE_MUNMAP(cleanup_fn, addr, length)	\
	safe_munmap(__FILE__, __LINE__, (cleanup_fn), (addr), (length))

int	safe_open(const char *file, const int lineno,
	    void (*cleanup_fn)(void), const char *pathname, int oflags, ...);
#define SAFE_OPEN(cleanup_fn, pathname, oflags, ...)	\
	safe_open(__FILE__, __LINE__, (cleanup_fn), (pathname), (oflags), \
	    ##__VA_ARGS__)

int	safe_pipe(const char *file, const int lineno,
	    void (*cleanup_fn)(void), int fildes[2]);
#define SAFE_PIPE(cleanup_fn, fildes)	\
	safe_pipe(__FILE__, __LINE__, cleanup_fn, (fildes))

ssize_t	safe_read(const char *file, const int lineno,
	    void (*cleanup_fn)(void), char len_strict, int fildes, void *buf,
	    size_t nbyte);
#define SAFE_READ(cleanup_fn, len_strict, fildes, buf, nbyte)	\
	safe_read(__FILE__, __LINE__, cleanup_fn, (len_strict), (fildes), \
	    (buf), (nbyte))

int	safe_setegid(const char *file, const int lineno,
	    void (*cleanup_fn)(void), gid_t egid);
#define SAFE_SETEGID(cleanup_fn, egid)	\
	safe_setegid(__FILE__, __LINE__, cleanup_fn, (egid))

int	safe_seteuid(const char *file, const int lineno,
	    void (*cleanup_fn)(void), uid_t euid);
#define SAFE_SETEUID(cleanup_fn, euid)	\
	safe_seteuid(__FILE__, __LINE__, cleanup_fn, (euid))

int	safe_setgid(const char *file, const int lineno,
	    void (*cleanup_fn)(void), gid_t gid);
#define SAFE_SETGID(cleanup_fn, gid)	\
	safe_setgid(__FILE__, __LINE__, cleanup_fn, (gid))

int	safe_setuid(const char *file, const int lineno,
	    void (*cleanup_fn)(void), uid_t uid);
#define SAFE_SETUID(cleanup_fn, uid)	\
	safe_setuid(__FILE__, __LINE__, cleanup_fn, (uid))

int	safe_unlink(const char *file, const int lineno,
	    void (*cleanup_fn)(void), const char *pathname);
#define SAFE_UNLINK(cleanup_fn, pathname)	\
	safe_unlink(__FILE__, __LINE__, cleanup_fn, (pathname))

ssize_t	safe_write(const char *file, const int lineno,
	    void (cleanup_fn)(void), char len_strict, int fildes,
	    const void *buf, size_t nbyte);
#define SAFE_WRITE(cleanup_fn, len_strict, fildes, buf, nbyte)	\
	safe_write(__FILE__, __LINE__, cleanup_fn, (len_strict), (fildes), \
	    (buf), (nbyte))

int safe_ftruncate(const char *file, const int lineno,
	    void (cleanup_fn)(void), int fd, off_t length);
#define SAFE_FTRUNCATE(cleanup_fn, fd, length) \
	safe_ftruncate(__FILE__, __LINE__, cleanup_fn, (fd), (length))

int safe_truncate(const char *file, const int lineno,
	    void (cleanup_fn)(void), const char *path, off_t length);
#define SAFE_TRUNCATE(cleanup_fn, fd, length) \
	safe_truncate(__FILE__, __LINE__, cleanup_fn, (path), (length))

#endif
#endif
