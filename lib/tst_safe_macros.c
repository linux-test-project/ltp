// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2017-2024 Linux Test Project
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include <sys/ptrace.h>
#include "config.h"
#ifdef HAVE_SYS_FANOTIFY_H
# include <sys/fanotify.h>
#endif
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "lapi/setns.h"
#include "tst_safe_macros.h"
#include "lapi/personality.h"
#include "lapi/pidfd.h"

int safe_access(const char *file, const int lineno,
	    const char *pathname, int mode)
{
	int rval;

	rval = access(pathname, mode);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"access(%s,%d) failed", pathname, mode);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid access(%s,%d) return value %d", pathname,
			mode, rval);
	}

	return rval;
}

int safe_setpgid(const char *file, const int lineno, pid_t pid, pid_t pgid)
{
	int rval;

	rval = setpgid(pid, pgid);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"setpgid(%i, %i) failed", pid, pgid);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid setpgid(%i, %i) return value %d", pid, pgid,
			rval);
	}

	return rval;
}

pid_t safe_getpgid(const char *file, const int lineno, pid_t pid)
{
	pid_t pgid;

	pgid = getpgid(pid);

	if (pgid == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO, "getpgid(%i) failed",
			pid);
	} else if (pgid < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid getpgid(%i) return value %d", pid, pgid);
	}

	return pgid;
}

int safe_setgroups(const char *file, const int lineno, size_t size, const gid_t *list)
{
	int rval;

	rval = setgroups(size, list);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"setgroups(%zu, %p) failed", size, list);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid setgroups(%zu, %p) return value %d", size,
			list, rval);
	}

	return rval;
}

int safe_getgroups(const char *file, const int lineno, int size, gid_t list[])
{
	int rval;

	rval = getgroups(size, list);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"getgroups(%i, %p)", size, list);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid getgroups(%i, %p) return value %d", size,
			list, rval);
	}

	return rval;
}

int safe_personality(const char *filename, unsigned int lineno,
		    unsigned long persona)
{
	int prev_persona = personality(persona);

	if (prev_persona == -1) {
		tst_brk_(filename, lineno, TBROK | TERRNO,
			 "persona(%ld) failed", persona);
	} else if (prev_persona < 0) {
		tst_brk_(filename, lineno, TBROK | TERRNO,
			 "Invalid persona(%ld) return value %d", persona,
			 prev_persona);
	}

	return prev_persona;
}

int safe_pidfd_open(const char *file, const int lineno, pid_t pid,
		   unsigned int flags)
{
	int rval;

	rval = pidfd_open(pid, flags);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "pidfd_open(%i, %i) failed", pid, flags);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "Invalid pidfd_open(%i, %i) return value %d",
			 pid, flags, rval);
	}

	return rval;
}

int safe_setregid(const char *file, const int lineno,
		  gid_t rgid, gid_t egid)
{
	int rval;

	rval = setregid(rgid, egid);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "setregid(%li, %li) failed", (long)rgid, (long)egid);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "Invalid setregid(%li, %li) return value %d",
			 (long)rgid, (long)egid, rval);
	}

	return rval;
}

int safe_setreuid(const char *file, const int lineno,
		  uid_t ruid, uid_t euid)
{
	int rval;

	rval = setreuid(ruid, euid);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "setreuid(%li, %li) failed", (long)ruid, (long)euid);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "Invalid setreuid(%li, %li) return value %d",
			 (long)ruid, (long)euid, rval);
	}

	return rval;
}

int safe_setresgid(const char *file, const int lineno,
	gid_t rgid, gid_t egid, gid_t sgid)
{
	int ret;

	ret = setresgid(rgid, egid, sgid);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"setregid(%li, %li, %li) failed", (long)rgid,
			(long)egid, (long)sgid);
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid setregid(%li, %li, %li) return value %d",
			(long)rgid, (long)egid, (long)sgid, ret);
	}

	return ret;
}

int safe_setresuid(const char *file, const int lineno,
	uid_t ruid, uid_t euid, uid_t suid)
{
	int ret;

	ret = setresuid(ruid, euid, suid);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"setreuid(%li, %li, %li) failed", (long)ruid,
			(long)euid, (long)suid);
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid setreuid(%li, %li, %li) return value %d",
			(long)ruid, (long)euid, (long)suid, ret);
	}

	return ret;
}

int safe_sigaction(const char *file, const int lineno,
				   int signum, const struct sigaction *act,
				   struct sigaction *oldact)
{
	int rval;

	rval = sigaction(signum, act, oldact);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"sigaction(%s (%d), %p, %p) failed",
			tst_strsig(signum), signum, act, oldact);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid sigaction(%s (%d), %p, %p) return value %d",
			tst_strsig(signum), signum, act, oldact, rval);
	}

	return rval;
}

int safe_sigaddset(const char *file, const int lineno,
				   sigset_t *sigs, int signo)
{
	int rval;

	rval = sigaddset(sigs, signo);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"sigaddset() %s (%i) failed", tst_strsig(signo),
			signo);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid sigaddset() %s (%i) return value %d",
			tst_strsig(signo), signo, rval);
	}

	return rval;
}

int safe_sigdelset(const char *file, const int lineno, sigset_t *sigs, int signo)
{
	int rval;

	rval = sigdelset(sigs, signo);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"sigdelset() %s (%i) failed", tst_strsig(signo),
			signo);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid sigdelset() %s (%i) return value %d",
			tst_strsig(signo), signo, rval);
	}

	return rval;
}

int safe_sigemptyset(const char *file, const int lineno, sigset_t *sigs)
{
	int rval;

	rval = sigemptyset(sigs);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO, "sigemptyset() failed");
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid sigemptyset() return value %d", rval);
	}

	return rval;
}

int safe_sigfillset(const char *file, const int lineno,
		     sigset_t *sigs)
{
	int rval;

	rval = sigfillset(sigs);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO, "sigfillset() failed");
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid sigfillset() return value %d", rval);
	}

	return rval;
}

static const char *strhow(int how)
{
	switch (how) {
	case SIG_BLOCK:
		return "SIG_BLOCK";
	case SIG_UNBLOCK:
		return "SIG_UNBLOCK";
	case SIG_SETMASK:
		return "SIG_SETMASK";
	default:
		return "???";
	}
}

int safe_sigprocmask(const char *file, const int lineno,
					 int how, sigset_t *set, sigset_t *oldset)
{
	int rval;

	rval = sigprocmask(how, set, oldset);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"sigprocmask(%s, %p, %p) failed", strhow(how), set,
			oldset);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid sigprocmask(%s, %p, %p) return value %d",
			strhow(how), set, oldset, rval);
	}

	return rval;
}

int safe_sigwait(const char *file, const int lineno, sigset_t *set, int *sig)
{
	int rval;

	rval = sigwait(set, sig);

	if (rval > 0) {
		errno = rval;
		tst_brk_(file, lineno, TBROK | TERRNO,
			"sigwait(%p, %p) failed", set, sig);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK,
			"Invalid sigwait(%p, %p) return value %d", set, sig,
			rval);
	}

	return rval;
}

struct group *safe_getgrnam(const char *file, const int lineno,
			    const char *name)
{
	struct group *rval;

	errno = 0;
	rval = getgrnam(name);
	if (rval == NULL) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"getgrnam(%s) failed", name);
	}

	return rval;
}

struct group *safe_getgrnam_fallback(const char *file, const int lineno,
				     const char *name, const char *fallback)
{
	struct group *rval;

	errno = 0;
	rval = getgrnam(name);
	if (rval == NULL) {
		tst_res_(file, lineno, TINFO,
			 "getgrnam(%s) failed - try fallback %s",
			 name, fallback);
		rval = safe_getgrnam(file, lineno, fallback);
	}

	return rval;
}

struct group *safe_getgrgid(const char *file, const int lineno, gid_t gid)
{
	struct group *rval;

	errno = 0;
	rval = getgrgid(gid);
	if (rval == NULL) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"getgrgid(%li) failed", (long)gid);
	}

	return rval;
}

int safe_chroot(const char *file, const int lineno, const char *path)
{
	int rval;

	rval = chroot(path);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO, "chroot(%s) failed",
			path);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "Invalid chroot(%s) return value %d", path, rval);
	}

	return rval;
}

int safe_unshare(const char *file, const int lineno, int flags)
{
	int res;

	res = unshare(flags);

	if (res == -1) {
		if (errno == EINVAL) {
			tst_brk_(file, lineno, TCONF | TERRNO,
				 "unshare(%d) unsupported", flags);
		} else {
			tst_brk_(file, lineno, TBROK | TERRNO,
				 "unshare(%d) failed", flags);
		}
	} else if (res) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "Invalid unshare(%d) return value %d", flags, res);
	}

	return res;
}

int safe_setns(const char *file, const int lineno, int fd, int nstype)
{
	int ret;

	ret = setns(fd, nstype);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO, "setns(%i, %i) failed",
			fd, nstype);
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid setns(%i, %i) return value %d", fd, nstype,
			ret);
	}

	return ret;
}

long tst_safe_ptrace(const char *file, const int lineno, int req, pid_t pid,
	void *addr, void *data)
{
	long ret;

	errno = 0;
	ret = ptrace(req, pid, addr, data);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO, "ptrace() failed");
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid ptrace() return value %ld", ret);
	}

	return ret;
}

int safe_pipe2(const char *file, const int lineno, int fildes[2], int flags)
{
	int ret;

	ret = pipe2(fildes, flags);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"pipe2({%d,%d}) failed with flag(%d)", fildes[0],
			fildes[1], flags);
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid pipe2({%d,%d}, %d) return value %d",
			fildes[0], fildes[1], flags, ret);
	}

	return ret;
}

int safe_dup(const char *file, const int lineno, int oldfd)
{
	int rval;

	rval = dup(oldfd);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"dup(%i) failed", oldfd);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid dup(%i) return value %d", oldfd, rval);
	}

	return rval;
}

int safe_dup2(const char *file, const int lineno, int oldfd, int newfd)
{
	int rval;

	rval = dup2(oldfd, newfd);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "dup2(%i, %i) failed", oldfd, newfd);
	} else if (rval != newfd) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "Invalid dup2(%i, %i) return value %d",
			 oldfd, newfd, rval);
	}

	return rval;
}

void *safe_calloc(const char *file, const int lineno, size_t nmemb, size_t size)
{
	void *rval;

	rval = calloc(nmemb, size);

	if (rval == NULL) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"calloc(%zu, %zu) failed", nmemb, size);
	}

	return rval;
}

void *safe_realloc(const char *file, const int lineno, void *ptr, size_t size)
{
	void *ret;

	ret = realloc(ptr, size);

	if (!ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"realloc(%p, %zu) failed", ptr, size);
	}

	return ret;
}

sighandler_t safe_signal(const char *file, const int lineno,
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

void safe_cmd(const char *file, const int lineno, const char *const argv[],
	const char *stdout_path, const char *stderr_path)
{
	int rval;

	switch ((rval = tst_cmd(argv, stdout_path, stderr_path,
		TST_CMD_PASS_RETVAL | TST_CMD_TCONF_ON_MISSING))) {
	case 0:
		break;
	default:
		tst_brk_(file, lineno, TBROK, "%s failed (%d)", argv[0], rval);
	}
}

int safe_msync(const char *file, const int lineno, void *addr,
				size_t length, int flags)
{
	int rval;

	rval = msync(addr, length, flags);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"msync(%p, %zu, %d) failed", addr, length, flags);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid msync(%p, %zu, %d) return value %d",
			addr, length, flags, rval);
	}

	return rval;
}

void safe_print_file(const char *file, const int lineno, char *path)
{
	FILE *pfile;
	char line[PATH_MAX];

	tst_res(TINFO, "=== %s ===", path);

	pfile = safe_fopen(file, lineno, NULL, path, "r");

	while (fgets(line, sizeof(line), pfile))
		fprintf(stderr, "%s", line);

	safe_fclose(file, lineno, NULL, pfile);
}

int safe_sscanf(const char *file, const int lineno, const char *restrict buffer, const char *restrict format, ...)
{
	va_list args;

	va_start(args, format);
	int ret = vsscanf(buffer, format, args);

	va_end(args);
	int placeholders = tst_count_scanf_conversions(format);

	if (ret == EOF)
		tst_brk_(file, lineno, TBROK | TERRNO, "got EOF from sscanf()");

	if (ret != placeholders)
		tst_brk_(file, lineno, TBROK | TERRNO, "wrong number of conversion, expected %d, got %d", placeholders, ret);

	return ret;
}

#define PROT_FLAG_STR(f) #f " | "
void tst_prot_to_str(const int prot, char *buf)
{
	char *ptr = buf;

	if (prot == PROT_NONE) {
		strcpy(buf, "PROT_NONE");
		return;
	}

	if (prot & PROT_READ) {
		strcpy(ptr, PROT_FLAG_STR(PROT_READ));
		ptr += sizeof(PROT_FLAG_STR(PROT_READ)) - 1;
	}

	if (prot & PROT_WRITE) {
		strcpy(ptr, PROT_FLAG_STR(PROT_WRITE));
		ptr += sizeof(PROT_FLAG_STR(PROT_WRITE)) - 1;
	}

	if (prot & PROT_EXEC) {
		strcpy(ptr, PROT_FLAG_STR(PROT_EXEC));
		ptr += sizeof(PROT_FLAG_STR(PROT_EXEC)) - 1;
	}

	if (buf != ptr)
		ptr[-3] = 0;
}

int safe_mprotect(const char *file, const int lineno,
	char *addr, size_t len, int prot)
{
	int rval;
	char prot_buf[512];

	tst_prot_to_str(prot, prot_buf);

	tst_res_(file, lineno, TDEBUG,
		"mprotect(%p, %ld, %s(%x))", addr, len, prot_buf, prot);

	rval = mprotect(addr, len, prot);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"mprotect(%p, %ld, %s(%x))", addr, len, prot_buf, prot);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"mprotect(%p, %ld, %s(%x)) return value %d",
			addr, len, prot_buf, prot, rval);
	}

	return rval;
}
