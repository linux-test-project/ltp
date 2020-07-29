/*
 * Copyright (C) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef SELECT_VAR__
#define SELECT_VAR__

#include "lapi/syscalls.h"
#include "tst_timer.h"

struct compat_sel_arg_struct {
	long _n;
	long _inp;
	long _outp;
	long _exp;
	long _tvp;
};

static int do_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	switch (tst_variant) {
	case 0:
		return select(nfds, readfds, writefds, exceptfds, timeout);
	break;
	case 1: {
#ifdef __LP64__
		return tst_syscall(__NR_select, nfds, readfds, writefds, exceptfds, timeout);
#else
		struct compat_sel_arg_struct arg = {
			._n = (long)nfds,
			._inp = (long)readfds,
			._outp = (long)writefds,
			._exp = (long)exceptfds,
			._tvp = (long)timeout,
		};

		return tst_syscall(__NR_select, &arg);
#endif /* __LP64__ */
	}
	case 2: {
		int ret;
		struct __kernel_old_timespec ts = {
			.tv_sec = timeout->tv_sec,
			.tv_nsec = timeout->tv_usec * 1000,
		};
		ret = tst_syscall(__NR_pselect6, nfds, readfds, writefds, exceptfds, &ts, NULL);
		timeout->tv_sec = ts.tv_sec;
		timeout->tv_usec = ts.tv_nsec / 1000;
		return ret;
	}
	case 3: {
		int ret = 0;
#if (__NR_pselect6_time64 != __LTP__NR_INVALID_SYSCALL)
		struct __kernel_timespec ts = {
			.tv_sec = timeout->tv_sec,
			.tv_nsec = timeout->tv_usec * 1000,
		};
		ret = tst_syscall(__NR_pselect6_time64, nfds, readfds, writefds, exceptfds, &ts, NULL);
		timeout->tv_sec = ts.tv_sec;
		timeout->tv_usec = ts.tv_nsec / 1000;
#else
		tst_brk(TCONF, "__NR_pselect6 time64 variant not supported");
#endif
		return ret;
	}
	case 4:
#ifdef __NR__newselect
		return tst_syscall(__NR__newselect, nfds, readfds, writefds, exceptfds, timeout);
#else
		tst_brk(TCONF, "__NR__newselect not implemented");
#endif
	break;
	}

	return -1;
}

static void select_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Testing libc select()");
	break;
	case 1:
		tst_res(TINFO, "Testing SYS_select syscall");
	break;
	case 2:
		tst_res(TINFO, "Testing SYS_pselect6 syscall");
	break;
	case 3:
		tst_res(TINFO, "Testing SYS_pselect6 time64 syscall");
	break;
	case 4:
		tst_res(TINFO, "Testing SYS__newselect syscall");
	break;
	}
}

#define TEST_VARIANTS 5

#endif /* SELECT_VAR__ */
