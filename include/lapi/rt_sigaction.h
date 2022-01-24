// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2009 Cisco Systems, Inc.  All Rights Reserved.
 * Copyright (c) 2009 FUJITSU LIMITED.  All Rights Reserved.
 * Author: Liu Bo <liubo2009@cn.fujitsu.com>
 * Author: Ngie Cooper <yaneurabeya@gmail.com>
 */

#ifndef LAPI_RT_SIGACTION_H__
#define LAPI_RT_SIGACTION_H__

#include "ltp_signal.h"

#define INVAL_SA_PTR ((void *)-1)

#if defined(__mips__)
struct kernel_sigaction {
	unsigned int sa_flags;
	void (* k_sa_handler)(int);
	sigset_t sa_mask;
};
#else
struct kernel_sigaction {
	void (* k_sa_handler)(int);
	unsigned long sa_flags;
	void (*sa_restorer) (void);
	sigset_t sa_mask;
};
#endif

/* This macro marks if (struct sigaction) has .sa_restorer member */
#if !defined(__ia64__) && !defined(__alpha__) && !defined(__hppa__) && !defined(__mips__)
# define HAVE_SA_RESTORER
#endif

#ifdef __x86_64__

/*
 * From asm/signal.h -- this value isn't exported anywhere outside of glibc and
 * asm/signal.h and is only required for the rt_sig* function family because
 * sigaction(2), et all, appends this if necessary to
 * (struct sigaction).sa_flags. HEH.
 *
 * I do #undef though, just in case...
 *
 * Also, from .../arch/x86/kernel/signal.c:448 for v2.6.30 (something or
 * other):
 *
 * x86-64 should always use SA_RESTORER.
 *
 * -- thus SA_RESTORER must always be defined along with
 * (struct sigaction).sa_restorer for this architecture.
 */
#undef SA_RESTORER
#define SA_RESTORER     0x04000000

void (*restore_rt)(void);

static void handler_h(int signal)
{
	return;
}

/* Setup an initial signal handler for signal number = sig for x86_64. */
static inline int sig_initial(int sig)
{
	int ret_code = -1;
	struct sigaction act, oact;

	act.sa_handler = handler_h;
	act.sa_flags = 0;
	/* Clear out the signal set. */
	if (sigemptyset(&act.sa_mask) < 0) {
		/* Add the signal to the mask set. */
	} else if (sigaddset(&act.sa_mask, sig) < 0) {
		/* Set act.sa_restorer via syscall(2) */
	} else if (sigaction(sig, &act, &oact) < 0) {
		/* Copy oact.sa_restorer via syscall(2) */
	} else if (sigaction(sig, &act, &oact) < 0) {
		/* And voila -- we just tricked the kernel into giving us our
		 * restorer function! */
	} else {
		restore_rt = oact.sa_restorer;
		ret_code = 0;
	}

	return ret_code;
}

#endif /* __x86_64__ */

#ifdef __sparc__
# if defined __arch64__ || defined __sparcv9

/*
 * Based on glibc/sysdeps/unix/sysv/linux/sparc/sparc64/sigaction.c
 */

extern char *__rt_sig_stub;

static void __attribute__((used)) __rt_sigreturn_stub(void)
{
	__asm__ ("__rt_sig_stub: mov %0, %%g1\n\t"
		"ta  0x6d\n\t"
		: /* no outputs */
		: "i" (__NR_rt_sigreturn));
}

# else /* sparc32 */

/*
 * Based on glibc/sysdeps/unix/sysv/linux/sparc/sparc32/sigaction.c
 */

extern char *__rt_sig_stub, *__sig_stub;

static void __attribute__((used)) __rt_sigreturn_stub(void)
{
	__asm__ ("__rt_sig_stub: mov %0, %%g1\n\t"
		"ta  0x10\n\t"
		: /* no outputs */
		: "i" (__NR_rt_sigreturn));
}

static void __attribute__((used)) __sigreturn_stub(void)
{
	__asm__ ("__sig_stub: mov %0, %%g1\n\t"
		"ta  0x10\n\t"
		: /* no outputs */
		: "i" (__NR_sigreturn));
}

# endif
#endif /* __sparc__ */

#ifdef __arc__

#undef SA_RESTORER
#define SA_RESTORER     0x04000000

/*
 * based on uClibc/libc/sysdeps/linux/arc/sigaction.c
 */
static void
__attribute__ ((optimize("Os"))) __attribute__((used)) restore_rt(void)
{
	__asm__ (
		"mov r8, %0	\n\t"
#ifdef __ARCHS__
		"trap_s	0	\n\t"
#else
		"trap0	\n\t"
#endif
		: /* no outputs */
		: "i" (__NR_rt_sigreturn)
		: "r8");
}
#endif

/* This is a wrapper for __NR_rt_sigaction syscall.
 * act/oact values of INVAL_SA_PTR is used to pass
 * an invalid pointer to syscall(__NR_rt_sigaction)
 *
 * Based on glibc/sysdeps/unix/sysv/linux/{...}/sigaction.c
 */

static int ltp_rt_sigaction(int signum, const struct sigaction *act,
			struct sigaction *oact, size_t sigsetsize)
{
	int ret;
	struct kernel_sigaction kact, koact;
	struct kernel_sigaction *kact_p = NULL;
	struct kernel_sigaction *koact_p = NULL;

	if (act == INVAL_SA_PTR) {
		kact_p = INVAL_SA_PTR;
	} else if (act) {
		kact.k_sa_handler = act->sa_handler;
		memcpy(&kact.sa_mask, &act->sa_mask, sizeof(sigset_t));
		kact.sa_flags = act->sa_flags;
#ifndef __mips__
		kact.sa_restorer = NULL;
#endif
		kact_p = &kact;
	}

	if (oact == INVAL_SA_PTR)
		koact_p = INVAL_SA_PTR;
	else if (oact)
		koact_p = &koact;

#ifdef __x86_64__
	sig_initial(signum);
#endif

#if defined __x86_64__ || defined __arc__
	kact.sa_flags |= SA_RESTORER;
	kact.sa_restorer = restore_rt;
#endif

#ifdef __sparc__
	unsigned long stub = 0;
# if defined __arch64__ || defined __sparcv9
	stub = ((unsigned long) &__rt_sig_stub) - 8;
# else /* sparc32 */
	if ((kact.sa_flags & SA_SIGINFO) != 0)
		stub = ((unsigned long) &__rt_sig_stub) - 8;
	else
		stub = ((unsigned long) &__sig_stub) - 8;
# endif
#endif


#ifdef __sparc__
	ret = tst_syscall(__NR_rt_sigaction, signum,
			kact_p, koact_p,
			stub, sigsetsize);
#else
	ret = tst_syscall(__NR_rt_sigaction, signum,
			kact_p, koact_p,
			sigsetsize);
#endif

	if (ret >= 0) {
		if (oact && (oact != INVAL_SA_PTR)) {
			oact->sa_handler = koact.k_sa_handler;
			memcpy(&oact->sa_mask, &koact.sa_mask,
				sizeof(sigset_t));
			oact->sa_flags = koact.sa_flags;
#ifdef HAVE_SA_RESTORER
			oact->sa_restorer = koact.sa_restorer;
#endif
		}
	}

	return ret;
}

#endif /* LAPI_RT_SIGACTION_H__ */
