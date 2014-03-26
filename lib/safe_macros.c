#include <sys/types.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include "test.h"
#include "safe_macros.h"

char *safe_basename(const char *file, const int lineno,
		    void (*cleanup_fn) (void), char *path)
{
	char *rval;

	rval = basename(path);
	if (rval == NULL)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "basename failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_chdir(const char *file, const int lineno, void (*cleanup_fn) (void),
	   const char *path)
{
	int rval;

	rval = chdir(path);
	if (rval == -1)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "chdir failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_close(const char *file, const int lineno, void (*cleanup_fn) (void),
	   int fildes)
{
	int rval;

	rval = close(fildes);
	if (rval == -1)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "close failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_creat(const char *file, const int lineno, void (*cleanup_fn) (void),
	   char *pathname, mode_t mode)
{
	int rval;

	rval = creat(pathname, mode);
	if (rval == -1)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "creat failed at %s:%d",
			 file, lineno);

	return (rval);
}

char *safe_dirname(const char *file, const int lineno,
		   void (*cleanup_fn) (void), char *path)
{
	char *rval;

	rval = dirname(path);
	if (rval == NULL)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "dirname failed at %s:%d",
			 file, lineno);

	return (rval);
}

char *safe_getcwd(const char *file, const int lineno, void (*cleanup_fn) (void),
		  char *buf, size_t size)
{
	char *rval;

	rval = getcwd(buf, size);
	if (rval == NULL)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "getcwd failed at %s:%d",
			 file, lineno);

	return (rval);
}

struct passwd *safe_getpwnam(const char *file, const int lineno,
			     void (*cleanup_fn) (void), const char *name)
{
	struct passwd *rval;

	rval = getpwnam(name);
	if (rval == NULL)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "getpwnam failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_getrusage(const char *file, const int lineno, void (*cleanup_fn) (void),
	       int who, struct rusage *usage)
{
	int rval;

	rval = getrusage(who, usage);
	if (rval == -1)
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "getrusage failed at %s:%d", file, lineno);

	return rval;
}

void *safe_malloc(const char *file, const int lineno, void (*cleanup_fn) (void),
		  size_t size)
{
	void *rval;

	rval = malloc(size);
	if (rval == NULL)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "malloc failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_mkdir(const char *file, const int lineno, void (*cleanup_fn) (void),
	   const char *pathname, mode_t mode)
{
	int rval;

	rval = mkdir(pathname, mode);
	if (rval == -1)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "mkdir failed at %s:%d",
			 file, lineno);

	return (rval);
}

void *safe_mmap(const char *file, const int lineno, void (*cleanup_fn) (void),
		void *addr, size_t length, int prot, int flags, int fd,
		off_t offset)
{
	void *rval;

	rval = mmap(addr, length, prot, flags, fd, offset);
	if (rval == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "mmap failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_munmap(const char *file, const int lineno, void (*cleanup_fn) (void),
	    void *addr, size_t length)
{
	int rval;

	rval = munmap(addr, length);
	if (rval == -1)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "munmap failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_open(const char *file, const int lineno, void (*cleanup_fn) (void),
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
		tst_brkm(TBROK | TERRNO, cleanup_fn, "open failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_pipe(const char *file, const int lineno, void (*cleanup_fn) (void),
	  int fildes[2])
{
	int rval;

	rval = pipe(fildes);
	if (rval == -1)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "pipe failed at %s:%d",
			 file, lineno);

	return (rval);
}

ssize_t
safe_read(const char *file, const int lineno, void (*cleanup_fn) (void),
	  char len_strict, int fildes, void *buf, size_t nbyte)
{
	ssize_t rval;

	rval = read(fildes, buf, nbyte);
	if (rval == -1 || (len_strict && (size_t)rval != nbyte))
		tst_brkm(TBROK | TERRNO, cleanup_fn, "read failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_setegid(const char *file, const int lineno, void (*cleanup_fn) (void),
	     gid_t egid)
{
	int rval;

	rval = setegid(egid);
	if (rval == -1)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "setegid failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_seteuid(const char *file, const int lineno, void (*cleanup_fn) (void),
	     uid_t euid)
{
	int rval;

	rval = seteuid(euid);
	if (rval == -1)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "seteuid failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_setgid(const char *file, const int lineno, void (*cleanup_fn) (void),
	    gid_t gid)
{
	int rval;

	rval = setgid(gid);
	if (rval == -1)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "setgid failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_setuid(const char *file, const int lineno, void (*cleanup_fn) (void),
	    uid_t uid)
{
	int rval;

	rval = setuid(uid);
	if (rval == -1)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "setuid failed at %s:%d",
			 file, lineno);

	return (rval);
}

int
safe_unlink(const char *file, const int lineno, void (*cleanup_fn) (void),
	    const char *pathname)
{
	int rval;

	rval = unlink(pathname);
	if (rval == -1)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "unlink failed at %s:%d",
			 file, lineno);

	return (rval);
}


int safe_link(const char *file, const int lineno,
              void (cleanup_fn)(void), const char *oldpath,
              const char *newpath)
{
	int rval;

	rval = link(oldpath, newpath);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
		         "link(%s, %s) failed at %s:%d",
			 oldpath, newpath, file, lineno);
	}

	return rval;
}

off_t safe_lseek(const char *file, const int lineno,
                 void (cleanup_fn)(void), int fd,
                 off_t offset, int whence)
{
	off_t rval;

	rval = lseek(fd, offset, whence);

	if (rval == (off_t) -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
		         "lseek(%i, %li, %i) failed at %s:%d",
		         fd, (long)offset, whence, file, lineno);
	}

	return rval;
}

int safe_symlink(const char *file, const int lineno,
                 void (cleanup_fn)(void), const char *oldpath,
                 const char *newpath)
{
	int rval;

	rval = symlink(oldpath, newpath);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
		         "link(%s, %s) failed at %s:%d",
			 oldpath, newpath, file, lineno);
	}

	return rval;
}

ssize_t
safe_write(const char *file, const int lineno, void (cleanup_fn) (void),
	   char len_strict, int fildes, const void *buf, size_t nbyte)
{
	ssize_t rval;

	rval = write(fildes, buf, nbyte);
	if ((len_strict == 0 && rval == -1) || (size_t)rval != nbyte)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "write failed at %s:%d",
			 file, lineno);

	return (rval);
}

int safe_ftruncate(const char *file, const int lineno,
		   void (cleanup_fn) (void), int fd, off_t length)
{
	int rval;

	rval = ftruncate(fd, length);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "ftruncate failed at %s:%d", file, lineno);
	}

	return rval;
}

int safe_truncate(const char *file, const int lineno,
		  void (cleanup_fn) (void), const char *path, off_t length)
{
	int rval;

	rval = truncate(path, length);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn, "truncate failed at %s:%d",
			 file, lineno);
	}

	return rval;
}

long safe_strtol(const char *file, const int lineno,
		 void (cleanup_fn) (void), char *str, long min, long max)
{
	long rval;
	char *endptr;

	errno = 0;
	rval = strtol(str, &endptr, 10);
	if ((errno == ERANGE && (rval == LONG_MAX || rval == LONG_MIN))
	    || (errno != 0 && rval == 0))
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "strtol failed at %s:%d", file, lineno);
	if (rval > max || rval < min)
		tst_brkm(TBROK, cleanup_fn,
			 "converted value out of range (%ld - %ld) at %s:%d",
			 min, max, file, lineno);
	if (endptr == str || (*endptr != '\0' && *endptr != '\n'))
		tst_brkm(TBROK, cleanup_fn,
			 "Invalid value: '%s' at %s:%d", str, file, lineno);

	return rval;
}

unsigned long safe_strtoul(const char *file, const int lineno,
			   void (cleanup_fn) (void), char *str,
			   unsigned long min, unsigned long max)
{
	unsigned long rval;
	char *endptr;

	errno = 0;
	rval = strtoul(str, &endptr, 10);
	if ((errno == ERANGE && rval == ULONG_MAX)
	    || (errno != 0 && rval == 0))
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "strtol failed at %s:%d", file, lineno);
	if (rval > max || rval < min)
		tst_brkm(TBROK, cleanup_fn,
			 "converted value out of range (%lu - %lu at %s:%d",
			 min, max, file, lineno);
	if (endptr == str || (*endptr != '\0' && *endptr != '\n'))
		tst_brkm(TBROK, cleanup_fn,
			 "Invalid value: '%s' at %s:%d", str, file, lineno);

	return rval;
}

long safe_sysconf(const char *file, const int lineno,
		  void (cleanup_fn) (void), int name)
{
	long rval;
	errno = 0;

	rval = sysconf(name);

	if (rval == -1) {
		if (errno == EINVAL)
			tst_brkm(TBROK | TERRNO, cleanup_fn,
				 "sysconf failed at %s:%d", file, lineno);
		else
			tst_resm(TINFO, "queried option is not available"
				 " or thers is no definite limit at %s:%d",
				 file, lineno);
	}

	return rval;
}

int safe_stat(const char *file, const int lineno,
	      void (cleanup_fn)(void), const char *path, struct stat *buf)
{
	int rval;

	rval = stat(path, buf);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "stat failed at %s:%d", file, lineno);
	}

	return rval;
}

int safe_fstat(const char *file, const int lineno,
	       void (cleanup_fn)(void), int fd, struct stat *buf)
{
	int rval;

	rval = fstat(fd, buf);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "fstat failed at %s:%d", file, lineno);
	}

	return rval;
}

int safe_lstat(const char *file, const int lineno,
	       void (cleanup_fn)(void), const char *path, struct stat *buf)
{
	int rval;

	rval = lstat(path, buf);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "lstat failed at %s:%d", file, lineno);
	}

	return rval;
}

int safe_getrlimit(const char *file, const int lineno,
		   void (cleanup_fn)(void), int resource, struct rlimit *rlim)
{
	int rval;

	rval = getrlimit(resource, rlim);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "getrlimit failed at %s:%d", file, lineno);
	}

	return rval;
}

int safe_setrlimit(const char *file, const int lineno, void (cleanup_fn)(void),
		   int resource, const struct rlimit *rlim)
{
	int rval;

	rval = setrlimit(resource, rlim);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "setrlimit failed at %s:%d", file, lineno);
	}

	return rval;
}

int safe_chmod(const char *file, const int lineno,
               void (cleanup_fn)(void), const char *path, mode_t mode)
{
	int rval;

	rval = chmod(path, mode);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "chmod failed at %s:%d", file, lineno);
	}

	return rval;
}

int safe_fchmod(const char *file, const int lineno,
                void (cleanup_fn)(void), int fd, mode_t mode)
{
	int rval;

	rval = fchmod(fd, mode);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "fchmod failed at %s:%d", file, lineno);
	}

	return rval;
}
