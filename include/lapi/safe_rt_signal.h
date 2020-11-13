/*
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef SAFE_RT_SIGNAL_H__
#define SAFE_RT_SIGNAL_H__

#include <signal.h>
#include "lapi/rt_sigaction.h"

static inline int safe_rt_sigaction(const char *file, const int lineno,
                                    int signum, const struct sigaction *act,
                                    struct sigaction *oact, size_t sigsetsize)
{
	int ret;

	ret = ltp_rt_sigaction(signum, act, oact, sigsetsize);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"ltp_rt_sigaction(%i, %p, %p, %zu) failed",
			signum, act, oact, sigsetsize);
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid ltp_rt_sigaction(%i, %p, %p, %zu) return value %d",
			signum, act, oact, sigsetsize, ret);
	}

	return ret;
}

#define SAFE_RT_SIGACTION(signum, act, oldact, sigsetsize) \
	safe_rt_sigaction(__FILE__, __LINE__, signum, act, oldact, sigsetsize)


static inline int safe_rt_sigprocmask(const char *file, const int lineno,
                                      int how, const sigset_t *set,
                                      sigset_t *oldset, size_t sigsetsize)
{
	int ret;

	ret = tst_syscall(__NR_rt_sigprocmask, how, set, oldset, sigsetsize);
	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"rt_sigprocmask(%i, %p, %p, %zu) failed",
			how, set, oldset, sigsetsize);
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid rt_sigprocmask(%i, %p, %p, %zu) return value %d",
			how, set, oldset, sigsetsize, ret);
	}

	return ret;
}

#define SAFE_RT_SIGPROCMASK(how, set, oldset, sigsetsize) \
	safe_rt_sigprocmask(__FILE__, __LINE__, how, set, oldset, sigsetsize)

#endif /* SAFE_RT_SIGNAL_H__ */
