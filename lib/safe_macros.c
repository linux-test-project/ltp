#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
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
	if (rval == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: basename(%s) failed",
			 file, lineno, path);
	}

	return rval;
}

int
safe_chdir(const char *file, const int lineno, void (*cleanup_fn) (void),
	   const char *path)
{
	int rval;

	rval = chdir(path);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: chdir(%s) failed",
			 file, lineno, path);
	}

	return rval;
}

int
safe_close(const char *file, const int lineno, void (*cleanup_fn) (void),
	   int fildes)
{
	int rval;

	rval = close(fildes);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: close(%d) failed",
			 file, lineno, fildes);
	}

	return rval;
}

int
safe_creat(const char *file, const int lineno, void (*cleanup_fn) (void),
	   char *pathname, mode_t mode)
{
	int rval;

	rval = creat(pathname, mode);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: creat(%s,0%o) failed",
			 file, lineno, pathname, mode);
	}

	return rval;
}

char *safe_dirname(const char *file, const int lineno,
		   void (*cleanup_fn) (void), char *path)
{
	char *rval;

	rval = dirname(path);
	if (rval == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: dirname(%s) failed",
			 file, lineno, path);
	}

	return rval;
}

char *safe_getcwd(const char *file, const int lineno, void (*cleanup_fn) (void),
		  char *buf, size_t size)
{
	char *rval;

	rval = getcwd(buf, size);
	if (rval == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: getcwd(%p,%zu) failed",
			 file, lineno, buf, size);
	}

	return rval;
}

struct passwd *safe_getpwnam(const char *file, const int lineno,
			     void (*cleanup_fn) (void), const char *name)
{
	struct passwd *rval;

	rval = getpwnam(name);
	if (rval == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: getpwnam(%s) failed",
			 file, lineno, name);
	}

	return rval;
}

int
safe_getrusage(const char *file, const int lineno, void (*cleanup_fn) (void),
	       int who, struct rusage *usage)
{
	int rval;

	rval = getrusage(who, usage);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: getrusage(%d,%p) failed",
			 file, lineno, who, usage);
	}

	return rval;
}

void *safe_malloc(const char *file, const int lineno, void (*cleanup_fn) (void),
		  size_t size)
{
	void *rval;

	rval = malloc(size);
	if (rval == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: malloc(%zu) failed",
			 file, lineno, size);
	}

	return rval;
}

int safe_mkdir(const char *file, const int lineno, void (*cleanup_fn) (void),
               const char *pathname, mode_t mode)
{
	int rval;

	rval = mkdir(pathname, mode);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: mkdir(%s,0%o) failed",
			 file, lineno, pathname, mode);
	}

	return (rval);
}

int safe_rmdir(const char *file, const int lineno, void (*cleanup_fn) (void),
               const char *pathname)
{
	int rval;

	rval = rmdir(pathname);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: rmdir(%s) failed",
			 file, lineno, pathname);
	}

	return (rval);
}

int safe_munmap(const char *file, const int lineno, void (*cleanup_fn) (void),
                void *addr, size_t length)
{
	int rval;

	rval = munmap(addr, length);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: munmap(%p,%zu) failed",
			 file, lineno, addr, length);
	}

	return rval;
}

int safe_open(const char *file, const int lineno, void (*cleanup_fn) (void),
              const char *pathname, int oflags, ...)
{
	va_list ap;
	int rval;
	mode_t mode;

	va_start(ap, oflags);
	/* Inspired by: http://patchwork.ozlabs.org/patch/350432/ */
	mode = va_arg(ap, __typeof__(+(mode_t)0));
	va_end(ap);

	rval = open(pathname, oflags, mode);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: open(%s,%d,0%o) failed",
			 file, lineno, pathname, oflags, mode);
	}

	return rval;
}

int safe_pipe(const char *file, const int lineno, void (*cleanup_fn) (void),
              int fildes[2])
{
	int rval;

	rval = pipe(fildes);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: pipe({%d,%d}) failed",
			 file, lineno, fildes[0], fildes[1]);
	}

	return rval;
}

ssize_t safe_read(const char *file, const int lineno, void (*cleanup_fn) (void),
                  char len_strict, int fildes, void *buf, size_t nbyte)
{
	ssize_t rval;

	rval = read(fildes, buf, nbyte);
	if (rval == -1 || (len_strict && (size_t)rval != nbyte)) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: read(%d,%p,%zu) failed, returned %zd",
			 file, lineno, fildes, buf, nbyte, rval);
	}

	return rval;
}

int safe_setegid(const char *file, const int lineno, void (*cleanup_fn) (void),
                 gid_t egid)
{
	int rval;

	rval = setegid(egid);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: setegid(%u) failed",
			 file, lineno, (unsigned) egid);
	}

	return rval;
}

int safe_seteuid(const char *file, const int lineno, void (*cleanup_fn) (void),
                 uid_t euid)
{
	int rval;

	rval = seteuid(euid);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: seteuid(%u) failed",
			 file, lineno, (unsigned) euid);
	}

	return rval;
}

int safe_setgid(const char *file, const int lineno, void (*cleanup_fn) (void),
                gid_t gid)
{
	int rval;

	rval = setgid(gid);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: setgid(%u) failed",
			 file, lineno, (unsigned) gid);
	}

	return rval;
}

int safe_setuid(const char *file, const int lineno, void (*cleanup_fn) (void),
                uid_t uid)
{
	int rval;

	rval = setuid(uid);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: setuid(%u) failed",
			 file, lineno, (unsigned) uid);
	}

	return rval;
}

int safe_getresuid(const char *file, const int lineno, void (*cleanup_fn)(void),
		   uid_t *ruid, uid_t *euid, uid_t *suid)
{
	int rval;

	rval = getresuid(ruid, euid, suid);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: getresuid(%p, %p, %p) failed",
			 file, lineno, ruid, euid, suid);
	}

	return rval;
}

int safe_getresgid(const char *file, const int lineno, void (*cleanup_fn)(void),
		   gid_t *rgid, gid_t *egid, gid_t *sgid)
{
	int rval;

	rval = getresgid(rgid, egid, sgid);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: getresgid(%p, %p, %p) failed",
			 file, lineno, rgid, egid, sgid);
	}

	return rval;
}

int safe_unlink(const char *file, const int lineno, void (*cleanup_fn) (void),
                const char *pathname)
{
	int rval;

	rval = unlink(pathname);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: unlink(%s) failed",
			 file, lineno, pathname);
	}

	return rval;
}


int safe_link(const char *file, const int lineno,
              void (cleanup_fn)(void), const char *oldpath,
              const char *newpath)
{
	int rval;

	rval = link(oldpath, newpath);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
		         "%s:%d: link(%s,%s) failed",
			 file, lineno, oldpath, newpath);
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
		         "%s:%d: symlink(%s,%s) failed",
			 file, lineno, oldpath, newpath);
	}

	return rval;
}

ssize_t safe_write(const char *file, const int lineno, void (cleanup_fn) (void),
                   char len_strict, int fildes, const void *buf, size_t nbyte)
{
	ssize_t rval;

	rval = write(fildes, buf, nbyte);
	if ((len_strict == 0 && rval == -1) || (size_t)rval != nbyte) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: write(%d,%p,%zu) failed",
		         file, lineno, fildes, buf, rval);
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
	    || (errno != 0 && rval == 0)) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: strtol(%s) failed", file, lineno, str);
	}

	if (endptr == str || (*endptr != '\0' && *endptr != '\n')) {
		tst_brkm(TBROK, cleanup_fn,
			 "%s:%d: strtol(%s): Invalid value", file, lineno, str);
	}

	if (rval > max || rval < min) {
		tst_brkm(TBROK, cleanup_fn,
			 "%s:%d: strtol(%s): %ld is out of range %ld - %ld",
			 file, lineno, str, rval, min, max);
	}

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
	    || (errno != 0 && rval == 0)) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: strtoul(%s) failed", file, lineno, str);
	}

	if (rval > max || rval < min) {
		tst_brkm(TBROK, cleanup_fn,
			 "%s:%d: strtoul(%s): %lu is out of range %lu - %lu",
			 file, lineno, str, rval, min, max);
	}

	if (endptr == str || (*endptr != '\0' && *endptr != '\n')) {
		tst_brkm(TBROK, cleanup_fn,
			 "Invalid value: '%s' at %s:%d", str, file, lineno);
	}

	return rval;
}

long safe_sysconf(const char *file, const int lineno,
		  void (cleanup_fn) (void), int name)
{
	long rval;
	errno = 0;

	rval = sysconf(name);

	if (rval == -1) {
		if (errno) {
			tst_brkm(TBROK | TERRNO, cleanup_fn,
				 "%s:%d: sysconf(%d) failed",
				 file, lineno, name);
		} else {
			tst_resm(TINFO, "%s:%d: sysconf(%d): "
				 "queried option is not available"
				 " or there is no definite limit",
				 file, lineno, name);
		}
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
			 "%s:%d: chmod(%s,0%o) failed",
			 file, lineno, path, mode);
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
			 "%s:%d: fchmod(%d,0%o) failed",
			 file, lineno, fd, mode);
	}

	return rval;
}

int safe_chown(const char *file, const int lineno, void (cleanup_fn)(void),
			const char *path, uid_t owner, gid_t group)
{
	int rval;

	rval = chown(path, owner, group);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			"%s:%d: chown(%s,%d,%d) failed",
			file, lineno, path, owner, group);
	}

	return rval;
}

int safe_fchown(const char *file, const int lineno, void (cleanup_fn)(void),
                int fd, uid_t owner, gid_t group)
{
	int rval;

	rval = fchown(fd, owner, group);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
		         "%s:%d: fchown(%d,%d,%d) failed",
			 file, lineno, fd, owner, group);
	}

	return rval;
}

pid_t safe_wait(const char *file, const int lineno, void (cleanup_fn)(void),
                int *status)
{
	pid_t rval;

	rval = wait(status);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: wait(%p) failed",
			 file, lineno, status);
	}

	return rval;
}

pid_t safe_waitpid(const char *file, const int lineno, void (cleanup_fn)(void),
                   pid_t pid, int *status, int opts)
{
	pid_t rval;

	rval = waitpid(pid, status, opts);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: waitpid(%d,%p,%d) failed",
			 file, lineno, pid, status, opts);
	}

	return rval;
}

#ifdef __linux__
void *safe_memalign(const char *file, const int lineno,
		    void (*cleanup_fn) (void), size_t alignment, size_t size)
{
	void *rval;

	rval = memalign(alignment, size);
	if (rval == NULL)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "memalign failed at %s:%d",
			 file, lineno);

	return rval;
}
#endif

int safe_kill(const char *file, const int lineno, void (cleanup_fn)(void),
	      pid_t pid, int sig)
{
	int rval;

	rval = kill(pid, sig);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: kill(%d,%s) failed",
			 file, lineno, pid, tst_strsig(sig));
	}

	return rval;
}

int safe_mkfifo(const char *file, const int lineno,
                void (*cleanup_fn)(void), const char *pathname, mode_t mode)
{
	int rval;

	rval = mkfifo(pathname, mode);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
		         "%s:%d: mkfifo(%s, 0%o) failed",
			 file, lineno, pathname, mode);
	}

	return rval;
}

int safe_rename(const char *file, const int lineno, void (*cleanup_fn)(void),
		const char *oldpath, const char *newpath)
{
	int rval;

	rval = rename(oldpath, newpath);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: rename(%s, %s) failed",
			 file, lineno, oldpath, newpath);
	}

	return rval;
}
