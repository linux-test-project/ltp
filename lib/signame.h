/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

const char *tst_strsig(int sig)
{
	static const struct pair signal_pairs[] = {
		PAIR(SIGHUP)
		PAIR(SIGINT)
		PAIR(SIGQUIT)
		PAIR(SIGILL)
	#ifdef SIGTRAP
		PAIR(SIGTRAP)
	#endif

	#ifdef SIGIOT
		/* SIGIOT same as SIGABRT */
		STRPAIR(SIGABRT, "SIGIOT/SIGABRT")
	#else
		PAIR(SIGABRT)
	#endif

	#ifdef SIGEMT
		PAIR(SIGEMT)
	#endif
	#ifdef SIGBUS
		PAIR(SIGBUS)
	#endif
		PAIR(SIGFPE)
		PAIR(SIGKILL)
		PAIR(SIGUSR1)
		PAIR(SIGSEGV)
		PAIR(SIGUSR2)
		PAIR(SIGPIPE)
		PAIR(SIGALRM)
		PAIR(SIGTERM)
	#ifdef SIGSTKFLT
		PAIR(SIGSTKFLT)
	#endif
		PAIR(SIGCHLD)
		PAIR(SIGCONT)
		PAIR(SIGSTOP)
		PAIR(SIGTSTP)
		PAIR(SIGTTIN)
		PAIR(SIGTTOU)
	#ifdef SIGURG
		PAIR(SIGURG)
	#endif
	#ifdef SIGXCPU
		PAIR(SIGXCPU)
	#endif
	#ifdef SIGXFSZ
		PAIR(SIGXFSZ)
	#endif
	#ifdef SIGVTALRM
		PAIR(SIGVTALRM)
	#endif
	#ifdef SIGPROF
		PAIR(SIGPROF)
	#endif
	#ifdef SIGWINCH
		PAIR(SIGWINCH)
	#endif

	#if defined(SIGIO) && defined(SIGPOLL)
		/* SIGPOLL same as SIGIO */
		STRPAIR(SIGIO, "SIGIO/SIGPOLL")
	#elif defined(SIGIO)
		PAIR(SIGIO)
	#elif defined(SIGPOLL)
		PAIR(SIGPOLL)
	#endif

	#ifdef SIGINFO
		PAIR(SIGINFO)
	#endif
	#ifdef SIGLOST
		PAIR(SIGLOST)
	#endif
	#ifdef SIGPWR
		PAIR(SIGPWR)
	#endif
	#if defined(SIGSYS)
		/*
		 * According to signal(7)'s manpage, SIGUNUSED is synonymous
		 * with SIGSYS on most architectures.
		 */
		STRPAIR(SIGSYS, "SIGSYS/SIGUNUSED")
	#endif
	};

	PAIR_LOOKUP(signal_pairs, sig);
};
