/*
 * Copyright (c) 2009 Cisco Systems, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */

#ifndef __LTP_SIGNAL_H
#define __LTP_SIGNAL_H

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include "config.h"

#define SIGSETSIZE (_NSIG / 8)

#ifdef LTP_RT_SIG_TEST

extern int  expected_signal_number;

#if defined(__x86_64__)

/*
 * From asm/signal.h -- this value isn't exported anywhere outside of glibc and
 * asm/signal.h and is only required for the rt_sig* function family because
 * sigaction(2), et all, appends this if necessary to
 * (struct sigaction).sa_flags. HEH.
 *
 * I do #undef though, just in case...
 */
#undef SA_RESTORER
#define SA_RESTORER	0x04000000

/* 
 * From .../arch/x86/kernel/signal.c:448 --
 *
 * x86-64 should always use SA_RESTORER.
 *
 * -- thus SA_RESTORER must always be defined along with
 * (struct sigaction).sa_restorer.
 */
#define ARCH_SPECIFIC_RT_SIGACTION_SETUP(__sa) __sa.sa_flags |= SA_RESTORER; __sa.sa_restorer = dummy_restorer;
/* defined(__x86_64) */
#else
#define ARCH_SPECIFIC_RT_SIGACTION_SETUP(__sa)
#endif

/*
 * To help direct sigsegv_sigaction_handler in determining whether or not the
 * segfault was valid.
 */
extern int  expected_signal_number;

/* 
 * A dummy sa_restorer function, because we have to have it if SA_RESTORER is
 * required.
 */
inline void
dummy_restorer(void) {
	tst_resm(TINFO, "%s called", __func__);
}

/* 
 * SIGSEGV will be thrown if an overflow occurs, or some other undesired
 * precondition. Let's catch it so the test will at least proceed and not
 * completely bomb the heck out.
 */
inline void
sigsegv_sigaction_handler(int signum, siginfo_t *siginfo, void *ucontext) {

	/*
	 * Backwards compatibility is a pain; I think that's what's driving
	 * the implicit sa_restorer BS on x86_64, and the reason why mips* and
	 * ppc* only has a sigaction handler (not rt_sigaction).
	 *
	 * GG for backwards compatibility and lack of documentation on an
	 * internal syscall...
	 */
#ifdef __x86_64__
	if (expected_signal_number == SIGRTMIN) {
		tst_resm(TINFO, "SIGRTMIN segfaults on x86_64 (known issue).");
	} else {
#endif
	tst_brkm(TBROK | TERRNO, NULL,
		"Uncaught SIGSEGV; please validate whether or not test meets "
		"functional requirements as per do_signal and callees in "
		"$KERN_SRC/arch/<arch>/kernel/signal.c is concerned");

#ifdef __x86_64__
	}
#endif
}

/* 
 * Catch SIGSEGV just in case one of the rt_sigaction calls tosses up a SIGSEGV
 * at the kernel level because of an -EFAULT was tossed by a caller. What a
 * PITA :].
 */
inline int
setup_sigsegv_sigaction_handler()
{
	int rc = -1;
	struct sigaction sa;
	sigset_t sigset;

	/* 
	 * Catch SIGSEGV just in case one of the rt_sigaction calls tosses up a
	 * SIGSEGV at the kernel level because of an -EFAULT was tossed by a
	 * caller. What a PITA :].
	 */
	sa.sa_sigaction = (void *)sigsegv_sigaction_handler;
	/* We want the handler to persist, so don't do SA_RESETHAND... */
	sa.sa_flags = SA_SIGINFO;

	if (sigemptyset(&sa.sa_mask) < 0) {
		tst_brkm(TBROK | TERRNO, CLEANUP,
			"Failed to call sigemptyset for SIGSEGV");
	} else if (sigaddset(&sigset, SIGSEGV) < 0) {
		tst_brkm(TBROK | TERRNO, CLEANUP,
			"Failed to do sigaddset for SIGSEGV");
	} else if (sigaction(SIGSEGV, &sa, (struct sigaction *) NULL) < 0) {
		tst_brkm(TBROK | TERRNO, CLEANUP,
			"Failed to setup sighandler for SIGSEGV");
	} else {
		rc = 0;
	}

	return rc;

}

#endif /* RT_SIG_TEST */

#endif
