/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2010-2018 Linux Test Project
 * Copyright (c) 2011-2015 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_SAFE_MACROS_H__
#define TST_SAFE_MACROS_H__

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>
#include <grp.h>

#include "safe_stdio_fn.h"
#include "safe_macros_fn.h"
#include "tst_cmd.h"

int safe_access(const char *filename, const int lineno, const char *pathname,
		   int mode);
#define SAFE_ACCESS(path, mode) \
	safe_access(__FILE__, __LINE__, (path), (mode))

#define SAFE_BASENAME(path) \
	safe_basename(__FILE__, __LINE__, NULL, (path))

#define SAFE_CHDIR(path) \
	safe_chdir(__FILE__, __LINE__, NULL, (path))

#define SAFE_CLOSE(fd) do { \
		safe_close(__FILE__, __LINE__, NULL, (fd)); \
		fd = -1; \
	} while (0)

#define SAFE_CREAT(pathname, mode) \
	safe_creat(__FILE__, __LINE__, NULL, (pathname), (mode))

#define SAFE_CHROOT(path) \
	safe_chroot(__FILE__, __LINE__, (path))
int safe_chroot(const char *file, const int lineno, const char *path);

#define SAFE_DIRNAME(path) \
	safe_dirname(__FILE__, __LINE__, NULL, (path))

int safe_dup(const char *file, const int lineno, int oldfd);

#define SAFE_DUP(oldfd) \
	safe_dup(__FILE__, __LINE__, (oldfd))

int safe_dup2(const char *file, const int lineno, int oldfd, int newfd);

#define SAFE_DUP2(oldfd, newfd)			\
	safe_dup2(__FILE__, __LINE__, (oldfd), (newfd))

#define SAFE_GETCWD(buf, size) \
	safe_getcwd(__FILE__, __LINE__, NULL, (buf), (size))

#define SAFE_GETPWNAM(name) \
	safe_getpwnam(__FILE__, __LINE__, NULL, (name))

#define SAFE_GETRUSAGE(who, usage) \
	safe_getrusage(__FILE__, __LINE__, NULL, (who), (usage))

#define SAFE_MALLOC(size) \
	safe_malloc(__FILE__, __LINE__, NULL, (size))

void *safe_realloc(const char *file, const int lineno, void *ptr, size_t size);

#define SAFE_REALLOC(ptr, size) \
	safe_realloc(__FILE__, __LINE__, (ptr), (size))

#define SAFE_MKDIR(pathname, mode) \
	safe_mkdir(__FILE__, __LINE__, NULL, (pathname), (mode))

#define SAFE_RMDIR(pathname) \
	safe_rmdir(__FILE__, __LINE__, NULL, (pathname))

#define SAFE_MUNMAP(addr, length) \
	safe_munmap(__FILE__, __LINE__, NULL, (addr), (length))

int safe_msync(const char *file, const int lineno, void *addr,
				size_t length, int flags);

#define SAFE_MSYNC(addr, length, flags) \
	safe_msync(__FILE__, __LINE__, (addr), (length), (flags))

#define SAFE_OPEN(pathname, oflags, ...) \
	safe_open(__FILE__, __LINE__, NULL, (pathname), (oflags), \
	    ##__VA_ARGS__)

#define SAFE_PIPE(fildes) \
	safe_pipe(__FILE__, __LINE__, NULL, (fildes))

int safe_pipe2(const char *file, const int lineno, int fildes[2], int flags);

#define SAFE_PIPE2(fildes, flags) \
	safe_pipe2(__FILE__, __LINE__, (fildes), (flags))

#define SAFE_READ(len_strict, fildes, buf, nbyte) \
	safe_read(__FILE__, __LINE__, NULL, (len_strict), (fildes), (buf), (nbyte))

#define SAFE_SETEGID(egid) \
	safe_setegid(__FILE__, __LINE__, NULL, (egid))

#define SAFE_SETEUID(euid) \
	safe_seteuid(__FILE__, __LINE__, NULL, (euid))

#define SAFE_SETGID(gid) \
	safe_setgid(__FILE__, __LINE__, NULL, (gid))

#define SAFE_SETUID(uid) \
	safe_setuid(__FILE__, __LINE__, NULL, (uid))

int safe_setregid(const char *file, const int lineno,
		  gid_t rgid, gid_t egid);

#define SAFE_SETREGID(rgid, egid) \
	safe_setregid(__FILE__, __LINE__, (rgid), (egid))

int safe_setreuid(const char *file, const int lineno,
		  uid_t ruid, uid_t euid);

#define SAFE_SETREUID(ruid, euid) \
	safe_setreuid(__FILE__, __LINE__, (ruid), (euid))

int safe_setresgid(const char *file, const int lineno,
	gid_t rgid, gid_t egid, gid_t sgid);
#define SAFE_SETRESGID(rgid, egid, sgid) \
	safe_setresgid(__FILE__, __LINE__, (rgid), (egid), (sgid))

int safe_setresuid(const char *file, const int lineno,
		  uid_t ruid, uid_t euid, uid_t suid);
#define SAFE_SETRESUID(ruid, euid, suid) \
	safe_setresuid(__FILE__, __LINE__, (ruid), (euid), (suid))

#define SAFE_GETRESUID(ruid, euid, suid) \
	safe_getresuid(__FILE__, __LINE__, NULL, (ruid), (euid), (suid))

#define SAFE_GETRESGID(rgid, egid, sgid) \
	safe_getresgid(__FILE__, __LINE__, NULL, (rgid), (egid), (sgid))

int safe_setpgid(const char *file, const int lineno, pid_t pid, pid_t pgid);

#define SAFE_SETPGID(pid, pgid) \
	safe_setpgid(__FILE__, __LINE__, (pid), (pgid))

pid_t safe_getpgid(const char *file, const int lineno, pid_t pid);

#define SAFE_GETPGID(pid) \
	safe_getpgid(__FILE__, __LINE__, (pid))

int safe_setgroups(const char *file, const int lineno, size_t size, const gid_t *list);

#define SAFE_SETGROUPS(size, list) \
	safe_setgroups(__FILE__, __LINE__, (size), (list))

int safe_getgroups(const char *file, const int lineno, int size, gid_t list[]);

#define SAFE_GETGROUPS(size, list) \
	safe_getgroups(__FILE__, __LINE__, (size), (list))

#define SAFE_UNLINK(pathname) \
	safe_unlink(__FILE__, __LINE__, NULL, (pathname))

#define SAFE_LINK(oldpath, newpath) \
	safe_link(__FILE__, __LINE__, NULL, (oldpath), (newpath))

#define SAFE_LINKAT(olddirfd, oldpath, newdirfd, newpath, flags) \
	safe_linkat(__FILE__, __LINE__, NULL, (olddirfd), (oldpath), \
		    (newdirfd), (newpath), (flags))

#define SAFE_READLINK(path, buf, bufsize) \
	safe_readlink(__FILE__, __LINE__, NULL, (path), (buf), (bufsize))

#define SAFE_SYMLINK(oldpath, newpath) \
	safe_symlink(__FILE__, __LINE__, NULL, (oldpath), (newpath))

#define SAFE_WRITE(len_strict, fildes, buf, nbyte) \
	safe_write(__FILE__, __LINE__, NULL, (len_strict), (fildes), (buf), (nbyte))

#define SAFE_STRTOL(str, min, max) \
	safe_strtol(__FILE__, __LINE__, NULL, (str), (min), (max))

#define SAFE_STRTOUL(str, min, max) \
	safe_strtoul(__FILE__, __LINE__, NULL, (str), (min), (max))

#define SAFE_STRTOF(str, min, max) \
	safe_strtof(__FILE__, __LINE__, NULL, (str), (min), (max))

#define SAFE_SYSCONF(name) \
	safe_sysconf(__FILE__, __LINE__, NULL, name)

#define SAFE_CHMOD(path, mode) \
	safe_chmod(__FILE__, __LINE__, NULL, (path), (mode))

#define SAFE_FCHMOD(fd, mode) \
	safe_fchmod(__FILE__, __LINE__, NULL, (fd), (mode))

#define SAFE_CHOWN(path, owner, group) \
	safe_chown(__FILE__, __LINE__, NULL, (path), (owner), (group))

#define SAFE_FCHOWN(fd, owner, group) \
	safe_fchown(__FILE__, __LINE__, NULL, (fd), (owner), (group))

#define SAFE_WAIT(status) \
	safe_wait(__FILE__, __LINE__, NULL, (status))

#define SAFE_WAITPID(pid, status, opts) \
	safe_waitpid(__FILE__, __LINE__, NULL, (pid), (status), (opts))

#define SAFE_KILL(pid, sig) \
	safe_kill(__FILE__, __LINE__, NULL, (pid), (sig))

#define SAFE_MEMALIGN(alignment, size) \
	safe_memalign(__FILE__, __LINE__, NULL, (alignment), (size))

#define SAFE_MKFIFO(pathname, mode) \
	safe_mkfifo(__FILE__, __LINE__, NULL, (pathname), (mode))

#define SAFE_RENAME(oldpath, newpath) \
	safe_rename(__FILE__, __LINE__, NULL, (oldpath), (newpath))

#define SAFE_MOUNT(source, target, filesystemtype, \
		   mountflags, data) \
	safe_mount(__FILE__, __LINE__, NULL, (source), (target), \
		   (filesystemtype), (mountflags), (data))

#define SAFE_UMOUNT(target) \
	safe_umount(__FILE__, __LINE__, NULL, (target))

#define SAFE_OPENDIR(name) \
	safe_opendir(__FILE__, __LINE__, NULL, (name))

#define SAFE_CLOSEDIR(dirp) \
	safe_closedir(__FILE__, __LINE__, NULL, (dirp))

#define SAFE_READDIR(dirp) \
	safe_readdir(__FILE__, __LINE__, NULL, (dirp))

#define SAFE_IOCTL_(file, lineno, fd, request, ...)          \
	({int tst_ret_ = ioctl(fd, request, ##__VA_ARGS__);  \
	  tst_ret_ < 0 ?                                     \
	   tst_brk_((file), (lineno), TBROK | TERRNO,        \
	            "ioctl(%i,%s,...) failed", fd, #request), 0 \
	 : tst_ret_;})

#define SAFE_IOCTL(fd, request, ...) \
	SAFE_IOCTL_(__FILE__, __LINE__, (fd), (request), ##__VA_ARGS__)

#define SAFE_FCNTL(fd, cmd, ...)                            \
	({int tst_ret_ = fcntl(fd, cmd, ##__VA_ARGS__);     \
	  tst_ret_ == -1 ?                                  \
	   tst_brk(TBROK | TERRNO,                          \
	            "fcntl(%i,%s,...) failed", fd, #cmd), 0 \
	 : tst_ret_;})

/*
 * following functions are inline because the behaviour may depend on
 * -D_FILE_OFFSET_BITS=64 compile flag
 */

static inline void *safe_mmap(const char *file, const int lineno,
                              void *addr, size_t length,
                              int prot, int flags, int fd, off_t offset)
{
	void *rval;

	rval = mmap(addr, length, prot, flags, fd, offset);
	if (rval == MAP_FAILED) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"mmap(%p,%zu,%d,%d,%d,%ld) failed",
			addr, length, prot, flags, fd, (long) offset);
	}

	return rval;
}
#define SAFE_MMAP(addr, length, prot, flags, fd, offset) \
	safe_mmap(__FILE__, __LINE__, (addr), (length), (prot), \
	(flags), (fd), (offset))

static inline int safe_ftruncate(const char *file, const int lineno,
                                 int fd, off_t length)
{
	int rval;

	rval = ftruncate(fd, length);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"ftruncate(%d,%ld) failed", fd, (long)length);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid ftruncate(%d,%ld) return value %d", fd,
			(long)length, rval);
	}

	return rval;
}
#define SAFE_FTRUNCATE(fd, length) \
	safe_ftruncate(__FILE__, __LINE__, (fd), (length))

static inline int safe_posix_fadvise(const char *file, const int lineno,
                                int fd, off_t offset, off_t len, int advice)
{
	int rval;

	rval = posix_fadvise(fd, offset, len, advice);

	if (rval)
		tst_brk_(file, lineno, TBROK,
			"posix_fadvise(%d,%ld,%ld,%d) failed: %s",
			fd, (long)offset, (long)len, advice, tst_strerrno(rval));

	return rval;
}
#define SAFE_POSIX_FADVISE(fd, offset, len, advice) \
	safe_posix_fadvise(__FILE__, __LINE__, (fd), (offset), (len), (advice))

static inline int safe_truncate(const char *file, const int lineno,
                                const char *path, off_t length)
{
	int rval;

	rval = truncate(path, length);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"truncate(%s,%ld) failed", path, (long)length);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid truncate(%s,%ld) return value %d", path,
			(long)length, rval);
	}

	return rval;
}
#define SAFE_TRUNCATE(path, length) \
	safe_truncate(__FILE__, __LINE__, (path), (length))

static inline int safe_stat(const char *file, const int lineno,
                            const char *path, struct stat *buf)
{
	int rval;

	rval = stat(path, buf);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"stat(%s,%p) failed", path, buf);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid stat(%s,%p) return value %d", path, buf,
			rval);
	}

	return rval;
}
#define SAFE_STAT(path, buf) \
	safe_stat(__FILE__, __LINE__, (path), (buf))

static inline int safe_fstat(const char *file, const int lineno,
                             int fd, struct stat *buf)
{
	int rval;

	rval = fstat(fd, buf);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"fstat(%d,%p) failed", fd, buf);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid fstat(%d,%p) return value %d", fd, buf, rval);
	}

	return rval;
}
#define SAFE_FSTAT(fd, buf) \
	safe_fstat(__FILE__, __LINE__, (fd), (buf))

static inline int safe_lstat(const char *file, const int lineno,
	const char *path, struct stat *buf)
{
	int rval;

	rval = lstat(path, buf);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"lstat(%s,%p) failed", path, buf);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid lstat(%s,%p) return value %d", path, buf,
			rval);
	}

	return rval;
}
#define SAFE_LSTAT(path, buf) \
	safe_lstat(__FILE__, __LINE__, (path), (buf))

static inline int safe_statfs(const char *file, const int lineno,
                              const char *path, struct statfs *buf)
{
	int rval;

	rval = statfs(path, buf);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"statfs(%s,%p) failed", path, buf);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid statfs(%s,%p) return value %d", path, buf,
			rval);
	}

	return rval;
}
#define SAFE_STATFS(path, buf) \
	safe_statfs(__FILE__, __LINE__, (path), (buf))

static inline off_t safe_lseek(const char *file, const int lineno,
                               int fd, off_t offset, int whence)
{
	off_t rval;

	rval = lseek(fd, offset, whence);

	if (rval == (off_t) -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"lseek(%d,%ld,%d) failed", fd, (long)offset, whence);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid lseek(%d,%ld,%d) return value %ld", fd,
			(long)offset, whence, (long)rval);
	}

	return rval;
}
#define SAFE_LSEEK(fd, offset, whence) \
	safe_lseek(__FILE__, __LINE__, (fd), (offset), (whence))

static inline int safe_getrlimit(const char *file, const int lineno,
                                 int resource, struct rlimit *rlim)
{
	int rval;

	rval = getrlimit(resource, rlim);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"getrlimit(%d,%p) failed", resource, rlim);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid getrlimit(%d,%p) return value %d", resource,
			rlim, rval);
	}

	return rval;
}
#define SAFE_GETRLIMIT(resource, rlim) \
	safe_getrlimit(__FILE__, __LINE__, (resource), (rlim))

static inline int safe_setrlimit(const char *file, const int lineno,
                                 int resource, const struct rlimit *rlim)
{
	int rval;

	rval = setrlimit(resource, rlim);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"setrlimit(%d,%p) failed", resource, rlim);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid setrlimit(%d,%p) return value %d", resource,
			rlim, rval);
	}

	return rval;
}
#define SAFE_SETRLIMIT(resource, rlim) \
	safe_setrlimit(__FILE__, __LINE__, (resource), (rlim))

typedef void (*sighandler_t)(int);
sighandler_t safe_signal(const char *file, const int lineno,
	int signum, sighandler_t handler);

#define SAFE_SIGNAL(signum, handler) \
	safe_signal(__FILE__, __LINE__, (signum), (handler))

int safe_sigaction(const char *file, const int lineno,
                   int signum, const struct sigaction *act,
                   struct sigaction *oldact);
#define SAFE_SIGACTION(signum, act, oldact) \
	safe_sigaction(__FILE__, __LINE__, (signum), (act), (oldact))

int safe_sigaddset(const char *file, const int lineno,
                    sigset_t *sigs, int signo);
#define SAFE_SIGADDSET(sigs, signo) \
	safe_sigaddset(__FILE__, __LINE__, (sigs), (signo))

int safe_sigdelset(const char *file, const int lineno,
                    sigset_t *sigs, int signo);
#define SAFE_SIGDELSET(sigs, signo) \
	safe_sigdelset(__FILE__, __LINE__, (sigs), (signo))

int safe_sigemptyset(const char *file, const int lineno,
                      sigset_t *sigs);
#define SAFE_SIGEMPTYSET(sigs) \
	safe_sigemptyset(__FILE__, __LINE__, (sigs))

int safe_sigfillset(const char *file, const int lineno,
		     sigset_t *sigs);
#define SAFE_SIGFILLSET(sigs) \
	safe_sigfillset(__FILE__, __LINE__, (sigs))

int safe_sigprocmask(const char *file, const int lineno,
                      int how, sigset_t *set, sigset_t *oldset);
#define SAFE_SIGPROCMASK(how, set, oldset) \
	safe_sigprocmask(__FILE__, __LINE__, (how), (set), (oldset))

int safe_sigwait(const char *file, const int lineno,
                  sigset_t *set, int *sig);
#define SAFE_SIGWAIT(set, sig) \
	safe_sigwait(__FILE__, __LINE__, (set), (sig))

#define SAFE_EXECLP(file, arg, ...) do {                   \
	execlp((file), (arg), ##__VA_ARGS__);              \
	tst_brk_(__FILE__, __LINE__, TBROK | TERRNO,       \
	         "execlp(%s, %s, ...) failed", file, arg); \
	} while (0)

#define SAFE_EXECL(file, arg, ...) do {				\
       execl((file), (arg), ##__VA_ARGS__);			\
       tst_brk_(__FILE__, __LINE__, TBROK | TERRNO,		\
                "execl(%s, %s, ...) failed", file, arg); 	\
       } while (0)

#define SAFE_EXECVP(file, arg) do {                   \
	execvp((file), (arg));              \
	tst_brk_(__FILE__, __LINE__, TBROK | TERRNO,       \
	         "execvp(%s, %p) failed", file, arg); \
	} while (0)

int safe_getpriority(const char *file, const int lineno, int which, id_t who);
#define SAFE_GETPRIORITY(which, who) \
	safe_getpriority(__FILE__, __LINE__, (which), (who))

struct group *safe_getgrnam(const char *file, const int lineno,
			    const char *name);
#define SAFE_GETGRNAM(name) \
	safe_getgrnam(__FILE__, __LINE__, (name))

struct group *safe_getgrnam_fallback(const char *file, const int lineno,
		const char *name, const char *fallback);
#define SAFE_GETGRNAM_FALLBACK(name, fallback) \
	safe_getgrnam_fallback(__FILE__, __LINE__, (name), (fallback))

struct group *safe_getgrgid(const char *file, const int lineno, gid_t gid);
#define SAFE_GETGRGID(gid) \
	safe_getgrgid(__FILE__, __LINE__, (gid))

ssize_t safe_getxattr(const char *file, const int lineno, const char *path,
	const char *name, void *value, size_t size);
#define SAFE_GETXATTR(path, name, value, size) \
	safe_getxattr(__FILE__, __LINE__, (path), (name), (value), (size))

int safe_setxattr(const char *file, const int lineno, const char *path,
            const char *name, const void *value, size_t size, int flags);
#define SAFE_SETXATTR(path, name, value, size, flags) \
	safe_setxattr(__FILE__, __LINE__, (path), (name), (value), (size), (flags))

int safe_lsetxattr(const char *file, const int lineno, const char *path,
            const char *name, const void *value, size_t size, int flags);
#define SAFE_LSETXATTR(path, name, value, size, flags) \
	safe_lsetxattr(__FILE__, __LINE__, (path), (name), (value), (size), (flags))

int safe_fsetxattr(const char *file, const int lineno, int fd, const char *name,
            const void *value, size_t size, int flags);
#define SAFE_FSETXATTR(fd, name, value, size, flags) \
	safe_fsetxattr(__FILE__, __LINE__, (fd), (name), (value), (size), (flags))

int safe_removexattr(const char *file, const int lineno, const char *path,
		const char *name);
#define SAFE_REMOVEXATTR(path, name) \
	safe_removexattr(__FILE__, __LINE__, (path), (name))

int safe_lremovexattr(const char *file, const int lineno, const char *path,
		const char *name);
#define SAFE_LREMOVEXATTR(path, name) \
	safe_lremovexattr(__FILE__, __LINE__, (path), (name))

int safe_fremovexattr(const char *file, const int lineno, int fd,
		const char *name);
#define SAFE_FREMOVEXATTR(fd, name) \
	safe_fremovexattr(__FILE__, __LINE__, (fd), (name))

int safe_fsync(const char *file, const int lineno, int fd);
#define SAFE_FSYNC(fd) safe_fsync(__FILE__, __LINE__, (fd))

int safe_setsid(const char *file, const int lineno);
#define SAFE_SETSID() safe_setsid(__FILE__, __LINE__)

int safe_mknod(const char *file, const int lineno, const char *pathname,
	mode_t mode, dev_t dev);
#define SAFE_MKNOD(pathname, mode, dev) \
	safe_mknod(__FILE__, __LINE__, (pathname), (mode), (dev))

int safe_mlock(const char *file, const int lineno, const char *addr,
	size_t len);
#define SAFE_MLOCK(addr, len) safe_mlock(__FILE__, __LINE__, (addr), (len))

int safe_munlock(const char *file, const int lineno, const char *addr,
	size_t len);
#define SAFE_MUNLOCK(addr, len) safe_munlock(__FILE__, __LINE__, (addr), (len))

int safe_mincore(const char *file, const int lineno, void *start,
	size_t length, unsigned char *vec);
#define SAFE_MINCORE(start, length, vec) \
	safe_mincore(__FILE__, __LINE__, (start), (length), (vec))

int safe_personality(const char *filename, unsigned int lineno,
		    unsigned long persona);
#define SAFE_PERSONALITY(persona) safe_personality(__FILE__, __LINE__, persona)

int safe_pidfd_open(const char *filename, const int lineno, pid_t pid,
		   unsigned int flags);
#define SAFE_PIDFD_OPEN(pid, flags) \
	safe_pidfd_open(__FILE__, __LINE__, (pid), (flags))

#define SAFE_SETENV(name, value, overwrite) do {		\
	if (setenv(name, value, overwrite)) {			\
		tst_brk_(__FILE__, __LINE__, TBROK | TERRNO,	\
			"setenv(%s, %s, %d) failed",		\
			name, value, overwrite);		\
	}							\
	} while (0)

int safe_unshare(const char *file, const int lineno, int flags);
#define SAFE_UNSHARE(flags) safe_unshare(__FILE__, __LINE__, (flags))

int safe_setns(const char *file, const int lineno, int fd, int nstype);
#define SAFE_SETNS(fd, nstype) safe_setns(__FILE__, __LINE__, (fd), (nstype))

/*
 * SAFE_CMD() is a wrapper for tst_cmd(). It runs a command passed via argv[]
 * and handles non-zero exit (exits with 'TBROK') and 'ENOENT' (the program not
 * in '$PATH', exits with 'TCONF').
 *
 * @param argv[] a 'NULL' terminated array of strings starting with the program
 * name which is followed by optional arguments.
 * @param stdout_path: path where to redirect stdout. Set NULL if redirection is
 * not needed.
 * @param stderr_path: path where to redirect stderr. Set NULL if redirection is
 * not needed.
 */
void safe_cmd(const char *file, const int lineno, const char *const argv[],
	const char *stdout_path, const char *stderr_path);
#define SAFE_CMD(argv, stdout_path, stderr_path) \
	safe_cmd(__FILE__, __LINE__, (argv), (stdout_path), (stderr_path))
/*
 * SAFE_PTRACE() treats any non-zero return value as error. Don't use it
 * for requests like PTRACE_PEEK* or PTRACE_SECCOMP_GET_FILTER which use
 * the return value to pass arbitrary data.
 */
long tst_safe_ptrace(const char *file, const int lineno, int req, pid_t pid,
	void *addr, void *data);
#define SAFE_PTRACE(req, pid, addr, data) \
	tst_safe_ptrace(__FILE__, __LINE__, req, pid, addr, data)

int safe_sysinfo(const char *file, const int lineno, struct sysinfo *info);
#define SAFE_SYSINFO(info) \
	safe_sysinfo(__FILE__, __LINE__, (info))

void safe_print_file(const char *file, const int lineno, char *path);

#endif /* SAFE_MACROS_H__ */
