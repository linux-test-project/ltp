/*
 * Copyright (c) 2015   Author: Oleg Nesterov <oleg@redhat.com>
 *                      Modify: Li Wang <liwang@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * you should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * Description:
 *
 *      save_xstate_sig()->drop_init_fpu() doesn't look right. setup_rt_frame()
 *      can fail after that, in this case the next setup_rt_frame() triggered
 *      by SIGSEGV won't save fpu simply because the old state was lost. This
 *      obviously mean that fpu won't be restored after sys_rt_sigreturn() from
 *      SIGSEGV handler.
 *
 *      These commits fix the issue on v3.17-rc3-3 stable kernel:
 *
 *      commit df24fb859a4e200d9324e2974229fbb7adf00aef
 * 		Author: Oleg Nesterov <oleg@redhat.com>
 * 		Date:   Tue Sep 2 19:57:17 2014 +0200
 *
 *      commit 66463db4fc5605d51c7bb81d009d5bf30a783a2c
 *              Author: Oleg Nesterov <oleg@redhat.com>
 *              Date:   Tue Sep 2 19:57:13 2014 +0200
 *
 * Reproduce:
 *	Test-case (needs -O2).
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include "test.h"
#include "lapi/syscalls.h"

char *TCID = "signal06";
int TST_TOTAL = 5;

#if __x86_64__

#define LOOPS 30000
#define VALUE 123.456

volatile double D;
volatile int FLAGE;

char altstack[4096 * 10] __attribute__((aligned(4096)));

void test(void)
{
	int loop = 0;
	int pid = getpid();

	D = VALUE;
	while (D == VALUE && loop < LOOPS) {
		/* sys_tkill(pid, SIGHUP); asm to avoid save/reload
		 * fp regs around c call */
		asm ("" : : "a"(__NR_tkill), "D"(pid), "S"(SIGHUP));
		asm ("syscall" : : : "ax");

		loop++;
	}

	FLAGE = 1;
	tst_resm(TINFO, "loop = %d", loop);

	if (loop == LOOPS) {
		tst_resm(TPASS, "%s call succeeded", TCID);
	} else {
		tst_resm(TFAIL, "Bug Reproduced!");
		tst_exit();
	}
}

void sigh(int sig LTP_ATTRIBUTE_UNUSED)
{
}

void *tfunc(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int i;

	for (i = -1; ; i *= -1) {
		if (i == -1) {
			TEST(mprotect(altstack, sizeof(altstack), PROT_READ));
			if (TEST_RETURN == -1)
				tst_brkm(TBROK | TTERRNO, NULL, "mprotect failed");
		}

		TEST(mprotect(altstack, sizeof(altstack), PROT_READ | PROT_WRITE));
		if (TEST_RETURN == -1)
			tst_brkm(TBROK | TTERRNO, NULL, "mprotect failed");

		if (FLAGE == 1)
			return NULL;
	}
}

int main(int ac, char **av)
{
	int i, lc;
	pthread_t pt;

	tst_parse_opts(ac, av, NULL, NULL);

	stack_t st = {
		.ss_sp = altstack,
		.ss_size = sizeof(altstack),
		.ss_flags = SS_ONSTACK,
	};

	struct sigaction sa = {
		.sa_handler = sigh,
	};

	TEST(sigaction(SIGSEGV, &sa, NULL));
	if (TEST_RETURN == -1)
		tst_brkm(TBROK | TTERRNO, NULL,
				"SIGSEGV signal setup failed");
	sigaltstack(&st, NULL);
	sa.sa_flags = SA_ONSTACK;

	TEST(sigaction(SIGHUP, &sa, NULL));
	if (TEST_RETURN == -1)
		tst_brkm(TBROK | TTERRNO, NULL,
				"SIGHUP signal setup failed");

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			FLAGE = 0;

			TEST(pthread_create(&pt, NULL, tfunc, NULL));
			if (TEST_RETURN)
				tst_brkm(TBROK | TRERRNO, NULL,
						"pthread_create failed");

			test();

			TEST(pthread_join(pt, NULL));
			if (TEST_RETURN)
				tst_brkm(TBROK | TRERRNO, NULL,
						"pthread_join failed");
		}
	}

	tst_exit();
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL, "Only test on x86_64.");
}
#endif
