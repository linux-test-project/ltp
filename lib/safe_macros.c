#include <sys/types.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include "test.h"
#include "safe_macros.h"

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
safe_chdir(const char *file, const int lineno, void (*cleanup_fn)(void),
    const char *path)
{
	int rval;

	rval = chdir(path);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "chdir failed at %s:%d",
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

char *
safe_getcwd(const char *file, const int lineno, void (*cleanup_fn)(void),
    char *buf, size_t size)
{
	char *rval;

	rval = getcwd(buf, size);
	if (rval == NULL)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "getcwd failed at %s:%d",
		    file, lineno);

	return (rval);
}

struct passwd*
safe_getpwnam(const char *file, const int lineno, void (*cleanup_fn)(void),
    const char *name)
{
	struct passwd *rval;

	rval = getpwnam(name);
	if (rval == NULL)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "getpwnam failed at %s:%d",
		    file, lineno);

	return (rval);
}

int
safe_getrusage(const char *file, const int lineno, void (*cleanup_fn)(void),
	    int who, struct rusage *usage)
{
	int rval;

	rval = getrusage(who, usage);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "getrusage failed at %s:%d",
		    file, lineno);

	return rval;
}

void*
safe_malloc(const char *file, const int lineno, void (*cleanup_fn)(void),
    size_t size)
{
	void *rval;

	rval = malloc(size);
	if (rval == NULL)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "malloc failed at %s:%d",
		    file, lineno);

	return (rval);
}

int
safe_mkdir(const char *file, const int lineno, void (*cleanup_fn)(void),
    const char *pathname, mode_t mode)
{
	int rval;

	rval = mkdir(pathname, mode);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "mkdir failed at %s:%d",
		    file, lineno);

	return (rval);
}

void*
safe_mmap(const char *file, const int lineno, void (*cleanup_fn)(void),
    void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	void *rval;

	rval = mmap(addr, length, prot, flags, fd, offset);
	if (rval == MAP_FAILED)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "mmap failed at %s:%d",
		    file, lineno);

	return (rval);
}

int
safe_munmap(const char *file, const int lineno, void (*cleanup_fn)(void),
    void *addr, size_t length)
{
	int rval;

	rval = munmap(addr, length);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "munmap failed at %s:%d",
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
safe_setegid(const char *file, const int lineno, void (*cleanup_fn)(void),
    gid_t egid)
{
	int rval;

	rval = setegid(egid);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "setegid failed at %s:%d",
		    file, lineno);

	return (rval);
}

int
safe_seteuid(const char *file, const int lineno, void (*cleanup_fn)(void),
    uid_t euid)
{
	int rval;

	rval = seteuid(euid);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "seteuid failed at %s:%d",
		    file, lineno);

	return (rval);
}

int
safe_setgid(const char *file, const int lineno, void (*cleanup_fn)(void),
    gid_t gid)
{
	int rval;

	rval = setgid(gid);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "setgid failed at %s:%d",
		    file, lineno);

	return (rval);
}

int
safe_setuid(const char *file, const int lineno, void (*cleanup_fn)(void),
    uid_t uid)
{
	int rval;

	rval = setuid(uid);
	if (rval == -1)
		tst_brkm(TBROK|TERRNO, cleanup_fn, "setuid failed at %s:%d",
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

int safe_ftruncate(const char *file, const int lineno,
	    void (cleanup_fn)(void), int fd, off_t length)
{
	int rval;
	
	rval = ftruncate(fd, length);
	if (rval == -1) {
		tst_brkm(TBROK|TERRNO, cleanup_fn, "ftruncate failed at %s:%d",
		         file, lineno);
	}

	return rval;
}

int safe_truncate(const char *file, const int lineno,
	    void (cleanup_fn)(void), const char *path, off_t length)
{
	int rval;

	rval = truncate(path, length);
	if (rval == -1) {
		tst_brkm(TBROK|TERRNO, cleanup_fn, "truncate failed at %s:%d",
		         file, lineno);
	}

	return rval;
}
