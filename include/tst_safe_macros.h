/*
 * Copyright (c) 2010-2015 Linux Test Project
 * Copyright (c) 2011-2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TST_SAFE_MACROS_H__
#define TST_SAFE_MACROS_H__

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>

#include "safe_macros_fn.h"

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

#define SAFE_DIRNAME(path) \
	safe_dirname(__FILE__, __LINE__, NULL, (path))

static inline int safe_dup(const char *file, const int lineno,
			   int oldfd)
{
	int rval;

	rval = dup(oldfd);
	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "dup(%i) failed", oldfd);
	}

	return rval;
}
#define SAFE_DUP(oldfd) \
	safe_dup(__FILE__, __LINE__, (oldfd))

#define	SAFE_GETCWD(buf, size) \
	safe_getcwd(__FILE__, __LINE__, NULL, (buf), (size))

#define SAFE_GETPWNAM(name) \
	safe_getpwnam(__FILE__, __LINE__, NULL, (name))

#define SAFE_GETRUSAGE(who, usage) \
	safe_getrusage(__FILE__, __LINE__, NULL, (who), (usage))

#define SAFE_MALLOC(size) \
	safe_malloc(__FILE__, __LINE__, NULL, (size))

#define SAFE_MKDIR(pathname, mode) \
	safe_mkdir(__FILE__, __LINE__, NULL, (pathname), (mode))

#define SAFE_RMDIR(pathname) \
	safe_rmdir(__FILE__, __LINE__, NULL, (pathname))

#define SAFE_MUNMAP(addr, length) \
	safe_munmap(__FILE__, __LINE__, NULL, (addr), (length))

#define SAFE_OPEN(pathname, oflags, ...) \
	safe_open(__FILE__, __LINE__, NULL, (pathname), (oflags), \
	    ##__VA_ARGS__)

#define SAFE_PIPE(fildes) \
	safe_pipe(__FILE__, __LINE__, NULL, (fildes))

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

#define SAFE_GETRESUID(ruid, euid, suid) \
	safe_getresuid(__FILE__, __LINE__, NULL, (ruid), (euid), (suid))

#define SAFE_GETRESGID(rgid, egid, sgid) \
	safe_getresgid(__FILE__, __LINE__, NULL, (rgid), (egid), (sgid))

int safe_setpgid(const char *file, const int lineno, pid_t pid, pid_t pgid);

#define SAFE_SETPGID(pid, pgid) \
	safe_setpgid(__FILE__, __LINE__, (pid), (pgid));

pid_t safe_getpgid(const char *file, const int lineno, pid_t pid);

#define SAFE_GETPGID(pid) \
	safe_getpgid(__FILE__, __LINE__, (pid))

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

#define SAFE_IOCTL(fd, request, ...)                         \
	({int ret = ioctl(fd, request, ##__VA_ARGS__);       \
	  ret < 0 ?                                          \
	   tst_brk(TBROK | TERRNO,                           \
	            "ioctl(%i,%s,...) failed", fd, #request) \
	 : ret;})

#define SAFE_FCNTL(fd, cmd, ...)                            \
	({int ret = fcntl(fd, cmd, ##__VA_ARGS__);          \
	  ret == -1 ?                                       \
	   tst_brk(TBROK | TERRNO,                          \
	            "fcntl(%i,%s,...) failed", fd, #cmd), 0 \
	 : ret;})

/*
 * following functions are inline because the behaviour may depend on
 * -D_FILE_OFFSET_BITS=64 -DOFF_T=off64_t compile flags
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
			 "ftruncate(%d,%ld) failed",
			 fd, (long)length);
	}

	return rval;
}
#define SAFE_FTRUNCATE(fd, length) \
	safe_ftruncate(__FILE__, __LINE__, (fd), (length))

static inline int safe_truncate(const char *file, const int lineno,
                                const char *path, off_t length)
{
	int rval;

	rval = truncate(path, length);
	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "truncate(%s,%ld) failed",
			 path, (long)length);
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
	}

	return rval;
}
#define SAFE_LSTAT(path, buf) \
	safe_lstat(__FILE__, __LINE__, (path), (buf))

static inline off_t safe_lseek(const char *file, const int lineno,
                               int fd, off_t offset, int whence)
{
	off_t rval;

	rval = lseek(fd, offset, whence);

	if (rval == (off_t) -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"lseek(%d,%ld,%d) failed",
			fd, (long)offset, whence);
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
			"getrlimit(%d,%p) failed",
			resource, rlim);
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
			 "setrlimit(%d,%p) failed",
			 resource, rlim);
	}

	return rval;
}
#define SAFE_SETRLIMIT(resource, rlim) \
	safe_setrlimit(__FILE__, __LINE__, (resource), (rlim))

typedef void (*sighandler_t)(int);
static inline sighandler_t safe_signal(const char *file, const int lineno,
				       int signum, sighandler_t handler)
{
	sighandler_t rval;

	rval = signal(signum, handler);

	if (rval == SIG_ERR) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"signal(%d,%p) failed",
			signum, handler);
	}

	return rval;
}

#define SAFE_SIGNAL(signum, handler) \
	safe_signal(__FILE__, __LINE__, (signum), (handler))

#define SAFE_EXECLP(file, arg, ...) do {                   \
	execlp((file), (arg), ##__VA_ARGS__);              \
	tst_brk_(__FILE__, __LINE__, TBROK | TERRNO,       \
	         "execlp(%s, %s, ...) failed", file, arg); \
	} while (0)

int safe_getpriority(const char *file, const int lineno, int which, id_t who);
#define SAFE_GETPRIORITY(which, who) \
	safe_getpriority(__FILE__, __LINE__, (which), (who))

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

int safe_fsync(const char *file, const int lineno, int fd);
#define SAFE_FSYNC(fd) safe_fsync(__FILE__, __LINE__, (fd))

int safe_setsid(const char *file, const int lineno);
#define SAFE_SETSID() safe_setsid(__FILE__, __LINE__)

int safe_mknod(const char *file, const int lineno, const char *pathname,
	mode_t mode, dev_t dev);
#define SAFE_MKNOD(pathname, mode, dev) \
	safe_mknod(__FILE__, __LINE__, (pathname), (mode), (dev))

int safe_fanotify_init(const char *file, const int lineno,
	unsigned int flags, unsigned int event_f_flags);
#define SAFE_FANOTIFY_INIT(fan, mode)  \
	safe_fanotify_init(__FILE__, __LINE__, (fan), (mode))

int safe_personality(const char *filename, unsigned int lineno,
		    unsigned long persona);
#define SAFE_PERSONALITY(persona) safe_personality(__FILE__, __LINE__, persona)


#endif /* SAFE_MACROS_H__ */
