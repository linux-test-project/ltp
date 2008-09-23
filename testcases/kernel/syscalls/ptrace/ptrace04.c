/*
 * make sure PEEKUSER matches GETREGS
 *
 * Copyright (c) 2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later
 */

#if defined(__bfin__)

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <asm/ptrace.h>

#include "test.h"
#include "usctest.h"

char *TCID = "ptrace04";

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

static pid_t pid;

static void make_a_baby(int argc, char *argv[])
{
	if (argc > 1 && !strcmp(argv[1], "child")) {
		/* if we're the child, just sit around doing nothing */
		while (1)
			sleep(1);
	}

	/* ptrace() stuff will fail for us if the child dies */
	signal(SIGCHLD, SIG_IGN);

	pid = vfork();
	if (pid == -1) {
		tst_resm(TFAIL, "vfork() failed");
		tst_exit();
	} else if (pid)
		return;

	errno = 0;
	long ret = ptrace(PTRACE_TRACEME, 0, NULL, NULL);
	if (ret && errno) {
		tst_resm(TFAIL, "PTRACE_TRACEME failed");
		tst_exit();
	}

	execlp(argv[0], argv[0], "child", NULL);
	tst_resm(TFAIL, "execlp() failed");
	tst_exit();
}

void compare_registers(unsigned char poison)
{
	struct pt_regs pt_regs;
	size_t i;
	long ret;
	bool failed = false;

	memset(&pt_regs, poison, sizeof(pt_regs));
	errno = 0;
	ret = ptrace(PTRACE_GETREGS, pid, NULL, &pt_regs);
	if (ret && errno) {
		tst_resm(TFAIL, "PTRACE_GETREGS failed: %s", strerror(errno));
		tst_exit();
	}

	for (i = 0; i < ARRAY_SIZE(regs); ++i) {
		errno = 0;
		ret = ptrace(PTRACE_PEEKUSER, pid, (void *)regs[i].off, NULL);
		if (ret && errno) {
			tst_resm(TINFO, "PTRACE_PEEKUSER: register %s (offset %li) failed: %s",
				regs[i].name, regs[i].off, strerror(errno));
			failed = true;
			continue;
		}

		long *pt_val = (void *)&pt_regs + regs[i].off;
		if (*pt_val != ret) {
			tst_resm(TINFO, "register %s (offset %li) did not match",
				regs[i].name, regs[i].off);
			failed = true;
		}
	}

	tst_resm((failed ? TFAIL : TPASS), "PTRACE PEEKUSER/GETREGS (poison 0x%02x)", poison);
}

int main(int argc, char *argv[])
{
	char *msg;

	if ((msg = parse_opts(argc, argv, NULL, NULL)))
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);

	make_a_baby(argc, argv);

	compare_registers(0x00);
	compare_registers(0xff);

	/* hopefully this worked */
	ptrace(PTRACE_KILL, pid, NULL, NULL);

	return 0;
}

#else

#include <stdio.h>

int main()
{
	puts("ptrace04: test not supported for your arch (yet)");
	return 0;
}

#endif
