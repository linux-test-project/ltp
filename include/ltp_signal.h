/*
 * Copyright (c) 2009 Cisco Systems, Inc.  All Rights Reserved.
 * Copyright (c) 2009 FUJITSU LIMITED.  All Rights Reserved.
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
 * Author: Liu Bo <liubo2009@cn.fujitsu.com>
 * Author: Garrett Cooper <yanegomi@gmail.com>
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
#define HAVE_SA_RESTORER
#define SA_RESTORER	0x04000000

struct kernel_sigaction {
	__sighandler_t k_sa_handler;
	unsigned long sa_flags;
	void (*sa_restorer) (void);
	sigset_t sa_mask;
};

void (*restore_rt) (void);

inline void
handler_h (int signal)
{
	return;
}

/* Setup an initial signal handler for signal number = sig for x86_64. */
inline int
sig_initial(int sig)
{
	int ret_code = -1;
	struct sigaction act, oact;

	act.sa_handler = handler_h;
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

#endif /* LTP_RT_SIG_TEST */

#endif
