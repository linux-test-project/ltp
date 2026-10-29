/*
 * make sure PEEKUSER matches GETREGS
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
#include <sys/ptrace.h>

#include "test.h"
#include "spawn_ptrace_child.h"

char *TCID = "ptrace04";

static void cleanup();

#define R(r) { .name = "PT_" #r, .off = PT_##r },
static struct {
	const char *name;
	long off;
} regs[] = {
#ifdef __bfin__
	R(ORIG_R0) R(ORIG_P0)
	    R(R0) R(R1) R(R2) R(R3) R(R4) R(R5) R(R6) R(R7)
	    R(P0) R(P1) R(P2) R(P3) R(P4) R(P5) R(FP) R(USP)
	    R(I0) R(I1) R(I2) R(I3)
	    R(M0) R(M1) R(M2) R(M3)
	    R(L0) R(L1) R(L2) R(L3)
	    R(B0) R(B1) R(B2) R(B3)
	    R(A0X) R(A0W) R(A1X) R(A1W)
	    R(LC0) R(LC1) R(LT0) R(LT1) R(LB0) R(LB1)
	    R(ASTAT)
	    R(RETS) R(PC) R(RETX) R(RETN) R(RETE)
	    R(SEQSTAT) R(IPEND) R(SYSCFG)
#endif
};

int TST_TOTAL = 2;

void compare_registers(unsigned char poison)
{
#if defined(HAVE_STRUCT_PTRACE_REGS) && defined(PTRACE_GETREGS)
	ptrace_regs _pt_regs;
	size_t i;
	long ret;
	bool failed = false;

	memset(&_pt_regs, poison, sizeof(_pt_regs));
	errno = 0;
	ret = ptrace(PTRACE_GETREGS, pid, NULL, &_pt_regs);
	if (ret && errno) {
		tst_resm(TFAIL | TERRNO, "PTRACE_GETREGS failed");
	} else {

		for (i = 0; i < ARRAY_SIZE(regs); ++i) {
			errno = 0;
			ret = ptrace(PTRACE_PEEKUSER, pid,
				     (void *)regs[i].off, NULL);
			if (ret && errno) {
				tst_resm(TFAIL | TERRNO,
					 "PTRACE_PEEKUSER: register %s "
					 "(offset %li) failed",
					 regs[i].name, regs[i].off);
				failed = true;
				continue;
			}

			long *pt_val = (void *)&_pt_regs + regs[i].off;
			if (*pt_val != ret) {
				tst_resm(TFAIL,
					 "register %s (offset %li) did not "
					 "match\n\tGETREGS: 0x%08lx "
					 "PEEKUSER: 0x%08lx",
					 regs[i].name, regs[i].off, *pt_val,
					 ret);
				failed = true;
			}

		}

	}

	tst_resm((failed ? TFAIL : TPASS),
		 "PTRACE PEEKUSER/GETREGS (poison 0x%02x)", poison);
#else
	tst_brkm(TCONF, cleanup, "System doesn't have ptrace_regs structure");
#endif
}

int main(int argc, char *argv[])
{
	tst_parse_opts(argc, argv, NULL, NULL);

	if (ARRAY_SIZE(regs) == 0)
		tst_brkm(TCONF, NULL, "test not supported for your arch (yet)");

	make_a_baby(argc, argv);

	/* first compare register states when execl() syscall starts */
	tst_resm(TINFO, "Before exec() in child");
	compare_registers(0x00);
	compare_registers(0xff);

	/* then compare register states after execl() syscall finishes */
	tst_resm(TINFO, "After exec() in child");
	errno = 0;
	if (ptrace(PTRACE_SYSCALL, pid, NULL, NULL) && errno) {
		tst_brkm(TFAIL, NULL, "PTRACE_SYSCALL failed: %s",
			 strerror(errno));
	}
	compare_registers(0x00);
	compare_registers(0xff);

	/* hopefully this worked */
	ptrace(PTRACE_KILL, pid, NULL, NULL);

	tst_exit();
}

static void cleanup(void)
{
}
