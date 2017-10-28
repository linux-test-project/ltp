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

#ifndef SAFE_MACROS_FN_H__
#define SAFE_MACROS_FN_H__

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>

char* safe_basename(const char *file, const int lineno,
                    void (*cleanup_fn)(void), char *path);

int safe_chdir(const char *file, const int lineno,
               void (*cleanup_fn)(void), const char *path);

int safe_close(const char *file, const int lineno,
               void (*cleanup_fn)(void), int fildes);

int safe_creat(const char *file, const int lineno,
               void (*cleanup_fn)(void), const char *pathname, mode_t mode);

char* safe_dirname(const char *file, const int lineno,
                   void (*cleanup_fn)(void), char *path);

char* safe_getcwd(const char *file, const int lineno,
                  void (*cleanup_fn)(void), char *buf, size_t size);

struct passwd* safe_getpwnam(const char *file, const int lineno,
                             void (*cleanup_fn)(void), const char *name);

int safe_getrusage(const char *file, const int lineno,
                   void (*cleanup_fn)(void), int who, struct rusage *usage);

void* safe_malloc(const char *file, const int lineno,
                  void (*cleanup_fn)(void), size_t size);

int safe_mkdir(const char *file, const int lineno,
               void (*cleanup_fn)(void), const char *pathname, mode_t mode);

int safe_rmdir(const char *file, const int lineno,
               void (*cleanup_fn)(void), const char *pathname);


int safe_munmap(const char *file, const int lineno,
                void (*cleanup_fn)(void), void *addr, size_t length);

int safe_open(const char *file, const int lineno,
              void (*cleanup_fn)(void), const char *pathname, int oflags, ...);

int safe_pipe(const char *file, const int lineno,
              void (*cleanup_fn)(void), int fildes[2]);

ssize_t safe_read(const char *file, const int lineno,
                  void (*cleanup_fn)(void), char len_strict, int fildes,
                  void *buf, size_t nbyte);

int safe_setegid(const char *file, const int lineno,
                 void (*cleanup_fn)(void), gid_t egid);

int safe_seteuid(const char *file, const int lineno,
                 void (*cleanup_fn)(void), uid_t euid);

int safe_setgid(const char *file, const int lineno,
                void (*cleanup_fn)(void), gid_t gid);

int safe_setuid(const char *file, const int lineno,
                void (*cleanup_fn)(void), uid_t uid);

int safe_getresuid(const char *file, const int lineno,
                   void (*cleanup_fn)(void),
                   uid_t *ruid, uid_t *euid, uid_t *suid);

int safe_getresgid(const char *file, const int lineno,
                   void (*cleanup_fn)(void),
                   gid_t *rgid, gid_t *egid, gid_t *sgid);

int safe_unlink(const char *file, const int lineno,
                void (*cleanup_fn)(void), const char *pathname);

int safe_link(const char *file, const int lineno,
              void (cleanup_fn)(void), const char *oldpath,
              const char *newpath);

int safe_linkat(const char *file, const int lineno,
		void (cleanup_fn)(void), int olddirfd, const char *oldpath,
		int newdirfd, const char *newpath, int flags);

ssize_t safe_readlink(const char *file, const int lineno,
		  void (cleanup_fn)(void), const char *path,
		  char *buf, size_t bufsize);

int safe_symlink(const char *file, const int lineno,
                 void (cleanup_fn)(void), const char *oldpath,
                 const char *newpath);

ssize_t safe_write(const char *file, const int lineno,
                   void (cleanup_fn)(void), char len_strict, int fildes,
                   const void *buf, size_t nbyte);

long safe_strtol(const char *file, const int lineno,
                 void (cleanup_fn)(void), char *str, long min, long max);

unsigned long safe_strtoul(const char *file, const int lineno,
                           void (cleanup_fn)(void),
                           char *str, unsigned long min, unsigned long max);

long safe_sysconf(const char *file, const int lineno,
		  void (cleanup_fn)(void), int name);

int safe_chmod(const char *file, const int lineno, void (cleanup_fn)(void),
	       const char *path, mode_t mode);

int safe_fchmod(const char *file, const int lineno, void (cleanup_fn)(void),
	        int fd, mode_t mode);

int safe_chown(const char *file, const int lineno, void (cleanup_fn)(void),
               const char *path, uid_t owner, gid_t group);

int safe_fchown(const char *file, const int lineno, void (cleanup_fn)(void),
                int fd, uid_t owner, gid_t group);

pid_t safe_wait(const char *file, const int lineno, void (cleanup_fn)(void),
                int *status);

pid_t safe_waitpid(const char *file, const int lineno, void (cleanup_fn)(void),
                   pid_t pid, int *status, int opts);

int safe_kill(const char *file, const int lineno, void (cleanup_fn)(void),
              pid_t pid, int sig);

void *safe_memalign(const char *file, const int lineno,
		    void (*cleanup_fn)(void), size_t alignment, size_t size);

int safe_mkfifo(const char *file, const int lineno,
		void (*cleanup_fn)(void), const char *pathname, mode_t mode);

int safe_rename(const char *file, const int lineno, void (*cleanup_fn)(void),
		const char *oldpath, const char *newpath);

int safe_mount(const char *file, const int lineno, void (*cleanup_fn)(void),
	       const char *source, const char *target,
	       const char *filesystemtype, unsigned long mountflags,
	       const void *data);

int safe_umount(const char *file, const int lineno, void (*cleanup_fn)(void),
		const char *target);

DIR* safe_opendir(const char *file, const int lineno, void (cleanup_fn)(void),
                  const char *name);

int safe_closedir(const char *file, const int lineno, void (cleanup_fn)(void),
                  DIR *dirp);

struct dirent *safe_readdir(const char *file, const int lineno,
                            void (cleanup_fn)(void),
                            DIR *dirp);

DIR* safe_opendir(const char *file, const int lineno,
                  void (cleanup_fn)(void),
                  const char *name);

struct dirent *safe_readdir(const char *file, const int lineno,
                            void (cleanup_fn)(void),
                            DIR *dirp);

int safe_closedir(const char *file, const int lineno,
                  void (cleanup_fn)(void),
                  DIR *dirp);

#endif /* SAFE_MACROS_FN_H__ */
