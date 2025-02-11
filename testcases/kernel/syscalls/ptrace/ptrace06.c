// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2008 Analog Devices Inc.
 * Copyright (c) Linux Test Project, 2008-2023
 * Copyright (c) 2025 Wei Gao <wegao@suse.com>
 */

/*\
 * Check out-of-bound/unaligned addresses given to
 *
 *  - {PEEK,POKE}{DATA,TEXT,USER}
 *  - {GET,SET}{,FG}REGS
 *  - {GET,SET}SIGINFO
 */

#include <stdlib.h>
#include <sys/ptrace.h>
#include "tst_test.h"

/* this should be sizeof(struct user), but that info is only found
 * in the kernel asm/user.h which is not exported to userspace.
 */

#if defined(__i386__)
# define SIZEOF_USER 284
#elif defined(__x86_64__)
# define SIZEOF_USER 928
#else
# define SIZEOF_USER 0x1000	/* just pick a big number */
#endif

static struct test_case_t {
	int request;
	long addr;
	long data;
} test_cases[] = {
	{PTRACE_PEEKDATA, .addr = 0},
	{PTRACE_PEEKDATA, .addr = 1},
	{PTRACE_PEEKDATA, .addr = 2},
	{PTRACE_PEEKDATA, .addr = 3},
	{PTRACE_PEEKDATA, .addr = -1},
	{PTRACE_PEEKDATA, .addr = -2},
	{PTRACE_PEEKDATA, .addr = -3},
	{PTRACE_PEEKDATA, .addr = -4},
	{PTRACE_PEEKTEXT, .addr = 0},
	{PTRACE_PEEKTEXT, .addr = 1},
	{PTRACE_PEEKTEXT, .addr = 2},
	{PTRACE_PEEKTEXT, .addr = 3},
	{PTRACE_PEEKTEXT, .addr = -1},
	{PTRACE_PEEKTEXT, .addr = -2},
	{PTRACE_PEEKTEXT, .addr = -3},
	{PTRACE_PEEKTEXT, .addr = -4},
	{PTRACE_PEEKUSER, .addr = SIZEOF_USER + 1},
	{PTRACE_PEEKUSER, .addr = SIZEOF_USER + 2},
	{PTRACE_PEEKUSER, .addr = SIZEOF_USER + 3},
	{PTRACE_PEEKUSER, .addr = SIZEOF_USER + 4},
	{PTRACE_PEEKUSER, .addr = -1},
	{PTRACE_PEEKUSER, .addr = -2},
	{PTRACE_PEEKUSER, .addr = -3},
	{PTRACE_PEEKUSER, .addr = -4},
	{PTRACE_POKEDATA, .addr = 0},
	{PTRACE_POKEDATA, .addr = 1},
	{PTRACE_POKEDATA, .addr = 2},
	{PTRACE_POKEDATA, .addr = 3},
	{PTRACE_POKEDATA, .addr = -1},
	{PTRACE_POKEDATA, .addr = -2},
	{PTRACE_POKEDATA, .addr = -3},
	{PTRACE_POKEDATA, .addr = -4},
	{PTRACE_POKETEXT, .addr = 0},
	{PTRACE_POKETEXT, .addr = 1},
	{PTRACE_POKETEXT, .addr = 2},
	{PTRACE_POKETEXT, .addr = 3},
	{PTRACE_POKETEXT, .addr = -1},
	{PTRACE_POKETEXT, .addr = -2},
	{PTRACE_POKETEXT, .addr = -3},
	{PTRACE_POKETEXT, .addr = -4},
	{PTRACE_POKEUSER, .addr = SIZEOF_USER + 1},
	{PTRACE_POKEUSER, .addr = SIZEOF_USER + 2},
	{PTRACE_POKEUSER, .addr = SIZEOF_USER + 3},
	{PTRACE_POKEUSER, .addr = SIZEOF_USER + 4},
	{PTRACE_POKEUSER, .addr = -1},
	{PTRACE_POKEUSER, .addr = -2},
	{PTRACE_POKEUSER, .addr = -3},
	{PTRACE_POKEUSER, .addr = -4},
#ifdef PTRACE_GETREGS
	{PTRACE_GETREGS, .data = 0},
	{PTRACE_GETREGS, .data = 1},
	{PTRACE_GETREGS, .data = 2},
	{PTRACE_GETREGS, .data = 3},
	{PTRACE_GETREGS, .data = -1},
	{PTRACE_GETREGS, .data = -2},
	{PTRACE_GETREGS, .data = -3},
	{PTRACE_GETREGS, .data = -4},
#endif
#ifdef PTRACE_GETFGREGS
	{PTRACE_GETFGREGS, .data = 0},
	{PTRACE_GETFGREGS, .data = 1},
	{PTRACE_GETFGREGS, .data = 2},
	{PTRACE_GETFGREGS, .data = 3},
	{PTRACE_GETFGREGS, .data = -1},
	{PTRACE_GETFGREGS, .data = -2},
	{PTRACE_GETFGREGS, .data = -3},
	{PTRACE_GETFGREGS, .data = -4},
#endif
#ifdef PTRACE_SETREGS
	{PTRACE_SETREGS, .data = 0},
	{PTRACE_SETREGS, .data = 1},
	{PTRACE_SETREGS, .data = 2},
	{PTRACE_SETREGS, .data = 3},
	{PTRACE_SETREGS, .data = -1},
	{PTRACE_SETREGS, .data = -2},
	{PTRACE_SETREGS, .data = -3},
	{PTRACE_SETREGS, .data = -4},
#endif
#ifdef PTRACE_SETFGREGS
	{PTRACE_SETFGREGS, .data = 0},
	{PTRACE_SETFGREGS, .data = 1},
	{PTRACE_SETFGREGS, .data = 2},
	{PTRACE_SETFGREGS, .data = 3},
	{PTRACE_SETFGREGS, .data = -1},
	{PTRACE_SETFGREGS, .data = -2},
	{PTRACE_SETFGREGS, .data = -3},
	{PTRACE_SETFGREGS, .data = -4},
#endif
#if HAVE_DECL_PTRACE_GETSIGINFO
	{PTRACE_GETSIGINFO, .data = 0},
	{PTRACE_GETSIGINFO, .data = 1},
	{PTRACE_GETSIGINFO, .data = 2},
	{PTRACE_GETSIGINFO, .data = 3},
	{PTRACE_GETSIGINFO, .data = -1},
	{PTRACE_GETSIGINFO, .data = -2},
	{PTRACE_GETSIGINFO, .data = -3},
	{PTRACE_GETSIGINFO, .data = -4},
#endif
#if HAVE_DECL_PTRACE_SETSIGINFO
	{PTRACE_SETSIGINFO, .data = 0},
	{PTRACE_SETSIGINFO, .data = 1},
	{PTRACE_SETSIGINFO, .data = 2},
	{PTRACE_SETSIGINFO, .data = 3},
	{PTRACE_SETSIGINFO, .data = -1},
	{PTRACE_SETSIGINFO, .data = -2},
	{PTRACE_SETSIGINFO, .data = -3},
	{PTRACE_SETSIGINFO, .data = -4},
#endif
};

#define SPT(x)[PTRACE_##x] = #x,
static char *strings[] = {
	SPT(TRACEME)
	SPT(PEEKTEXT)
	SPT(PEEKDATA)
	SPT(PEEKUSER)
	SPT(POKETEXT)
	SPT(POKEDATA)
	SPT(POKEUSER)
#ifdef PTRACE_GETREGS
	SPT(GETREGS)
#endif
#ifdef PTRACE_SETREGS
	SPT(SETREGS)
#endif
#ifdef PTRACE_GETSIGINFO
	SPT(GETSIGINFO)
#endif
#ifdef PTRACE_SETSIGINFO
	SPT(SETSIGINFO)
#endif
#ifdef PTRACE_GETFGREGS
	SPT(GETFGREGS)
#endif
#ifdef PTRACE_SETFGREGS
	SPT(SETFGREGS)
#endif
	SPT(KILL)
	SPT(SINGLESTEP)
};

static void child(void)
{
	SAFE_PTRACE(PTRACE_TRACEME, 0, NULL, NULL);
	raise(SIGSTOP);
	exit(0);
}

static void run(void)
{
	size_t i;
	int pid;
	int status;
	int exp_errnos[] = {EIO, EFAULT};

	pid = SAFE_FORK();

	if (!pid)
		child();

	SAFE_WAIT(&status);

	if (!WIFSTOPPED(status))
		tst_brk(TBROK, "child %d was not stopped", pid);

	for (i = 0; i < ARRAY_SIZE(test_cases); ++i) {
		struct test_case_t *tc = &test_cases[i];

		TST_EXP_FAIL_ARR(ptrace(tc->request, pid, (void *)tc->addr,
					(void *)tc->data), exp_errnos, ARRAY_SIZE(exp_errnos),
					"ptrace(%s, ..., %li, %li) failed as expected",
					 strings[tc->request], tc->addr, tc->data);
	}

	SAFE_PTRACE(PTRACE_CONT, pid, NULL, NULL);

}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
};
