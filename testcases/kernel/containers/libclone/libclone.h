#ifndef __LIBCLONE_H
#define __LIBCLONE_H

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <sched.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>

#define T_UNSHARE 0
#define T_CLONE 1
#define T_NONE 2

#ifndef SYS_unshare
#ifdef __NR_unshare
#define SYS_unshare __NR_unshare
#elif __i386__
#define SYS_unshare 310
#elif __ia64__
#define SYS_unshare 1296
#elif __x86_64__
#define SYS_unshare 272
#elif __s390x__ || __s390__
#define SYS_unshare 303
#elif __powerpc__
#define SYS_unshare 282
#else
#error "unshare not supported on this architecure."
#endif
#endif

#ifndef CLONE_NEWUTS
#define CLONE_NEWUTS		0x04000000
#endif

#ifndef CLONE_NEWIPC
#define CLONE_NEWIPC		0x08000000
#endif

#ifndef CLONE_NEWUSER
#define CLONE_NEWUSER		0x10000000
#endif

#ifndef CLONE_NEWNET2
#define CLONE_NEWNET2		0x20000000
#endif

#ifndef CLONE_NEWNET3
#define CLONE_NEWNET3		0x40000000
#endif

#ifndef CLONE_NEWPID
#define CLONE_NEWPID		0x80000000
#endif

/*
 * Run fn1 in a unshared environmnent, and fn2 in the original context
 * Fn2 may be NULL.
 */

int do_clone_tests(unsigned long clone_flags,
			int(*fn1)(void *arg), void *arg1,
			int(*fn2)(void *arg), void *arg2);

int do_unshare_tests(unsigned long clone_flags,
			int (*fn1)(void *arg), void *arg1,
			int (*fn2)(void *arg), void *arg2);

int do_fork_tests(int (*fn1)(void *arg), void *arg1,
			int (*fn2)(void *arg), void *arg2);

int do_clone_unshare_test(int use_clone, unsigned long clone_flags,
			int (*fn1)(void *arg), void *arg1);

int do_clone_unshare_tests(int use_clone, unsigned long clone_flags,
			int (*fn1)(void *arg), void *arg1,
			int (*fn2)(void *arg), void *arg2);

#endif
