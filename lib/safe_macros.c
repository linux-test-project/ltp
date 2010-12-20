#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdarg.h>
#include <unistd.h>
#include "test.h"

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
