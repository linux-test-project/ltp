/*
 * check out-of-bound/unaligned addresses given to
 *  - {PEEK,POKE}{DATA,TEXT,USER}
 *  - {GET,SET}{,FG}REGS
 *  - {GET,SET}SIGINFO
 *
 * Copyright (c) 2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <config.h>
#include "ptrace.h"

#include "test.h"
#include "spawn_ptrace_child.h"
#include "config.h"

/* this should be sizeof(struct user), but that info is only found
 * in the kernel asm/user.h which is not exported to userspace.
 */
#if defined(__i386__)
#define SIZEOF_USER 284
#elif defined(__x86_64__)
#define SIZEOF_USER 928
#else
#define SIZEOF_USER 0x1000	/* just pick a big number */
#endif

char *TCID = "ptrace06";

struct test_case_t {
	int request;
	long addr;
	long data;
} test_cases[] = {
	{
	PTRACE_PEEKDATA,.addr = 0}, {
	PTRACE_PEEKDATA,.addr = 1}, {
	PTRACE_PEEKDATA,.addr = 2}, {
	PTRACE_PEEKDATA,.addr = 3}, {
	PTRACE_PEEKDATA,.addr = -1}, {
	PTRACE_PEEKDATA,.addr = -2}, {
	PTRACE_PEEKDATA,.addr = -3}, {
	PTRACE_PEEKDATA,.addr = -4}, {
	PTRACE_PEEKTEXT,.addr = 0}, {
	PTRACE_PEEKTEXT,.addr = 1}, {
	PTRACE_PEEKTEXT,.addr = 2}, {
	PTRACE_PEEKTEXT,.addr = 3}, {
	PTRACE_PEEKTEXT,.addr = -1}, {
	PTRACE_PEEKTEXT,.addr = -2}, {
	PTRACE_PEEKTEXT,.addr = -3}, {
	PTRACE_PEEKTEXT,.addr = -4}, {
	PTRACE_PEEKUSER,.addr = SIZEOF_USER + 1}, {
	PTRACE_PEEKUSER,.addr = SIZEOF_USER + 2}, {
	PTRACE_PEEKUSER,.addr = SIZEOF_USER + 3}, {
	PTRACE_PEEKUSER,.addr = SIZEOF_USER + 4}, {
	PTRACE_PEEKUSER,.addr = -1}, {
	PTRACE_PEEKUSER,.addr = -2}, {
	PTRACE_PEEKUSER,.addr = -3}, {
	PTRACE_PEEKUSER,.addr = -4}, {
	PTRACE_POKEDATA,.addr = 0}, {
	PTRACE_POKEDATA,.addr = 1}, {
	PTRACE_POKEDATA,.addr = 2}, {
	PTRACE_POKEDATA,.addr = 3}, {
	PTRACE_POKEDATA,.addr = -1}, {
	PTRACE_POKEDATA,.addr = -2}, {
	PTRACE_POKEDATA,.addr = -3}, {
	PTRACE_POKEDATA,.addr = -4}, {
	PTRACE_POKETEXT,.addr = 0}, {
	PTRACE_POKETEXT,.addr = 1}, {
	PTRACE_POKETEXT,.addr = 2}, {
	PTRACE_POKETEXT,.addr = 3}, {
	PTRACE_POKETEXT,.addr = -1}, {
	PTRACE_POKETEXT,.addr = -2}, {
	PTRACE_POKETEXT,.addr = -3}, {
	PTRACE_POKETEXT,.addr = -4}, {
	PTRACE_POKEUSER,.addr = SIZEOF_USER + 1}, {
	PTRACE_POKEUSER,.addr = SIZEOF_USER + 2}, {
	PTRACE_POKEUSER,.addr = SIZEOF_USER + 3}, {
	PTRACE_POKEUSER,.addr = SIZEOF_USER + 4}, {
	PTRACE_POKEUSER,.addr = -1}, {
	PTRACE_POKEUSER,.addr = -2}, {
	PTRACE_POKEUSER,.addr = -3}, {
	PTRACE_POKEUSER,.addr = -4},
#ifdef PTRACE_GETREGS
	{
	PTRACE_GETREGS,.data = 0}, {
	PTRACE_GETREGS,.data = 1}, {
	PTRACE_GETREGS,.data = 2}, {
	PTRACE_GETREGS,.data = 3}, {
	PTRACE_GETREGS,.data = -1}, {
	PTRACE_GETREGS,.data = -2}, {
	PTRACE_GETREGS,.data = -3}, {
	PTRACE_GETREGS,.data = -4},
#endif
#ifdef PTRACE_GETFGREGS
	{
	PTRACE_GETFGREGS,.data = 0}, {
	PTRACE_GETFGREGS,.data = 1}, {
	PTRACE_GETFGREGS,.data = 2}, {
	PTRACE_GETFGREGS,.data = 3}, {
	PTRACE_GETFGREGS,.data = -1}, {
	PTRACE_GETFGREGS,.data = -2}, {
	PTRACE_GETFGREGS,.data = -3}, {
	PTRACE_GETFGREGS,.data = -4},
#endif
#ifdef PTRACE_SETREGS
	{
	PTRACE_SETREGS,.data = 0}, {
	PTRACE_SETREGS,.data = 1}, {
	PTRACE_SETREGS,.data = 2}, {
	PTRACE_SETREGS,.data = 3}, {
	PTRACE_SETREGS,.data = -1}, {
	PTRACE_SETREGS,.data = -2}, {
	PTRACE_SETREGS,.data = -3}, {
	PTRACE_SETREGS,.data = -4},
#endif
#ifdef PTRACE_SETFGREGS
	{
	PTRACE_SETFGREGS,.data = 0}, {
	PTRACE_SETFGREGS,.data = 1}, {
	PTRACE_SETFGREGS,.data = 2}, {
	PTRACE_SETFGREGS,.data = 3}, {
	PTRACE_SETFGREGS,.data = -1}, {
	PTRACE_SETFGREGS,.data = -2}, {
	PTRACE_SETFGREGS,.data = -3}, {
	PTRACE_SETFGREGS,.data = -4},
#endif
#if HAVE_DECL_PTRACE_GETSIGINFO
	{
	PTRACE_GETSIGINFO,.data = 0}, {
	PTRACE_GETSIGINFO,.data = 1}, {
	PTRACE_GETSIGINFO,.data = 2}, {
	PTRACE_GETSIGINFO,.data = 3}, {
	PTRACE_GETSIGINFO,.data = -1}, {
	PTRACE_GETSIGINFO,.data = -2}, {
	PTRACE_GETSIGINFO,.data = -3}, {
	PTRACE_GETSIGINFO,.data = -4},
#endif
#if HAVE_DECL_PTRACE_SETSIGINFO
	{
	PTRACE_SETSIGINFO,.data = 0}, {
	PTRACE_SETSIGINFO,.data = 1}, {
	PTRACE_SETSIGINFO,.data = 2}, {
	PTRACE_SETSIGINFO,.data = 3}, {
	PTRACE_SETSIGINFO,.data = -1}, {
	PTRACE_SETSIGINFO,.data = -2}, {
	PTRACE_SETSIGINFO,.data = -3}, {
	PTRACE_SETSIGINFO,.data = -4},
#endif
};

int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int argc, char *argv[])
{
	size_t i;
	long ret;
	int saved_errno;

	tst_parse_opts(argc, argv, NULL, NULL);

	make_a_baby(argc, argv);

	for (i = 0; i < ARRAY_SIZE(test_cases); ++i) {
		struct test_case_t *tc = &test_cases[i];

		errno = 0;
		ret =
		    ptrace(tc->request, pid, (void *)tc->addr,
			   (void *)tc->data);
		saved_errno = errno;
		if (ret != -1)
			tst_resm(TFAIL,
				 "ptrace(%s, ..., %li, %li) returned %li instead of -1",
				 strptrace(tc->request), tc->addr, tc->data,
				 ret);
		else if (saved_errno != EIO && saved_errno != EFAULT)
			tst_resm(TFAIL,
				 "ptrace(%s, ..., %li, %li) expected errno EIO or EFAULT; actual: %i (%s)",
				 strptrace(tc->request), tc->addr, tc->data,
				 saved_errno, strerror(saved_errno));
		else
			tst_resm(TPASS,
				 "ptrace(%s, ..., %li, %li) failed as expected",
				 strptrace(tc->request), tc->addr, tc->data);
	}

	/* hopefully this worked */
	ptrace(PTRACE_KILL, pid, NULL, NULL);

	tst_exit();
}
