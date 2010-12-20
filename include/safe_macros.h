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

char *
safe_basename(const char *file, const int lineno, void (*cleanup_fn)(void),
    char *path)
{
	char *rval;

	rval = basename(path);
	if (rval == NULL)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "basename failed at %s:%d",
		    file, lineno);

	return (rval);
}
#define SAFE_BASENAME(cleanup_fn, path)	\
	safe_basename(__FILE__, __LINE__, (cleanup_fn), (path))

int
safe_close(const char *file, const int lineno, void (*cleanup_fn)(void),
    int fildes)
{
	int rval;

	rval = close(fildes);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "close failed at %s:%d",
		    file, lineno);

	return (rval);
}
#define SAFE_CLOSE(cleanup_fn, fildes)	\
	safe_close(__FILE__, __LINE__, (cleanup_fn), (fildes))

int
safe_creat(const char *file, const int lineno, void (*cleanup_fn)(void),
    char *pathname, mode_t mode)
{
	int rval;

	rval = creat(pathname, mode);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "pipe failed at %s:%d",
		    file, lineno);

	return (rval);
}
#define SAFE_CREAT(cleanup_fn, pathname, mode)	\
	safe_creat(__FILE__, __LINE__, cleanup_fn, (pathname), (mode))

char *
safe_dirname(const char *file, const int lineno, void (*cleanup_fn)(void),
    char *path)
{
	char *rval;

	rval = dirname(path);
	if (rval == NULL)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "dirname failed at %s:%d",
		    file, lineno);

	return (rval);
}
#define SAFE_DIRNAME(cleanup_fn, path)	\
	safe_dirname(__FILE__, __LINE__, (cleanup_fn), (path))

int
safe_open(const char *file, const int lineno, void (*cleanup_fn)(void),
    const char *pathname, int oflags, ...)
{
	va_list ap;
	int rval;
	mode_t mode;

	va_start(ap, oflags);
	mode = va_arg(ap, mode_t);
	va_end(ap);

	rval = open(pathname, oflags, mode);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "open failed at %s:%d",
		    file, lineno);

	return (rval);
}
#define SAFE_OPEN(cleanup_fn, pathname, oflags, ...)	\
	safe_open(__FILE__, __LINE__, (cleanup_fn), (pathname), (oflags), \
	    ##__VA_ARGS__)

int
safe_pipe(const char *file, const int lineno, void (*cleanup_fn)(void),
    int fildes[2])
{
	int rval;

	rval = pipe(fildes);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "pipe failed at %s:%d",
		    file, lineno);

	return (rval);
}
#define SAFE_PIPE(cleanup_fn, fildes)	\
	safe_pipe(__FILE__, __LINE__, cleanup_fn, (fildes))

ssize_t
safe_read(const char *file, const int lineno, void (*cleanup_fn)(void),
    char len_strict, int fildes, void *buf, size_t nbyte)
{
	ssize_t rval;

	rval = read(fildes, buf, nbyte);
	if ((len_strict == 0 && rval == -1) || rval != nbyte)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "read failed at %s:%d",
		    file, lineno);

	return (rval);
}
#define SAFE_READ(cleanup_fn, len_strict, fildes, buf, nbyte)	\
	safe_read(__FILE__, __LINE__, cleanup_fn, (len_strict), (fildes), \
	    (buf), (nbyte))

int
safe_unlink(const char *file, const int lineno, void (*cleanup_fn)(void),
    const char *pathname)
{
	int rval;

	rval = unlink(pathname);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "unlink failed at %s:%d",
		    file, lineno);

	return (rval);
}
#define SAFE_UNLINK(cleanup_fn, pathname)	\
	safe_unlink(__FILE__, __LINE__, cleanup_fn, (pathname))

ssize_t
safe_write(const char *file, const int lineno, void (cleanup_fn)(void),
    char len_strict, int fildes, const void *buf, size_t nbyte)
{
	ssize_t rval;

	rval = write(fildes, buf, nbyte);
	if ((len_strict == 0 && rval == -1) || rval != nbyte)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "write failed at %s:%d",
		    file, lineno);

	return (rval);
}
#define SAFE_WRITE(cleanup_fn, len_strict, fildes, buf, nbyte)	\
	safe_write(__FILE__, __LINE__, cleanup_fn, (len_strict), (fildes), \
	    (buf), (nbyte))

#endif
#endif
