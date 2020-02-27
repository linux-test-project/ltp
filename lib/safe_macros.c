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
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
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
	   const char *pathname, mode_t mode)
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

	/* Android's NDK's mode_t is smaller than an int, which results in
	 * SIGILL here when passing the mode_t type.
	 */
	mode = va_arg(ap, int);

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

int safe_linkat(const char *file, const int lineno,
		void (cleanup_fn)(void), int olddirfd, const char *oldpath,
		int newdirfd, const char *newpath, int flags)
{
	int rval;

	rval = linkat(olddirfd, oldpath, newdirfd, newpath, flags);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: linkat(%d,%s,%d,%s,%d) failed",
			 file, lineno, olddirfd, oldpath, newdirfd,
			 newpath, flags);
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
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: readlink(%s,%p,%zu) failed",
			 file, lineno, path, buf, bufsize);
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
	if (rval == -1 || (len_strict && (size_t)rval != nbyte)) {
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
		return rval;
	}

	if (endptr == str || (*endptr != '\0' && *endptr != '\n')) {
		tst_brkm(TBROK, cleanup_fn,
			 "%s:%d: strtol(%s): Invalid value", file, lineno, str);
		return 0;
	}

	if (rval > max || rval < min) {
		tst_brkm(TBROK, cleanup_fn,
			 "%s:%d: strtol(%s): %ld is out of range %ld - %ld",
			 file, lineno, str, rval, min, max);
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
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: strtoul(%s) failed", file, lineno, str);
		return rval;
	}

	if (rval > max || rval < min) {
		tst_brkm(TBROK, cleanup_fn,
			 "%s:%d: strtoul(%s): %lu is out of range %lu - %lu",
			 file, lineno, str, rval, min, max);
		return 0;
	}

	if (endptr == str || (*endptr != '\0' && *endptr != '\n')) {
		tst_brkm(TBROK, cleanup_fn,
			 "Invalid value: '%s' at %s:%d", str, file, lineno);
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
	int rval;

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

		tst_resm(TINFO, "Trying FUSE...");
		snprintf(buf, sizeof(buf), "mount.%s '%s' '%s'",
			 filesystemtype, source, target);

		rval = tst_system(buf);
		if (WIFEXITED(rval) && WEXITSTATUS(rval) == 0)
			return 0;

		tst_brkm(TBROK, cleanup_fn, "mount.%s failed with %i",
			 filesystemtype, rval);
		return -1;
	} else {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: mount(%s, %s, %s, %lu, %p) failed",
			 file, lineno, source, target, filesystemtype,
			 mountflags, data);
	}

	return -1;
}

int safe_umount(const char *file, const int lineno, void (*cleanup_fn)(void),
		const char *target)
{
	int rval;

	rval = tst_umount(target);

	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: umount(%s) failed",
			 file, lineno, target);
	}

	return rval;
}

DIR* safe_opendir(const char *file, const int lineno, void (cleanup_fn)(void),
                  const char *name)
{
	DIR *rval;

	rval = opendir(name);

	if (!rval) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
		         "%s:%d: opendir(%s) failed", file, lineno, name);
	}

	return rval;
}

int safe_closedir(const char *file, const int lineno, void (cleanup_fn)(void),
                  DIR *dirp)
{
	int rval;

	rval = closedir(dirp);

	if (rval) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
		         "%s:%d: closedir(%p) failed", file, lineno, dirp);
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
		tst_brkm(TBROK | TERRNO, cleanup_fn,
		         "%s:%d: readdir(%p) failed", file, lineno, dirp);
	}

	errno = err;
	return rval;
}

int safe_getpriority(const char *file, const int lineno, int which, id_t who)
{
	int rval, err = errno;

	errno = 0;
	rval = getpriority(which, who);
	if (errno) {
		tst_brkm(TBROK | TERRNO, NULL,
		         "%s:%d getpriority(%i, %i) failed",
			 file, lineno, which, who);
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
			tst_brkm(TCONF, NULL,
				 "%s:%d: no xattr support in fs or mounted "
				 "without user_xattr option", file, lineno);
		}

		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: getxattr(%s, %s, %p, %zu) failed",
			 file, lineno, path, name, value, size);
	}

	return rval;
}

int safe_setxattr(const char *file, const int lineno, const char *path,
		  const char *name, const void *value, size_t size, int flags)
{
	int rval;

	rval = setxattr(path, name, value, size, flags);

	if (rval) {
		if (errno == ENOTSUP) {
			tst_brkm(TCONF, NULL,
				 "%s:%d: no xattr support in fs or mounted "
				 "without user_xattr option", file, lineno);
		}

		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: setxattr(%s, %s, %p, %zu) failed",
			 file, lineno, path, name, value, size);
	}

	return rval;
}

int safe_lsetxattr(const char *file, const int lineno, const char *path,
		   const char *name, const void *value, size_t size, int flags)
{
	int rval;

	rval = lsetxattr(path, name, value, size, flags);

	if (rval) {
		if (errno == ENOTSUP) {
			tst_brkm(TCONF, NULL,
				 "%s:%d: no xattr support in fs or mounted "
				 "without user_xattr option", file, lineno);
		}

		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: lsetxattr(%s, %s, %p, %zu, %i) failed",
			 file, lineno, path, name, value, size, flags);
	}

	return rval;
}

int safe_fsetxattr(const char *file, const int lineno, int fd, const char *name,
		   const void *value, size_t size, int flags)
{
	int rval;

	rval = fsetxattr(fd, name, value, size, flags);

	if (rval) {
		if (errno == ENOTSUP) {
			tst_brkm(TCONF, NULL,
				 "%s:%d: no xattr support in fs or mounted "
				 "without user_xattr option", file, lineno);
		}

		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: fsetxattr(%i, %s, %p, %zu, %i) failed",
			 file, lineno, fd, name, value, size, flags);
	}

	return rval;
}

int safe_removexattr(const char *file, const int lineno, const char *path,
		const char *name)
{
	int rval;

	rval = removexattr(path, name);

	if (rval) {
		if (errno == ENOTSUP) {
			tst_brkm(TCONF, NULL,
				"%s:%d: no xattr support in fs or mounted "
				"without user_xattr option", file, lineno);
		}

		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: removexattr(%s, %s) failed",
			 file, lineno, path, name);
	}

	return rval;
}

int safe_lremovexattr(const char *file, const int lineno, const char *path,
		const char *name)
{
	int rval;

	rval = lremovexattr(path, name);

	if (rval) {
		if (errno == ENOTSUP) {
			tst_brkm(TCONF, NULL,
				"%s:%d: no xattr support in fs or mounted "
				"without user_xattr option", file, lineno);
		}

		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: lremovexattr(%s, %s) failed",
			 file, lineno, path, name);
	}

	return rval;
}

int safe_fremovexattr(const char *file, const int lineno, int fd,
		const char *name)
{
	int rval;

	rval = fremovexattr(fd, name);

	if (rval) {
		if (errno == ENOTSUP) {
			tst_brkm(TCONF, NULL,
				"%s:%d: no xattr support in fs or mounted "
				"without user_xattr option", file, lineno);
		}

		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: fremovexattr(%i, %s) failed",
			 file, lineno, fd, name);
	}

	return rval;
}

int safe_fsync(const char *file, const int lineno, int fd)
{
	int rval;

	rval = fsync(fd);

	if (rval) {
		tst_brkm(TBROK | TERRNO, NULL,
			"%s:%d: fsync(%i) failed", file, lineno, fd);
	}

	return rval;
}

pid_t safe_setsid(const char *file, const int lineno)
{
	pid_t rval;

	rval = setsid();
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: setsid() failed", file, lineno);
	}

	return rval;
}

int safe_mknod(const char *file, const int lineno, const char *pathname,
	mode_t mode, dev_t dev)
{
	int rval;

	rval = mknod(pathname, mode, dev);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: mknod() failed", file, lineno);
	}

	return rval;
}

int safe_mlock(const char *file, const int lineno, const void *addr,
	size_t len)
{
	int rval;

	rval = mlock(addr, len);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: mlock() failed", file, lineno);
	}

	return rval;
}

int safe_munlock(const char *file, const int lineno, const void *addr,
	size_t len)
{
	int rval;

	rval = munlock(addr, len);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: munlock() failed", file, lineno);
	}

	return rval;
}

int safe_mincore(const char *file, const int lineno, void *start,
	size_t length, unsigned char *vec)
{
	int rval;

	rval = mincore(start, length, vec);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: mincore() failed", file, lineno);
	}

	return rval;
}
