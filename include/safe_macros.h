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
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdarg.h>
#include <unistd.h>

char*	safe_basename(const char *file, const int lineno,
	    void (*cleanup_fn)(void), char *path);
#define SAFE_BASENAME(cleanup_fn, path)	\
	safe_basename(__FILE__, __LINE__, (cleanup_fn), (path))

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

#endif
#endif
