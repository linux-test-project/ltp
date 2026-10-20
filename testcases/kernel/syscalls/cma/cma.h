// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef CMA_H__
#define CMA_H__

#include <sys/prctl.h>
#include "tst_test.h"
#include "tst_capability.h"

#define PTRACE_SCOPE "/proc/sys/kernel/yama/ptrace_scope"

static inline void cma_check_ptrace_scope(void)
{
	struct tst_cap cap = TST_CAP(TST_CAP_REQ, CAP_SYS_PTRACE);
	int scope;

	if (access(PTRACE_SCOPE, F_OK))
		return;

	SAFE_FILE_SCANF(PTRACE_SCOPE, "%d", &scope);

	if (scope >= 3)
		tst_brk(TCONF, "Yama ptrace_scope=%d forbids process_vm_*()", scope);

	if (scope == 2)
		tst_cap_action(&cap);
}

static inline void cma_allow_parent_ptracer(void)
{
	if (prctl(PR_SET_PTRACER, getppid(), 0, 0, 0) && errno != EINVAL)
		tst_brk(TBROK | TERRNO, "prctl(PR_SET_PTRACER)");
}

#endif /* CMA_H__ */
