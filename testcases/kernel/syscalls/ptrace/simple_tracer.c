/*
 * simple example ptrace() code to help build basis for other tests
 *
 * Copyright (c) 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <linux/ptrace.h>
#include <asm/ptrace.h>

#include "test.h"
#include "usctest.h"
#include "spawn_ptrace_child.h"

#include "syscalls.h"

char *TCID = "simple_tracer";
int TST_TOTAL = 0;

#define _decode(name, val) \
({ \
	if (sizeof(long) == 4) \
		printf(name ":%08lx ", val); \
	else if (sizeof(long) == 8) \
		printf(name ":%016lx ", val); \
	else \
		printf(name ":%lx ", val); \
	val; \
})
#define decode(reg) _decode(#reg, pt->reg)
#define decode_user(name, offset) \
	_decode(name, vptrace(PTRACE_PEEKUSER, pid, offset, NULL));
#define decode_sysnum(nr) printf("%s ", get_sysnum(nr))
static void decode_regs(struct pt_regs *pt)
{
#if defined(__bfin__)
	long nr = decode_user("orig_p0", PT_ORIG_P0);
	decode(p0);
	decode(r0);
	decode(r1);
	decode(r2);
	decode(r3);
	decode(r4);
	decode(r5);
	decode_sysnum(nr);
	puts("");
#elif defined(__i386__)
	long nr = decode_user("orig_eax", 4 * ORIG_EAX);
	decode(eax);
	decode(ebx);
	decode(ecx);
	decode(edx);
	decode(esi);
	decode(edi);
	decode(ebp);
	decode_sysnum(nr);
	puts("");
#else
	puts("no idea how to decode your arch");
#endif
}

int main(int argc, char *argv[])
{
	struct pt_regs pt_regs;
	long ret;
	int status;

	make_a_baby(argc, argv);

	while (1) {
		ret = vptrace(PTRACE_GETREGS, pid, NULL, &pt_regs);
		if (ret)
			break;
		decode_regs(&pt_regs);

		ret = vptrace(PTRACE_SYSCALL, pid, NULL, NULL);
		if (ret)
			break;

		if (waitpid(pid, &status, 0) == -1)
			break;
	}

	/* hopefully this worked */
	vptrace(PTRACE_KILL, pid, NULL, NULL);

	return 0;
}
