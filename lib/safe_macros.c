// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2010-2020
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/xattr.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>
#include "lapi/fcntl.h"
#include "test.h"
#include "safe_macros.h"

char *safe_basename(const char *file, const int lineno,
		    void (*cleanup_fn) (void), char *path)
{
	char *rval;

	rval = basename(path);

	if (rval == NULL) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"basename(%s) failed", path);
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
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"chdir(%s) failed", path);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid chdir(%s) return value %d", path, rval);
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
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"close(%d) failed", fildes);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid close(%d) return value %d", fildes, rval);
	}

	return rval;
}

int
safe_creat(const char *file, const int lineno, void (*cleanup_fn) (void),
	   const char *pathname, mode_t mode)
{
	int rval;

	rval = creat(pathname, mode);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"creat(%s,%04o) failed", pathname, mode);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid creat(%s,%04o) return value %d", pathname,
			mode, rval);
	}

	return rval;
}

char *safe_dirname(const char *file, const int lineno,
		   void (*cleanup_fn) (void), char *path)
{
	char *rval;

	rval = dirname(path);

	if (rval == NULL) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"dirname(%s) failed", path);
	}

	return rval;
}

char *safe_getcwd(const char *file, const int lineno, void (*cleanup_fn) (void),
		  char *buf, size_t size)
{
	char *rval;

	rval = getcwd(buf, size);

	if (rval == NULL) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"getcwd(%p,%zu) failed", buf, size);
	}

	return rval;
}

struct passwd *safe_getpwnam(const char *file, const int lineno,
			     void (*cleanup_fn) (void), const char *name)
{
	struct passwd *rval;

	rval = getpwnam(name);

	if (rval == NULL) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"getpwnam(%s) failed", name);
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
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"getrusage(%d,%p) failed", who, usage);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid getrusage(%d,%p) return value %d", who,
			usage, rval);
	}

	return rval;
}

void *safe_malloc(const char *file, const int lineno, void (*cleanup_fn) (void),
		  size_t size)
{
	void *rval;

	rval = malloc(size);

	if (rval == NULL) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"malloc(%zu) failed", size);
	}

	return rval;
}

int safe_mkdir(const char *file, const int lineno, void (*cleanup_fn) (void),
               const char *pathname, mode_t mode)
{
	int rval;

	rval = mkdir(pathname, mode);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"mkdir(%s, %04o) failed", pathname, mode);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid mkdir(%s, %04o) return value %d", pathname,
			mode, rval);
	}

	return (rval);
}

int safe_rmdir(const char *file, const int lineno, void (*cleanup_fn) (void),
               const char *pathname)
{
	int rval;

	rval = rmdir(pathname);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"rmdir(%s) failed", pathname);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid rmdir(%s) return value %d", pathname, rval);
	}

	return (rval);
}

int safe_munmap(const char *file, const int lineno, void (*cleanup_fn) (void),
                void *addr, size_t length)
{
	int rval;

	rval = munmap(addr, length);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"munmap(%p,%zu) failed", addr, length);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid munmap(%p,%zu) return value %d", addr,
			length, rval);
	}

	return rval;
}

int safe_open(const char *file, const int lineno, void (*cleanup_fn) (void),
              const char *pathname, int oflags, ...)
{
	int rval;
	mode_t mode = 0;

	if (TST_OPEN_NEEDS_MODE(oflags)) {
		va_list ap;

		va_start(ap, oflags);

		/* Android's NDK's mode_t is smaller than an int, which results in
		 * SIGILL here when passing the mode_t type.
		 */
		mode = va_arg(ap, int);

		va_end(ap);
	}

	rval = open(pathname, oflags, mode);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"open(%s,%d,%04o) failed", pathname, oflags, mode);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid open(%s,%d,%04o) return value %d", pathname,
			oflags, mode, rval);
	}

	return rval;
}

int safe_pipe(const char *file, const int lineno, void (*cleanup_fn) (void),
              int fildes[2])
{
	int rval;

	rval = pipe(fildes);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"pipe({%d,%d}) failed", fildes[0], fildes[1]);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid pipe({%d,%d}) return value %d", fildes[0],
			fildes[1], rval);
	}

	return rval;
}

ssize_t safe_read(const char *file, const int lineno, void (*cleanup_fn) (void),
                  char len_strict, int fildes, void *buf, size_t nbyte)
{
	ssize_t rval;

	rval = read(fildes, buf, nbyte);

	if (rval == -1 || (len_strict && (size_t)rval != nbyte)) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"read(%d,%p,%zu) failed, returned %zd", fildes, buf,
			nbyte, rval);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid read(%d,%p,%zu) return value %zd", fildes,
			buf, nbyte, rval);
	}

	return rval;
}

int safe_setegid(const char *file, const int lineno, void (*cleanup_fn) (void),
                 gid_t egid)
{
	int rval;

	rval = setegid(egid);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"setegid(%u) failed", (unsigned int)egid);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid setegid(%u) return value %d",
			(unsigned int)egid, rval);
	}

	return rval;
}

int safe_seteuid(const char *file, const int lineno, void (*cleanup_fn) (void),
                 uid_t euid)
{
	int rval;

	rval = seteuid(euid);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"seteuid(%u) failed", (unsigned int)euid);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid seteuid(%u) return value %d",
			(unsigned int)euid, rval);
	}

	return rval;
}

int safe_setgid(const char *file, const int lineno, void (*cleanup_fn) (void),
                gid_t gid)
{
	int rval;

	rval = setgid(gid);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"setgid(%u) failed", (unsigned int)gid);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid setgid(%u) return value %d",
			(unsigned int)gid, rval);
	}

	return rval;
}

int safe_setuid(const char *file, const int lineno, void (*cleanup_fn) (void),
                uid_t uid)
{
	int rval;

	rval = setuid(uid);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"setuid(%u) failed", (unsigned int)uid);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid setuid(%u) return value %d",
			(unsigned int)uid, rval);
	}

	return rval;
}

int safe_getresuid(const char *file, const int lineno, void (*cleanup_fn)(void),
		   uid_t *ruid, uid_t *euid, uid_t *suid)
{
	int rval;

	rval = getresuid(ruid, euid, suid);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"getresuid(%p, %p, %p) failed", ruid, euid, suid);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid getresuid(%p, %p, %p) return value %d", ruid,
			euid, suid, rval);
	}

	return rval;
}

int safe_getresgid(const char *file, const int lineno, void (*cleanup_fn)(void),
		   gid_t *rgid, gid_t *egid, gid_t *sgid)
{
	int rval;

	rval = getresgid(rgid, egid, sgid);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"getresgid(%p, %p, %p) failed", rgid, egid, sgid);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid getresgid(%p, %p, %p) return value %d", rgid,
			egid, sgid, rval);
	}

	return rval;
}

int safe_unlink(const char *file, const int lineno, void (*cleanup_fn) (void),
                const char *pathname)
{
	int rval;

	rval = unlink(pathname);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"unlink(%s) failed", pathname);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid unlink(%s) return value %d", pathname, rval);
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
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
		        "link(%s,%s) failed", oldpath, newpath);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
		        "Invalid link(%s,%s) return value %d", oldpath,
			newpath, rval);
	}

	return rval;
}

int safe_linkat(const char *file, const int lineno,
		void (cleanup_fn)(void), int olddirfd, const char *oldpath,
		int newdirfd, const char *newpath, int flags)
{
	int rval;

	rval = linkat(olddirfd, oldpath, newdirfd, newpath, flags);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"linkat(%d,%s,%d,%s,%d) failed", olddirfd, oldpath,
			newdirfd, newpath, flags);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid linkat(%d,%s,%d,%s,%d) return value %d",
			olddirfd, oldpath, newdirfd, newpath, flags, rval);
	}

	return rval;
}

ssize_t safe_readlink(const char *file, const int lineno,
		  void (cleanup_fn)(void), const char *path,
		  char *buf, size_t bufsize)
{
	ssize_t rval;

	rval = readlink(path, buf, bufsize);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"readlink(%s,%p,%zu) failed", path, buf, bufsize);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid readlink(%s,%p,%zu) return value %zd", path,
			buf, bufsize, rval);
	} else {
		/* readlink does not append a NUL byte to the buffer.
		 * Add it now. */
		if ((size_t) rval < bufsize)
			buf[rval] = '\0';
		else
			buf[bufsize-1] = '\0';
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
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"symlink(%s,%s) failed", oldpath, newpath);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid symlink(%s,%s) return value %d", oldpath,
			newpath, rval);
	}

	return rval;
}

ssize_t safe_write(const char *file, const int lineno, void (cleanup_fn) (void),
		   enum safe_write_opts len_strict, int fildes, const void *buf,
		   size_t nbyte)
{
	ssize_t rval;
	const void *wbuf = buf;
	size_t len = nbyte;
	int iter = 0;

	do {
		iter++;
		rval = write(fildes, wbuf, len);
		if (rval == -1) {
			if (len_strict == SAFE_WRITE_RETRY)
				tst_resm_(file, lineno, TINFO,
					"write() wrote %zu bytes in %d calls",
					nbyte-len, iter);
			tst_brkm_(file, lineno, TBROK | TERRNO,
				cleanup_fn, "write(%d,%p,%zu) failed",
				fildes, buf, nbyte);
		}

		if (len_strict == SAFE_WRITE_ANY)
			return rval;

		if (len_strict == SAFE_WRITE_ALL) {
			if ((size_t)rval != nbyte)
				tst_brkm_(file, lineno, TBROK | TERRNO,
					cleanup_fn, "short write(%d,%p,%zu) "
					"return value %zd",
					fildes, buf, nbyte, rval);
			return rval;
		}

		wbuf += rval;
		len -= rval;
	} while (len > 0);

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
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"strtol(%s) failed", str);
		return rval;
	}

	if (endptr == str || (*endptr != '\0' && *endptr != '\n')) {
		tst_brkm_(file, lineno, TBROK, cleanup_fn,
			"strtol(%s): Invalid value", str);
		return 0;
	}

	if (rval > max || rval < min) {
		tst_brkm_(file, lineno, TBROK, cleanup_fn,
			"strtol(%s): %ld is out of range %ld - %ld",
			str, rval, min, max);
		return 0;
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
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"strtoul(%s) failed", str);
		return rval;
	}

	if (endptr == str || (*endptr != '\0' && *endptr != '\n')) {
		tst_brkm_(file, lineno, TBROK, cleanup_fn,
			"Invalid value: '%s'", str);
		return 0;
	}

	if (rval > max || rval < min) {
		tst_brkm_(file, lineno, TBROK, cleanup_fn,
			"strtoul(%s): %lu is out of range %lu - %lu",
			str, rval, min, max);
		return 0;
	}

	return rval;
}

float safe_strtof(const char *file, const int lineno,
		  void (cleanup_fn) (void), char *str,
		  float min, float max)
{
	float rval;
	char *endptr;

	errno = 0;
	rval = strtof(str, &endptr);

	if (errno) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"strtof(%s) failed", str);
		return rval;
	}

	if (endptr == str || (*endptr != '\0' && *endptr != '\n')) {
		tst_brkm_(file, lineno, TBROK, cleanup_fn,
			"Invalid value: '%s'", str);
		return 0;
	}

	if (rval > max || rval < min) {
		tst_brkm_(file, lineno, TBROK, cleanup_fn,
			"strtof(%s): %f is out of range %f - %f",
			str, rval, min, max);
		return 0;
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
			tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
				"sysconf(%d) failed", name);
		} else {
			tst_resm_(file, lineno, TINFO,
				"sysconf(%d): queried option is not available or there is no definite limit",
				name);
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
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"chmod(%s,%04o) failed", path, mode);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid chmod(%s,%04o) return value %d", path, mode,
			rval);
	}

	return rval;
}

int safe_fchmod(const char *file, const int lineno,
                void (cleanup_fn)(void), int fd, mode_t mode)
{
	int rval;

	rval = fchmod(fd, mode);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"fchmod(%d,%04o) failed", fd, mode);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid fchmod(%d,%04o) return value %d", fd, mode,
			rval);
	}

	return rval;
}

int safe_chown(const char *file, const int lineno, void (cleanup_fn)(void),
			const char *path, uid_t owner, gid_t group)
{
	int rval;

	rval = chown(path, owner, group);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"chown(%s,%d,%d) failed", path, owner, group);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid chown(%s,%d,%d) return value %d", path,
			owner, group, rval);
	}

	return rval;
}

int safe_fchown(const char *file, const int lineno, void (cleanup_fn)(void),
                int fd, uid_t owner, gid_t group)
{
	int rval;

	rval = fchown(fd, owner, group);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"fchown(%d,%d,%d) failed", fd, owner, group);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid fchown(%d,%d,%d) return value %d", fd,
			owner, group, rval);
	}

	return rval;
}

pid_t safe_wait(const char *file, const int lineno, void (cleanup_fn)(void),
                int *status)
{
	pid_t rval;

	rval = wait(status);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"wait(%p) failed", status);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid wait(%p) return value %d", status, rval);
	}

	return rval;
}

pid_t safe_waitpid(const char *file, const int lineno, void (cleanup_fn)(void),
                   pid_t pid, int *status, int opts)
{
	pid_t rval;

	rval = waitpid(pid, status, opts);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"waitpid(%d,%p,%d) failed", pid, status, opts);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid waitpid(%d,%p,%d) return value %d", pid,
			status, opts, rval);
	}

	return rval;
}

void *safe_memalign(const char *file, const int lineno,
		    void (*cleanup_fn) (void), size_t alignment, size_t size)
{
	void *rval;

	rval = memalign(alignment, size);

	if (rval == NULL) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"memalign() failed");
	}

	return rval;
}

int safe_kill(const char *file, const int lineno, void (cleanup_fn)(void),
	      pid_t pid, int sig)
{
	int rval;

	rval = kill(pid, sig);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"kill(%d,%s) failed", pid, tst_strsig(sig));
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid kill(%d,%s) return value %d", pid,
			tst_strsig(sig), rval);
	}

	return rval;
}

int safe_mkfifo(const char *file, const int lineno,
                void (*cleanup_fn)(void), const char *pathname, mode_t mode)
{
	int rval;

	rval = mkfifo(pathname, mode);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"mkfifo(%s, %04o) failed", pathname, mode);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid mkfifo(%s, %04o) return value %d", pathname,
			mode, rval);
	}

	return rval;
}

int safe_rename(const char *file, const int lineno, void (*cleanup_fn)(void),
		const char *oldpath, const char *newpath)
{
	int rval;

	rval = rename(oldpath, newpath);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"rename(%s, %s) failed", oldpath, newpath);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid rename(%s, %s) return value %d", oldpath,
			newpath, rval);
	}

	return rval;
}

static const char *const fuse_fs_types[] = {
	"exfat",
	"ntfs",
};

static int possibly_fuse(const char *fs_type)
{
	unsigned int i;

	if (!fs_type)
		return 0;

	for (i = 0; i < ARRAY_SIZE(fuse_fs_types); i++) {
		if (!strcmp(fuse_fs_types[i], fs_type))
			return 1;
	}

	return 0;
}

int safe_mount(const char *file, const int lineno, void (*cleanup_fn)(void),
	       const char *source, const char *target,
	       const char *filesystemtype, unsigned long mountflags,
	       const void *data)
{
	int rval = -1;

	/*
	 * Don't try using the kernel's NTFS driver when mounting NTFS, since
	 * the kernel's NTFS driver doesn't have proper write support.
	 */
	if (!filesystemtype || strcmp(filesystemtype, "ntfs")) {
		rval = mount(source, target, filesystemtype, mountflags, data);
		if (!rval)
			return 0;
	}

	/*
	 * The FUSE filesystem executes mount.fuse helper, which tries to
	 * execute corresponding binary name which is encoded at the start of
	 * the source string and separated by # from the device name.
         *
	 * The mount helpers are called mount.$fs_type.
	 */
	if (possibly_fuse(filesystemtype)) {
		char buf[1024];

		tst_resm_(file, lineno, TINFO, "Trying FUSE...");
		snprintf(buf, sizeof(buf), "mount.%s '%s' '%s'",
			filesystemtype, source, target);

		rval = tst_system(buf);
		if (WIFEXITED(rval) && WEXITSTATUS(rval) == 0)
			return 0;

		tst_brkm_(file, lineno, TBROK, cleanup_fn,
			"mount.%s failed with %i", filesystemtype, rval);
		return -1;
	} else if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"mount(%s, %s, %s, %lu, %p) failed", source, target,
			filesystemtype, mountflags, data);
	} else {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid mount(%s, %s, %s, %lu, %p) return value %d",
			source, target, filesystemtype, mountflags, data,
			rval);
	}

	return rval;
}

int safe_umount(const char *file, const int lineno, void (*cleanup_fn)(void),
		const char *target)
{
	int rval;

	rval = tst_umount(target);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"umount(%s) failed", target);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid umount(%s) return value %d", target, rval);
	}

	return rval;
}

DIR* safe_opendir(const char *file, const int lineno, void (cleanup_fn)(void),
                  const char *name)
{
	DIR *rval;

	rval = opendir(name);

	if (!rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"opendir(%s) failed", name);
	}

	return rval;
}

int safe_closedir(const char *file, const int lineno, void (cleanup_fn)(void),
                  DIR *dirp)
{
	int rval;

	rval = closedir(dirp);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"closedir(%p) failed", dirp);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid closedir(%p) return value %d", dirp, rval);
	}

	return rval;
}

struct dirent *safe_readdir(const char *file, const int lineno, void (cleanup_fn)(void),
                            DIR *dirp)
{
	struct dirent *rval;
	int err = errno;

	errno = 0;
	rval = readdir(dirp);

	if (!rval && errno) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"readdir(%p) failed", dirp);
	}

	errno = err;
	return rval;
}

int safe_getpriority(const char *file, const int lineno, int which, id_t who)
{
	int rval, err = errno;

	errno = 0;
	rval = getpriority(which, who);

	if (rval == -1 && errno) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"getpriority(%i, %i) failed", which, who);
	} else if (errno) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"getpriority(%i, %i) failed with return value %d",
			which, who, rval);
	}

	errno = err;
	return rval;
}

ssize_t safe_getxattr(const char *file, const int lineno, const char *path,
		      const char *name, void *value, size_t size)
{
	ssize_t rval;

	rval = getxattr(path, name, value, size);

	if (rval == -1) {
		if (errno == ENOTSUP) {
			tst_brkm_(file, lineno, TCONF, NULL,
				"no xattr support in fs or mounted without user_xattr option");
			return rval;
		}

		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"getxattr(%s, %s, %p, %zu) failed",
			path, name, value, size);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid getxattr(%s, %s, %p, %zu) return value %zd",
			path, name, value, size, rval);
	}

	return rval;
}

int safe_setxattr(const char *file, const int lineno, const char *path,
		  const char *name, const void *value, size_t size, int flags)
{
	int rval;

	rval = setxattr(path, name, value, size, flags);

	if (rval == -1) {
		if (errno == ENOTSUP) {
			tst_brkm_(file, lineno, TCONF, NULL,
				"no xattr support in fs, mounted without user_xattr option "
				"or invalid namespace/name format");
			return rval;
		}

		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"setxattr(%s, %s, %p, %zu) failed",
			path, name, value, size);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid setxattr(%s, %s, %p, %zu) return value %d",
			path, name, value, size, rval);
	}

	return rval;
}

int safe_lsetxattr(const char *file, const int lineno, const char *path,
		   const char *name, const void *value, size_t size, int flags)
{
	int rval;

	rval = lsetxattr(path, name, value, size, flags);

	if (rval == -1) {
		if (errno == ENOTSUP) {
			tst_brkm_(file, lineno, TCONF, NULL,
				"no xattr support in fs, mounted without user_xattr option "
				"or invalid namespace/name format");
			return rval;
		}

		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"lsetxattr(%s, %s, %p, %zu, %i) failed",
			path, name, value, size, flags);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid lsetxattr(%s, %s, %p, %zu, %i) return value %d",
			path, name, value, size, flags, rval);
	}

	return rval;
}

int safe_fsetxattr(const char *file, const int lineno, int fd, const char *name,
		   const void *value, size_t size, int flags)
{
	int rval;

	rval = fsetxattr(fd, name, value, size, flags);

	if (rval == -1) {
		if (errno == ENOTSUP) {
			tst_brkm_(file, lineno, TCONF, NULL,
				"no xattr support in fs, mounted without user_xattr option "
				"or invalid namespace/name format");
			return rval;
		}

		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"fsetxattr(%i, %s, %p, %zu, %i) failed",
			fd, name, value, size, flags);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid fsetxattr(%i, %s, %p, %zu, %i) return value %d",
			fd, name, value, size, flags, rval);
	}

	return rval;
}

int safe_removexattr(const char *file, const int lineno, const char *path,
		const char *name)
{
	int rval;

	rval = removexattr(path, name);

	if (rval == -1) {
		if (errno == ENOTSUP) {
			tst_brkm_(file, lineno, TCONF, NULL,
				"no xattr support in fs or mounted without user_xattr option");
			return rval;
		}

		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"removexattr(%s, %s) failed", path, name);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid removexattr(%s, %s) return value %d", path,
			name, rval);
	}

	return rval;
}

int safe_lremovexattr(const char *file, const int lineno, const char *path,
		const char *name)
{
	int rval;

	rval = lremovexattr(path, name);

	if (rval == -1) {
		if (errno == ENOTSUP) {
			tst_brkm_(file, lineno, TCONF, NULL,
				"no xattr support in fs or mounted without user_xattr option");
			return rval;
		}

		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"lremovexattr(%s, %s) failed", path, name);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid lremovexattr(%s, %s) return value %d", path,
			name, rval);
	}

	return rval;
}

int safe_fremovexattr(const char *file, const int lineno, int fd,
		const char *name)
{
	int rval;

	rval = fremovexattr(fd, name);

	if (rval == -1) {
		if (errno == ENOTSUP) {
			tst_brkm_(file, lineno, TCONF, NULL,
				"no xattr support in fs or mounted without user_xattr option");
			return rval;
		}

		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"fremovexattr(%i, %s) failed", fd, name);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid fremovexattr(%i, %s) return value %d", fd,
			name, rval);
	}

	return rval;
}

int safe_fsync(const char *file, const int lineno, int fd)
{
	int rval;

	rval = fsync(fd);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"fsync(%i) failed", fd);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid fsync(%i) return value %d", fd, rval);
	}

	return rval;
}

pid_t safe_setsid(const char *file, const int lineno)
{
	pid_t rval;

	rval = setsid();

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"setsid() failed");
	}

	return rval;
}

int safe_mknod(const char *file, const int lineno, const char *pathname,
	mode_t mode, dev_t dev)
{
	int rval;

	rval = mknod(pathname, mode, dev);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"mknod() failed");
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid mknod() return value %d", rval);
	}

	return rval;
}

int safe_mlock(const char *file, const int lineno, const void *addr,
	size_t len)
{
	int rval;

	rval = mlock(addr, len);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"mlock() failed");
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid mlock() return value %d", rval);
	}

	return rval;
}

int safe_munlock(const char *file, const int lineno, const void *addr,
	size_t len)
{
	int rval;

	rval = munlock(addr, len);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"munlock() failed");
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid munlock() return value %d", rval);
	}

	return rval;
}

int safe_mincore(const char *file, const int lineno, void *start,
	size_t length, unsigned char *vec)
{
	int rval;

	rval = mincore(start, length, vec);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"mincore() failed");
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid mincore() return value %d", rval);
	}

	return rval;
}

int safe_sysinfo(const char *file, const int lineno, struct sysinfo *info)
{
	int ret;

	errno = 0;
	ret = sysinfo(info);

	if (ret == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"sysinfo() failed");
	} else if (ret) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid sysinfo() return value %d", ret);
	}

	return ret;
}
