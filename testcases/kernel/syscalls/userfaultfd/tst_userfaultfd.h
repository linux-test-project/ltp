// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 * Author: Ricardo Branco <rbranco@suse.com>
 */

static inline int sys_userfaultfd(int flags)
{
	TEST(tst_syscall(__NR_userfaultfd, flags));

	if (TST_RET == -1) {
		if (TST_ERR == EPERM) {
			tst_res(TCONF, "Hint: check /proc/sys/vm/unprivileged_userfaultfd");
			tst_brk(TCONF | TTERRNO,
				"userfaultfd() requires CAP_SYS_PTRACE on this system");
		} else
			tst_brk(TBROK | TTERRNO,
				"Could not create userfault file descriptor");
	}

	return TST_RET;
}
