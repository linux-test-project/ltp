// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Verify that after writing an invalid base address into a segment entry,
 * a subsequent segment entry read will raise SIGSEV.
 */

#include "tst_test.h"

#ifdef __i386__
#include "common.h"

int read_segment(unsigned int index)
{
	int res;

	__asm__ __volatile__("\n\
			push    $0x0007;\n\
			pop     %%fs;\n\
			movl    %%fs:(%1), %0"
			     : "=r"(res)
			     : "r"(index * sizeof(int)));

	return res;
}

void run(void)
{
	int pid, status, seg[4];

	seg[0] = 12345;

	create_segment(seg, sizeof(seg));

	TST_EXP_EQ_LI(read_segment(0), seg[0]);

	create_segment(0, 10);

	pid = SAFE_FORK();
	if (!pid) {
		read_segment(0);
		exit(1);
	}

	SAFE_WAITPID(pid, &status, 0);
	if (WIFSIGNALED(status) && (WTERMSIG(status) == SIGSEGV))
		tst_res(TPASS, "generate SEGV as expected");
	else
		tst_res(TFAIL, "child %s", tst_strstatus(status));
}
#endif /* __i386__ */

static struct tst_test test = {
#ifdef __i386__
	.test_all = run,
#endif /* __i386__ */
	.supported_archs = (const char *[]){"x86", NULL},
	.forks_child = 1,
};
