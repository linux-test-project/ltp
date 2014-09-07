/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */

/* $Id: tst_sig.c,v 1.13 2009/08/28 09:29:01 vapier Exp $ */

/*****************************************************************************
	OS Testing  - Silicon Graphics, Inc.

	FUNCTION IDENTIFIER : tst_sig  Set up for unexpected signals.

	AUTHOR          : David D. Fenner

	CO-PILOT        : Bill Roske

	DATE STARTED    : 06/06/90

	This module may be linked with c-modules requiring unexpected
	signal handling.  The parameters to tst_sig are as follows:

		fork_flag - set to FORK or NOFORK depending upon whether the
			calling program executes a fork() system call.  It
			is normally the case that the calling program treats
			SIGCHLD as an expected signal if fork() is being used.

		handler - a pointer to the unexpected signal handler to
			be executed after an unexpected signal has been
			detected.  If handler is set to DEF_HANDLER, a
			default handler is used.  This routine should be
			declared as function returning an int.

		cleanup - a pointer to a cleanup routine to be executed
			by the unexpected signal handler before tst_exit is
			called.  This parameter is set to NULL if no cleanup
			routine is required.  An external variable, T_cleanup
			is set so that other user-defined handlers have
			access to the cleanup routine.  This routine should be
			declared as returning type void.

***************************************************************************/

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "test.h"

#define MAXMESG 150		/* size of mesg string sent to tst_res */

static void (*T_cleanup) ();

static void def_handler();	/* default signal handler */
static void (*tst_setup_signal(int, void (*)(int))) (int);

/****************************************************************************
 * tst_sig() : set-up to catch unexpected signals.  fork_flag is set to NOFORK
 *    if SIGCHLD is to be an "unexpected signal", otherwise it is set to
 *    FORK.  cleanup points to a cleanup routine to be executed before
 *    tst_exit is called (cleanup is set to NULL if no cleanup is desired).
 *    handler is a pointer to the signal handling routine (if handler is
 *    set to NULL, a default handler is used).
 ***************************************************************************/

void tst_sig(int fork_flag, void (*handler) (), void (*cleanup) ())
{
	int sig;
#ifdef _SC_SIGRT_MIN
	long sigrtmin, sigrtmax;
#endif

	/*
	 * save T_cleanup and handler function pointers
	 */
	T_cleanup = cleanup;	/* used by default handler */

	if (handler == DEF_HANDLER) {
		/* use default handler */
		handler = def_handler;
	}
#ifdef _SC_SIGRT_MIN
	sigrtmin = sysconf(_SC_SIGRT_MIN);
	sigrtmax = sysconf(_SC_SIGRT_MAX);
#endif

	/*
	 * now loop through all signals and set the handlers
	 */

	for (sig = 1; sig < NSIG; sig++) {
		/*
		 * SIGKILL is never unexpected.
		 * SIGCHLD is only unexpected when
		 *    no forking is being done.
		 * SIGINFO is used for file quotas and should be expected
		 */

#ifdef _SC_SIGRT_MIN
		if (sig >= sigrtmin && sig <= sigrtmax)
			continue;
#endif

		switch (sig) {
		case SIGKILL:
		case SIGSTOP:
		case SIGCONT:
#if !defined(_SC_SIGRT_MIN) && defined(__SIGRTMIN) && defined(__SIGRTMAX)
			/* Ignore all real-time signals */
		case __SIGRTMIN:
		case __SIGRTMIN + 1:
		case __SIGRTMIN + 2:
		case __SIGRTMIN + 3:
		case __SIGRTMIN + 4:
		case __SIGRTMIN + 5:
		case __SIGRTMIN + 6:
		case __SIGRTMIN + 7:
		case __SIGRTMIN + 8:
		case __SIGRTMIN + 9:
		case __SIGRTMIN + 10:
		case __SIGRTMIN + 11:
		case __SIGRTMIN + 12:
		case __SIGRTMIN + 13:
		case __SIGRTMIN + 14:
		case __SIGRTMIN + 15:
/* __SIGRTMIN is 37 on HPPA rather than 32 *
 * as on i386, etc.                        */
#if !defined(__hppa__)
		case __SIGRTMAX - 15:
		case __SIGRTMAX - 14:
		case __SIGRTMAX - 13:
		case __SIGRTMAX - 12:
		case __SIGRTMAX - 11:
#endif
		case __SIGRTMAX - 10:
		case __SIGRTMAX - 9:
		case __SIGRTMAX - 8:
		case __SIGRTMAX - 7:
		case __SIGRTMAX - 6:
		case __SIGRTMAX - 5:
		case __SIGRTMAX - 4:
		case __SIGRTMAX - 3:
		case __SIGRTMAX - 2:
		case __SIGRTMAX - 1:
		case __SIGRTMAX:
#endif
#ifdef SIGSWAP
		case SIGSWAP:
#endif /* SIGSWAP */

#ifdef SIGCKPT
		case SIGCKPT:
#endif
#ifdef SIGRESTART
		case SIGRESTART:
#endif
			/*
			 * pthread-private signals SIGPTINTR and SIGPTRESCHED.
			 * Setting a handler for these signals is disallowed when
			 * the binary is linked against libpthread.
			 */
#ifdef SIGPTINTR
		case SIGPTINTR:
#endif /* SIGPTINTR */
#ifdef SIGPTRESCHED
		case SIGPTRESCHED:
#endif /* SIGPTRESCHED */
#ifdef _SIGRESERVE
		case _SIGRESERVE:
#endif
#ifdef _SIGDIL
		case _SIGDIL:
#endif
#ifdef _SIGCANCEL
		case _SIGCANCEL:
#endif
#ifdef _SIGGFAULT
		case _SIGGFAULT:
#endif
			break;

		case SIGCHLD:
			if (fork_flag == FORK)
				continue;

		default:
			if (tst_setup_signal(sig, handler) == SIG_ERR)
				tst_resm(TWARN | TERRNO,
					 "signal failed for signal %d", sig);
			break;
		}
	}
}

/****************************************************************************
 * def_handler() : default signal handler that is invoked when
 *      an unexpected signal is caught.
 ***************************************************************************/

static void def_handler(int sig)
{
	/*
	 * Break remaining test cases, do any cleanup, then exit
	 */
	tst_brkm(TBROK, T_cleanup,
		 "unexpected signal %s(%d) received (pid = %d).",
		 tst_strsig(sig), sig, getpid());
}

/*
 * tst_setup_signal - A function like signal(), but we have
 *                    control over its personality.
 */
static void (*tst_setup_signal(int sig, void (*handler) (int))) (int) {
	struct sigaction my_act, old_act;
	int ret;

	my_act.sa_handler = handler;
	my_act.sa_flags = SA_RESTART;
	sigemptyset(&my_act.sa_mask);

	ret = sigaction(sig, &my_act, &old_act);

	if (ret == 0)
		return old_act.sa_handler;
	else
		return SIG_ERR;
}
